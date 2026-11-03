# 第三部分　查看文档（Part 3: Viewing Documents）

# 第 4 章　构建 HTML 树

*Chapter 4: Constructing an HTML Tree*

到目前为止，我们的浏览器把网页看成由开始标签（open tag）、结束标签（close tag）和文本组成的一段流。但 HTML 实际上是一棵树，而且尽管树结构到目前为止还无关紧要，它对后续的特性——比如 CSS、JavaScript 和视觉效果——却将是核心所在。所以本章会添加一个正式的 HTML 解析器（parser），并把布局引擎（layout engine）改造成使用它。

## 4.1　由节点组成的树

HTML 树¹ 为每一对开始标签和结束标签建立一个节点，也为每一段文本建立一个节点。² 图 4.1 展示了一个体现这种结构的简单 HTML 文档。

要让浏览器能用上树，词法单元（token）需要进化成节点（node）。这意味着给每个单元加上一个子节点列表和一个指向父节点的指针。下面是新的 `Text` 类，表示位于树的叶节点处的文本：

```python
class Text:
    def __init__(self, text, parent):
        self.text = text
        self.children = []
        self.parent = parent
```

既然一个节点由两个标签（开始标签和结束标签）构成，我们把 `Tag` 类改名为 `Element`，并把它写成这样：

```python
class Element:
    def __init__(self, tag, parent):
        self.tag = tag
        self.children = []
        self.parent = parent
```

为了保持一致，我给 `Text` 和 `Element` 都加了 `children` 字段，尽管文本节点永远不会有子节点。

从源代码构建节点树的过程称为解析（parsing）。解析器一次处理一个元素或文本节点，逐步把树建起来。但这意味着解析器在工作过程中需要保存一棵不完整的树。例如，假设解析器到目前为止读入了这段 HTML：

```html
<html><video></video><section><h1>This is my webpage
```

*图 4.1　一份 HTML 文档，展示了标签、文本与嵌套结构。*

示意图：一份 HTML 文档（从 `<!doctype html>` 到 `</html>` 逐层嵌套），左侧标注各部分的角色——标签、文本内容、无关紧要的空白、结束标签。

*图 4.2　解析一段 HTML 时已完成与未完成的节点。*

示意图：上方「未完成节点」：html → section → h1（依次嵌套，仍可继续添加子节点）；下方「已完成节点」：video、文本节点；最底一行「词法单元」：`<html>` `<video>` `</video>` `<section>` `<h1>` This is my。

解析器已经看到了五个标签（外加一个文本节点）。剩下的 HTML 还会包含更多开始标签、结束标签和文本，但无论它接下来看到什么词法单元，都不会再有新节点被加到已经闭合的 `<video>` 标签下面。所以那个节点已经「完成」。但其他节点还未完成：取决于接下来的 HTML 是什么，`<html>`、`<section>` 和 `<h1>` 节点都还可能加入新的子节点——见图 4.2。

由于解析器从头到尾读取 HTML 文件，这些未完成的标签总是位于树中某个固定的部分：它们已被打开但尚未闭合；它们在源代码中总是出现在已完成节点之后；而且它们总是其他未完成标签的子节点。利用这些事实，我们用一个按「父节点在前、子节点在后」排序的未完成标签列表来表示不完整的树。列表中的第一个节点是 HTML 树的根；最后一个节点是最近一个未完成的标签。³

解析比我们之前做的词法分析（lexing）要复杂一点，所以我们要把它拆成几个函数，组织进一个新的 `HTMLParser` 类。这个类还可以保存它正在分析的源代码以及那棵不完整的树：

```python
class HTMLParser:
    def __init__(self, body):
        self.body = body
        self.unfinished = []
```

解析器启动之前一个标签都还没见到，所以保存树的 `unfinished` 列表起初是空的。而随着解析器读入词法单元，这个列表会被逐渐填满。让我们先满怀憧憬地把现有的 `lex` 函数改名为 `parse`，从这里开始：

```python
class HTMLParser:
    def parse(self):
        # ...
```

我们需要对 `parse` 动一点手术。目前 `parse` 创建 `Tag` 和 `Text` 对象并把它们追加到 `out` 数组。我们要求它创建 `Element` 和 `Text` 对象，并把它们加入未完成的树。由于树比列表复杂一些，我会把「向树中添加」的逻辑挪到两个新方法 `add_text` 和 `add_tag` 里。

```python
def parse(self):
    text = ""
    in_tag = False
    for c in self.body:
        if c == "<":
            in_tag = True
            if text: self.add_text(text)
            text = ""
        elif c == ">":
            in_tag = False
            self.add_tag(text)
            text = ""
        else:
            text += c
    if not in_tag and text:
        self.add_text(text)
    return self.finish()
```

`out` 变量不见了，另外注意我还把返回值挪到了一个新的 `finish` 方法，由它把不完整的树变成最终的完整树。那么：怎么向树里添加东西呢？

> **深入探索**
> HTML 源自一长串文档处理系统。它的前身 SGML [2] 可以一直追溯到 RUNOFF [3]，与如今用于 Linux 手册页的 troff [4] 是近亲。当年把 SGML 标准化的那个委员会 [5]，如今负责 .odf、.docx 和 .epub 格式。

## 4.2　构建树

我们来谈谈如何把节点加进树里。要添加一个文本节点，就把它添加为最后一个未完成节点的子节点：

```python
class HTMLParser:
    def add_text(self, text):
        parent = self.unfinished[-1]
        node = Text(text, parent)
        parent.children.append(node)
```

而标签要更复杂一点，因为它既可能是开始标签，也可能是结束标签：

```python
class HTMLParser:
    def add_tag(self, tag):
        if tag.startswith("/"):
            # ...
        else:
            # ...
```

开始标签会在列表末尾添加一个未完成节点：

```python
def add_tag(self, tag):
    # ...
    else:
        parent = self.unfinished[-1]
        node = Element(tag, parent)
        self.unfinished.append(node)
```

结束标签则是把最后一个未完成节点收尾：把它添加为列表中前一个未完成节点的子节点：

```python
def add_tag(self, tag):
    if tag.startswith("/"):
        node = self.unfinished.pop()
        parent = self.unfinished[-1]
        parent.children.append(node)
    # ...
```

解析器完成之后，只需把所有还未完成的节点逐一收尾，就把不完整的树变成了完整的树：

```python
class HTMLParser:
    def finish(self):
        while len(self.unfinished) > 1:
            node = self.unfinished.pop()
            parent = self.unfinished[-1]
            parent.children.append(node)
        return self.unfinished.pop()
```

这几乎就是一个完整的解析器了，但在文档的开头和结尾它还不太能正常工作。第一个开始标签是一个没有父节点的边界情况：

```python
def add_tag(self, tag):
    # ...
    else:
        parent = self.unfinished[-1] if self.unfinished else None
        # ...
```

最后一个标签同样是边界情况，因为已经没有未完成的节点可以容纳它：

```python
def add_tag(self, tag):
    if tag.startswith("/"):
        if len(self.unfinished) == 1: return
        # ...
```

好，全都搞定了。来测试一下我们的解析器，看看它工作得怎么样！

> **深入探索**
> 欠考虑的 JavaScript `document.write` 方法允许 JavaScript 在 HTML 源代码正在被解析时修改它！这其实是个坏主意 [6]。实现 `document.write` 必须让 HTML 解析器停下来执行 JavaScript，而这会拖慢页面后续用到的图片、CSS 和 JavaScript 的请求。为了解决这个问题，现代浏览器采用推测解析（speculative parsing）[7]，在解析还没完成时就开始加载额外的资源。

## 4.3　调试解析器

我们怎么知道解析器做的事是对的——也就是说，它构建出了正确的树？着手的地方就是看看它产出的树。用一个简单的递归美化打印器（pretty-printer）就能做到：

```python
def print_tree(node, indent=0):
    print(" " * indent, node)
    for child in node.children:
        print_tree(child, indent + 2)
```

这里我们打印树中的每个节点，并用缩进来展现树的结构。既然每个节点都要打印，就值得花点时间给它们一个漂亮的打印形式；在 Python 里，这意味着定义 `__repr__` 函数：

```python
class Text:
    def __repr__(self):
        return repr(self.text)
class Element:
    def __repr__(self):
        return "<" + self.tag + ">"
```

一般来说，为任何数据对象定义 `__repr__` 方法都是个好主意，而且这些 `__repr__` 方法应该打印出所有相关字段。

在与本章对应的网页 [8] 上试一试：解析 HTML 源代码，然后调用 `print_tree` 把它可视化：

```python
body = URL(sys.argv[1]).request()
nodes = HTMLParser(body).parse()
print_tree(nodes)
```

开头你会看到类似这样的内容：

```text
<!doctype html>
    '\n'
    <html lang="en-US" xml:lang="en-US">
    '\n'
    <head>
        '\n  '
        <meta charset="utf-8" />
```

马上就有几个问题跳了出来。先从最顶上的 `<!doctype html>` 标签说起。这个特殊标签叫做文档类型声明（doctype）[9]，它永远是 HTML 文档的第一样东西。但它其实根本算不上一个元素，也不该有结束标签。我们的浏览器不会拿 doctype 做任何事，所以最好把它丢掉：⁴

```python
def add_tag(self, tag):
    if tag.startswith("!"): return
    # ...
```

这会忽略所有以感叹号开头的标签，不仅丢掉了文档类型声明，也丢掉了注释——HTML 中注释写成 `<!-- comment text -->`。

不过，只丢掉 doctype 还不够——现在运行解析器，它会崩溃。这是因为 doctype 后面跟着一个换行符，解析器把它当作文本，并试图插入树中。可这时候还没有树，因为解析器还没见过任何开始标签。为了简单起见，我们让浏览器直接跳过只含空白字符的文本节点，绕开这个问题：⁵

```python
def add_text(self, text):
    if text.isspace(): return
    # ...
```

现在，解析 browser.engineering 主页得到的 HTML 树开头部分大致是这样：

```text
<html lang="en-US" xml:lang="en-US">
    <head>
    <meta charset="utf-8" /="">
```

下一个问题：为什么所有东西缩进得这么深？为什么这些打开的元素一直不闭合？

> **深入探索**
> 在 SGML 中，文档类型声明包含一个定义合法标签的 URL，旧版本的 HTML 也推荐这样做。浏览器确实会利用文档类型声明的缺失来识别 [10] 非常古老的、SGML 之前的 HTML 版本，⁶ 但它们并不使用那个 URL，所以对现代 HTML 而言，`<!doctype html>` 就是最好的文档类型声明。

## 4.4　自闭合标签

像 `<meta>` 和 `<link>` 这样的元素就是所谓的自闭合（self-closing）标签：这些标签不包围内容，所以你永远不会写 `</meta>` 或 `</link>`。我们的解析器需要对它们做特殊支持。HTML 中有一份具体的清单 [12] 列出了这些自闭合标签（规范称它们为「空（void）」标签）：⁷

```python
SELF_CLOSING_TAGS = [
    "area", "base", "br", "col", "embed", "hr", "img", "input",
    "link", "meta", "param", "source", "track", "wbr",
]
```

我们的解析器需要自动闭合这份清单里的标签：

```python
def add_tag(self, tag):
    # ...
    elif tag in self.SELF_CLOSING_TAGS:
        parent = self.unfinished[-1]
        node = Element(tag, parent)
        parent.children.append(node)
```

这段代码看起来是对的，但并不能正常工作。为什么？因为我们的解析器在寻找一个名叫 `meta` 的标签，实际找到的标签名却是 "meta name = ..."。自闭合代码没有被触发，是因为 `<meta>` 标签带有属性。

HTML 属性（attribute）为元素附加信息；一个开始标签可以带任意多个属性。属性的值可以加引号、不加引号，或者整个省略。我们先专注于基本的属性支持，忽略含有空白字符的属性值——那种情况有点复杂。

既然我们不处理值中的空白字符，就可以按空白字符切分，得到标签名和各个「属性–值」对：

```python
class HTMLParser:
    def get_attributes(self, text):
        parts = text.split()
        tag = parts[0].casefold()
        attributes = {}
        for attrpair in parts[1:]:
            # ...
        return tag, attributes
```

HTML 标签名不区分大小写，属性名也一样，所以我对它们做了大小写折叠（casefold）。⁸ 接着，在循环体里，我把每个「属性–值」对拆成名称和值。最简单的情况是不带引号的属性，两者由一个等号分隔：

```python
def get_attributes(self, text):
    # ...
    for attrpair in parts[1:]:
        if "=" in attrpair:
            key, value = attrpair.split("=", 1)
            attributes[key.casefold()] = value
    # ...
```

值也可以省略，比如 `<input disabled>`，这时属性值默认为空字符串：

```python
for attrpair in parts[1:]:
    # ...
    else:
        attributes[attrpair.casefold()] = ""
```

最后，值还可以带引号，这种情况下必须把引号剥掉：⁹

```python
if "=" in attrpair:
    # ...
    if len(value) > 2 and value[0] in ["'", "\""]:
        value = value[1:-1]
    # ...
```

我们把这些属性存进 `Element`：

```python
class Element:
    def __init__(self, tag, attributes, parent):
        self.tag = tag
        self.attributes = attributes
        # ...
```

这就意味着我们需要在 `add_tag` 的开头调用 `get_attributes`，拿到构造 `Element` 所需的属性。

```python
def add_tag(self, tag):
    tag, attributes = self.get_attributes(tag)
```

记得在 `add_tag` 里改用 `tag` 和 `attributes` 而不是 `text`，然后再次运行你的解析器：

```text
<html>
    <head>
    <meta>
    <link>
    <link>
    <link>
    <link>
    <link>
    <meta>
```

很接近了！没错，如果你打印属性，会发现带空白字符的属性（比如某个 `meta` 标签上的 author）会被错误地解析成多个属性，而自闭合标签末尾的斜杠会被错误地当成一个额外的属性。更好的解析器会修复这些问题。但我们还是让解析器保持现状——对我们要构建的浏览器来说，这些问题不会造成麻烦——接着把它和浏览器集成起来。

> **深入探索**
> 在 XHTML [14] 看起来可能取代 HTML 的那些年里，给自闭合标签末尾加一个斜杠（比如 `<br/>`）成了时髦做法，而我这样的老派人一直没改掉这个习惯。但与 XML [15] 不同，HTML 里的自闭合标签是按名字识别的，并不靠什么特殊语法，所以这个斜杠是可有可无的。

## 4.5　使用节点树

目前 `Layout` 类是一个词法单元接着一个词法单元地工作；我们现在想让它改成节点接着节点。所以，把旧的 `token` 方法拆成两部分：开始标签的所有分支放进新的 `open_tag` 方法，结束标签的所有分支放进新的 `close_tag` 方法：¹⁰

```python
class Layout:
    def open_tag(self, tag):
        if tag == "i":
            self.style = "italic"
        # ...
    def close_tag(self, tag):
        if tag == "i":
            self.style = "roman"
        # ...
```

现在需要 `Layout` 对象去遍历节点树，按正确的顺序调用 `open_tag`、`close_tag` 和 `text`：

```python
def recurse(self, tree):
    if isinstance(tree, Text):
        for word in tree.text.split():
            self.word(word)
    else:
        self.open_tag(tree.tag)
        for child in tree.children:
            self.recurse(child)
        self.close_tag(tree.tag)
```

`Layout` 的构造函数现在可以调用 `recurse`，而不必循环遍历词法单元列表。我们还需要浏览器构建节点树，像这样：

```python
class Browser:
    def load(self, url):
        body = url.request()
        self.nodes = HTMLParser(body).parse()
        self.display_list = Layout(self.nodes).display_list
        self.draw()
```

运行一下——浏览器现在用的应该就是解析出的 HTML 树了。

> **深入探索**
> doctype 语法是一种版本管理机制——声明网页使用的是哪个版本的 HTML。但实际上，doctype 的 `html` 值表示的不只是某个特定的 HTML 版本，而是更宽泛意义上的 HTML 活标准（living standard）[16]。¹¹ 之所以叫「活」标准，是因为它随着特性的加入一直在变。这些变化的机制不过是浏览器发布新特性，而不是 HTML「版本」的任何变化。总的来说，Web 是一个无版本的平台——新特性经常以增强的形式加入，但前提是它们不破坏现有特性。¹²

## 4.6　处理作者的错误

现在，解析器能正确处理 HTML 页面了——至少当 HTML 出自那种循规蹈矩、记得写 `<head>` 标签、闭合每个开始标签、早上还会把床铺整理好的程序员之手时。凡人没有这样的自律，所以浏览器还必须处理破损、混乱、没有 head 的 HTML。事实上，现代 HTML 解析器能把任意一串字符变成一棵 HTML 树，无论标记多么混乱。¹³

完整的算法，正如你料想的那样，复杂得难以置信：几十种越来越特殊的情形构成了一部人类错误分类学。不过它比较好用的特性之一是隐式标签（implicit tag）。通常，HTML 文档以一段熟悉的样板开头：

```html
<!doctype html>
<html>
    <head>
    </head>
    <body>
    </body>
</html>
```

实际上，除了 doctype，这六个标签全是可选的：网页省略它们时，浏览器会自动插入。让我们通过一个新的 `implicit_tags` 函数在浏览器里插入隐式标签。我们要在 `add_text` 和 `add_tag` 里都调用它：

```python
class HTMLParser:
    def add_text(self, text):
        if text.isspace(): return
        self.implicit_tags(None)
        # ...
    def add_tag(self, tag):
        tag, attributes = self.get_attributes(tag)
        if tag.startswith("!"): return
        self.implicit_tags(tag)
        # ...
```

注意，对于被忽略的空白文本和 doctype，并没有调用 `implicit_tags`。我们还要在 `finish` 里调用它，确保即使是空字符串也能创建出 `<html>` 和 `<body>` 标签：

```python
class HTMLParser:
    def finish(self):
        if not self.unfinished:
            self.implicit_tags(None)
        # ...
```

`implicit_tags` 的参数是标签名（文本节点则传 `None`），我们拿它与未完成标签的列表比较，判断漏掉了什么：

```python
class HTMLParser:
    def implicit_tags(self, tag):
        while True:
            open_tags = [node.tag for node in self.unfinished]
            # ...
```

`implicit_tags` 里有一个循环，因为可能连续漏掉多个标签；循环的每一轮只补一个。要判断该补哪个隐式标签（如果需要补的话），得考察已打开的标签和正要插入的标签。先从最简单的情形——隐式 `<html>` 标签——开始。如果文档的第一个标签不是 `<html>`，就需要一个隐式的 `<html>` 标签：

```python
while True:
    # ...
    if open_tags == [] and tag != "html":
        self.add_tag("html")
```

`<head>` 和 `<body>` 也都可以省略，但要判断该补哪一个，得看正在添加的是什么标签：

```python
while True:
    # ...
    elif open_tags == ["html"] \
        and tag not in ["head", "body", "/html"]:
        if tag in self.HEAD_TAGS:
            self.add_tag("head")
        else:
            self.add_tag("body")
```

这里的 `HEAD_TAGS` 列出了本应放进 `<head>` 元素里的那些标签：¹⁴

```python
class HTMLParser:
    HEAD_TAGS = [
        "base", "basefont", "bgsound", "noscript",
        "link", "meta", "title", "style", "script",
    ]
```

注意，如果 `<html>` 和 `<head>` 标签都被省略，`implicit_tags` 会循环两圈把两个都插进来。第一圈里 `open_tags` 是 `[]`，代码添加 `<html>` 标签；随后第二圈里 `open_tags` 是 `["html"]`，于是添加 `<head>` 标签。¹⁵

最后，如果解析器还在 `<head>` 里、却看到了本应属于 `<body>` 的元素，`</head>` 标签也可以是隐式的：

```python
while True:
    # ...
    elif open_tags == ["html", "head"] and \
        tag not in ["/head"] + self.HEAD_TAGS:
        self.add_tag("/head")
```

严格说来，`</body>` 和 `</html>` 标签也可以是隐式的。但由于我们的 `finish` 函数本来就会闭合所有未完成的标签，这不需要额外的代码。于是 `implicit_tags` 只剩下一件事：退出循环。

```python
while True:
    # ...
    else:
        break
```

当然，处理畸形 HTML 的规则还有更多：格式化标签、嵌套段落、内嵌的可缩放矢量图形（Scalable Vector Graphics，SVG）与 MathML，以及其他各种复杂情况。每一种都有充满边界情况的复杂规则。但关于处理作者错误的讨论就到此为止吧。

处理畸形 HTML 的这些规则看起来很随意，事实也确实如此：它们是多年来试图猜出人们写那段 HTML 时的「本意」而演化出来的，如今被编纂进 HTML 解析标准 [17]。当然，这些规则有时会「猜」错——但正如 Web 上常见的那样，重要的是每个浏览器的做法都一样，而不是各自去猜什么才是正确的做法。

现在来收获成果吧！图 4.3 展示了用我们自己的浏览器加载本书网站 [18] 的截图。¹⁶

*图 4.3　在本章版本的浏览器中查看 https://browser.engineering/index.html。*

> **深入探索**
> 多亏了隐式标签，你基本可以跳过 `<html>`、`<body>` 和 `<head>` 元素，它们会被隐式地补回来。事实上，HTML 解析器的众多状态 [19] 保证了比这更严格的东西：每个 HTML 文档都恰好有一个 `<head>` 和一个 `<body>`，并且顺序符合预期。¹⁷

## 4.7　总结

本章让我们的浏览器明白了 HTML 是一棵树，而不只是一份平铺的词法单元列表。我们添加了：

- 一个把 HTML 词法单元转换成树的解析器；
- 识别并处理元素属性的代码；
- 针对某些畸形 HTML 文档的自动修复；
- 一个对 HTML 树进行布局的递归算法。

HTML 的树结构对显示视觉上复杂的网页至关重要，我们将在下一章看到这一点。

## 4.8　大纲

我们浏览器中完整的函数、类和方法集合应该看起来像这样：

```python
class URL:
    def __init__(url)
    def request()
class Text:
    def __init__(text, parent)
    def __repr__()
class Element:
    def __init__(tag, attributes, parent)
    def __repr__()
def print_tree(node, indent)
class HTMLParser:
    SELF_CLOSING_TAGS
    HEAD_TAGS
    def __init__(body)
    def parse()
    def get_attributes(text)
    def add_text(text)
    def add_tag(tag)
    def implicit_tags(tag)
    def finish()
FONTS
def get_font(size, weight, style)
WIDTH, HEIGHT
HSTEP, VSTEP
class Layout:
    def __init__(tree)
    def recurse(tree)
    def open_tag(tag)
    def close_tag(tag)
    def flush()
    def word(word)
SCROLL_STEP
class Browser:
    def __init__()
    def draw()
    def load(url)
    def scrolldown(e)
```

## 4.9　练习

### 练习 4.1　注释（Comments）

更新 HTML 词法分析器以支持注释。HTML 中的注释以 `<!--` 开头、以 `-->` 结尾。不过，注释和标签并不相同：注释里可以包含任何文本，包括左尖括号和右尖括号。词法分析器应当跳过注释，完全不生成任何词法单元。检验一下：`<!-->` 是一个完整的注释，还是只开启了一个注释？

### 练习 4.2　段落（Paragraphs）

一个段落包含另一个段落意味着什么，这并不清楚。修改解析器，使 `<p>hello<p>world</p>` 这样的文档得到两个并列的段落，而不是一个段落套着另一个段落；真实浏览器也是这么做的。对 `<li>` 元素做同样的处理，但确保嵌套列表仍然可行。

### 练习 4.3　脚本（Scripts）

嵌入 `<script>` 标签的 JavaScript 代码会把左尖括号当作「小于」的意思。修改你的词法分析器，把 `<script>` 标签的内容特殊对待：`<script>` 内部不允许任何标签，唯一的例外是 `</script>` 结束标签。¹⁸

### 练习 4.4　带引号的属性（Quoted attributes）

带引号的属性可以包含空格和右尖括号。修复词法分析器，让它正确支持这一点。提示：当前的词法分析器是一台有限状态机（finite state machine），有两个状态（由 `in_tag` 决定）。你需要更多的状态。

### 练习 4.5　语法高亮（Syntax highlighting）

按练习 1.5 的方式实现 view-source 协议，但要让它对 HTML 页面的源代码做语法高亮。HTML 标签的源代码保持普通字体，而文本内容显示为粗体。如果你已经实现了练习 1.5，还要把文本包进 `<pre>` 标签以保留换行。提示：以 HTML 解析器为基类构建你的语法高亮器。

### 练习 4.6　错误嵌套的格式化标签（Mis-nested formatting tags）

扩展你的 HTML 解析器，支持诸如 `<b>Bold<i>both</b>italic</i>` 这样的标记。这需要记录已打开的文本格式化元素的集合，并在文本格式化元素以错误顺序闭合时插入隐式的开始标签和结束标签。以前面的粗体/斜体为例，应当在 `</b>` 之前插入隐式的 `</i>`，并在 `</b>` 之后插入隐式的 `<i>`。

## 链接

[1] https://en.wikipedia.org/wiki/Document_Object_Model
[2] https://en.wikipedia.org/wiki/Standard_Generalized_Markup_Language
[3] https://en.wikipedia.org/wiki/TYPSET_and_RUNOFF
[4] https://troff.org
[5] https://www.iso.org/committee/45374.html
[6] https://developer.mozilla.org/en-US/docs/Web/API/Document/write
[7] https://developer.mozilla.org/en-US/docs/Glossary/speculative_parsing
[8] https://browser.engineering/html.html
[9] https://html.spec.whatwg.org/multipage/syntax.html#the-doctype
[10] https://developer.mozilla.org/en-US/docs/Web/HTML/Quirks_Mode_and_Standards_Mode
[11] https://hsivonen.fi/doctype/
[12] https://html.spec.whatwg.org/multipage/syntax.html#void-elements
[13] https://www.b-list.org/weblog/2018/nov/26/case/
[14] https://www.w3.org/TR/xhtml1/
[15] https://www.w3.org/TR/xml/#sec-starttags
[16] https://html.spec.whatwg.org/
[17] https://browser.engineering/index.html
[18] https://html.spec.whatwg.org/multipage/parsing.html#parsing-main-afterbody
[19] https://html.spec.whatwg.org/multipage/parsing.html#script-data-end-tag-name-state

## 脚注

¹ 这棵树通常被称为 DOM 树，即文档对象模型（Document Object Model）[1]。眼下我还是继续叫它 HTML 树。

² 现实中还有其他类型的节点，比如注释、doctype、CDATA 区段和处理指令。甚至还有一些已废弃的类型！

³ 在 Python 以及大多数其他语言中，从列表末尾添加、删除元素要比从列表开头做这些更快。

⁴ 真实浏览器利用 doctype 在标准兼容模式与旧版解析、布局模式之间切换。

⁵ 真实浏览器会保留空白字符，以便把 `make<span></span>up` 正确渲染为一个单词，而把 `make<span> </span>up` 渲染为两个单词。我们的浏览器不会这样做。另外，忽略空白字符还省去了对只含空白的文本标签的特殊处理，让后面的章节更简单。

⁶ 还有一种叫做「几乎标准（almost standards）[11]」模式或「有限怪癖（limited quirks）」模式的古怪东西，起因是表格单元格垂直布局的一次不向后兼容的变更。是的，这些不是我编出来的！

⁷ 这些标签很多都相当冷门。浏览器还支持另外一些这里没有列出的、已废弃的自闭合标签，比如 `keygen`。

⁸ 在切罗基语（Cherokee）这类语言里，把文本转成小写是做不区分大小写比较的错误方式 [13]。具体到 HTML，标签名只使用 ASCII 字符，因此转小写其实就够了，但我用的是 Python 的 `casefold` 函数，因为这是个值得养成的好习惯。

⁹ 带引号的属性允许引号内出现空白字符。要正确解析它，需要类似有限状态机（finite state machine）的做法，而不能只是按空白切分。

¹⁰ 文本词法单元的那个分支不再需要了，因为我们的浏览器可以直接调用现有的 `add_text` 方法。

¹¹ 预计 HTML 今后不会再新增任何 doctype 版本。

¹² 特性是可以移除的，但前提是绝大多数网站都不再使用它。与其他平台相比，这使得移除 Web 特性变得非常困难。

¹³ 是的，这很疯狂；2000 年代初的几年里 W3C 还曾试图把它取消，但失败了。

¹⁴ `<script>` 标签既可以放在 `head` 里，也可以放在 `body` 里，但默认放进 `head`。

¹⁵ 这些 `add_tag` 方法自身也会调用 `implicit_tags`，这意味着如果漏掉某种情形，就可能陷入无限循环。我已仔细确保 `implicit_tags` 添加的每个标签本身不会再触发更多的隐式标签。

¹⁶ 说句公道话，用第 3 章的浏览器看起来其实差不多。

¹⁷ 至少对每个文档而言如此。使用框架（frame）或模板的 HTML 文件可能有多对 `<head>` 和 `<body>`，但它们对应的是不同的文档。

¹⁸ 严格来说，它只是 `</script` 后跟空格、制表符、\v、\r、斜杠或大于号 [20]。如果你需要在 JavaScript 代码里谈论 `</script>` 标签，就得把它拆成多个字符串。
