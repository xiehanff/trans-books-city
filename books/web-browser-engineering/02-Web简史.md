# 导言 ii：Web 简史

*Introduction ii: History of the Web*

本章将深入探讨 Web 本身的历史：它从何而来，Web 与浏览器又是如何一步步演变至今的。这段历史并不追求面面俱到；¹ 重点放在催生 Web 的关键事件与思想，以及其发明者们当初的目标与动机。

## ii.1 Memex 的概念

关于计算机如何可能彻底变革信息领域，一次影响深远的早期探索是 Vannevar Bush 1945 年的文章《As We May Think》（诚如所思）[2]。这篇文章设想了一种名为 memex（记忆延伸机）[3] 的机器，它帮助单个的人查看并探索世界上的所有信息（见图 ii.1）。尽管它是用当时的缩微胶片（microfilm）屏幕技术来描述的，但即便用户界面和技术细节有所不同，它的宗旨和概念与我们所熟知的今天的 Web 仍有着明显的相似之处。从核心上讲，Web 正是围绕这种类 Memex 的目标组织起来的：表示并展示信息，为人类提供高效学习与探索的途径。

人类这个物种的集体知识与智慧，早已超出了任何单个人类心智、组织、图书馆、国家、文化、群体或语言的容量。然而，尽管我们作为人类连所有可知之事的一小部分都注定无法尽知，我们仍然可以利用技术比以往更高效地学习，尤其是快速获取那些需要学习、记忆或回忆的信息。请看 Vannevar Bush 描述的这段想象中的研究会话——它与我们如今有时使用 Web 的方式惊人地相似：

> 不妨假设 memex 的主人对弓箭的起源与特性感兴趣。……他的 memex 里存有数十本可能相关的书籍和文章。他先快速翻阅一部百科全书，找到一篇有趣但颇为简略的文章，让它保持投影显示。接着，他在一部历史著作中找到另一条相关条目，并把两者关联起来。如此继续下去，他便构建起一条由众多条目组成的踪迹（trail）。

计算机和互联网让我们得以处理并存储想要的信息。但真正帮助我们组织、查找这些信息与知识，从而让其发挥作用的，是 Web。²

*图 ii.1 《As We May Think》的最初刊行版。（Dunkoman [4] 来自 Wikipedia [5]，CC BY 2.0 [6]。）*

《As We May Think》着重强调了 memex 的两项特性：信息记录的查找，以及相关记录之间的关联。事实上，文章强调的是后者的重要性——我们正是通过在已知事物之间建立此前未知的联系来学习的：

> 当任何类型的数据被放入存储器时，它们都按字母或数字顺序归档。……人类心智的运作方式并非如此。它靠联想（association）来运作。

Bush 所说的"关联"，指的是一条经由人工挑选的链接、从一条记录通往下一条记录的思路踪迹。他所设想的不仅是一座万能图书馆，还是一种记录我们所学成果的通用方式。

## ii.2 Web 的诞生

由超链接（hyperlink）[9] 连接超文本（hypertext）[8] 文档的概念，是由 Ted Nelson 主持的 Xanadu 计划（Project Xanadu）[10] 于 1964 至 1965 年间提出的。³ 超文本就是用指向其他文本的超链接标记起来的文本。⁴ 听起来很耳熟？网页就是超文本，网页之间的链接就是超链接。书写网页的格式是 HTML，加载网页的协议是 HTTP，这两个缩写中都包含"HyperText"。早期的超文本编辑系统（Hypertext Editing System）的一个例子见图 ii.2。

*图 ii.2 1969 年，一位计算机操作员正在使用超文本编辑系统。（Gregory Lloyd 来自 Wikipedia [12]，CC BY-SA 4.0 International [13]。）*

独立于 Xanadu 计划，第一个超链接系统出现时用于在单个文档内滚动导航；后来它被推广为在多个文档之间建立链接。而与那些早期系统一样，Web 既支持文档内部的链接，也支持文档之间的链接。例如，URL http://browser.engineering/history.html#the-web-emerges 指向一个名为 "history.html" 的文档，并且确切指向其中名为 "the-web-emerges" 的元素：也就是本节。访问该 URL 将会加载本章并滚动到本节。

这些工作还构成了 Douglas Engelbart 的"演示之母"（mother of all demos）[14] 中一个关键部分的来源并给予了它灵感——那也许是计算史上最具影响力的技术演示（见图 ii.3）。那场演示不仅展示了 Web 的关键概念，还介绍了计算机鼠标和图形用户界面（graphical user interface），而这两者正是浏览器 UI 的核心组成部分。⁵

当然，这些研究与 Web 的"文档–URL–超链接"机制之间有着非常直接的联系——Web 正是建立在超文本思想之上并将其付诸实践。例如，HyperTIES [16] 系统就带有高亮显示的超链接，它被用来制作世界上第一份以电子方式出版的学术期刊——1988 年的那期《Communications of the ACM》[17]。Tim Berners-Lee 将那期 1988 年的刊物列为万维网（World Wide Web）的灵感来源，⁶ 在万维网中，他将链接概念与互联网的可用性结合起来，从而实现了此前几十年间所有这些工作的诸多最初目标。⁷

*图 ii.3 Doug Engelbart 正在进行"演示之母"。（SRI International，经 Doug Engelbart Institute [15] 提供。）*

"hyperlink"（超链接）一词可能是在 1987 年被创造出来的，与 Apple 计算机上的 HyperCard [18] 系统有关。该系统也是最早（或许就是第一个）引入下述概念的系统：用脚本来增强超文本，让脚本处理点击等用户事件并执行增强 UI 的操作——就像网页上的 JavaScript 一样！与大多数前身不同，它还拥有图形化的 UI 元素，而不只是文本。

1989 至 1990 年间，第一个 Web 浏览器（名为 World Wide Web，见图 ii.4）和第一个 Web 服务器（按照 UNIX 命名惯例取名为 httpd，即 HTTP Daemon，HTTP 守护进程）诞生了，它们由 Tim Berners-Lee 编写。有趣的是，尽管那个浏览器的某些能力还比不上你将在本书中实现的浏览器，⁸ 但在另一些方面，它甚至超越了现代浏览器所具备的能力。⁹ 1990 年 12 月 20 日，第一个网页 [19] 问世。即使在今天，我们在本书中要实现的浏览器也能轻松渲染这个网页。¹⁰

*图 ii.4 WorldWideWeb 浏览器的截图。（《Communications of the ACM》[21]，1994 年 8 月。）*

1991 年，Berners-Lee 在 alt.hypertext Usenet 新闻组 [20] 上宣传了他的浏览器和这一概念。

Berners-Lee 的《Web 简史》（Brief History of the Web）[22] 强调了促使万维网成为我们今天所知的 Web 的若干其他关键因素。其中一个关键因素是它的去中心化（decentralized）本质，他将这一点归因于他当时供职的 CERN [23] 的学术文化。去中心化是 Web 区别于此前及此后许多系统的关键特征，他对这一点的阐述值得在此引用（斜体为本书作者所加）：

> 显然，人们需要一个类似 Enquire¹¹ 但人人都能使用的东西。我希望它能够扩展（scale）：如果两个人各自独立地开始使用它，之后又开始合作，他们应当能把各自的信息直接链接起来，而无需做任何其他改动。*这就是 Web 的概念。*

这段话道出了 Web 的一项关键价值主张：它的去中心化本质。Web 的成功有多个原因，但它们全都与去中心化有关：

- 由于做任何事情都不存在守门人，任何人——哪怕是新手——都能轻松制作并发布简单的网页。
- 由于网页仅凭 URL 即可标识，流量可以从电子邮件、社交网络和搜索引擎等外部来源涌入 Web。此外，站点之间的兼容性和超链接的力量形成了网络效应（network effect）[24]，进一步强化了来自 Web 内部的超链接的作用。
- 由于 Web 不受任何单一实体的控制——并且通过标准组织保持这种状态——它避免了垄断控制与操纵的问题。

## ii.3 浏览器

第一个得到广泛分发的浏览器也许是 ViolaWWW [25]（见图 ii.5）；这款浏览器还开创了多项有趣的功能，例如 applet（小应用程序）和图片。它又反过来启发了 1993 年发布的 NCSA Mosaic [26]（见图 ii.6）。Mosaic 的两位最初作者之一后来联合创立了 Netscape，该公司打造了 Netscape Navigator [27]（见图 ii.7）——第一款商用浏览器，¹² 于 1994 年面世。感受到威胁 [28] 的 Microsoft 于 1995 年推出了 Internet Explorer（见图 ii.8），并很快将它与 Windows 95 捆绑。

随之而来的是"第一次浏览器大战"（first browser war）[35] 的时代：Netscape Navigator 与 Internet Explorer [36] 之间的竞争。市场上还有一些份额较小的其他浏览器，其中一个著名的例子是 Opera [37]。WebKit [38] 项目始于 1999 年；Safari [39] 以及基于 Chromium [40] 的浏览器（如 Chrome 和较新版本的 Edge [41]）都源自这一代码库。同样，Gecko [42] 渲染引擎最初由 Netscape 从 1997 年开始开发；Firefox [43] 浏览器正是源自该代码库。在第一次浏览器大战期间，本书简易浏览器的几乎所有核心功能都相继加入，包括 CSS、DOM 和 JavaScript。

*图 ii.5 ViolaWWW。（《Viola in a Nutshell》[29]）*

*图 ii.6 Mosaic。（Wikipedia [30]，CC0 1.0 [31]。）*

*图 ii.7 Netscape Navigator 1.22。（Wikipedia [32]。）*

*图 ii.8 Internet Explorer 1.0。（Wikipedia [33]，经 Microsoft [34] 许可使用。）*

据 Wikipedia 记载，"第二次浏览器大战"发生在 2004 至 2017 年 [44]，在多种浏览器之间展开，尤其是 Internet Explorer、Firefox、Safari 和 Chrome。起初，Safari 和 Chrome 使用同一个渲染引擎，但 Chrome 于 2013 年分叉出 Blink [45]，Microsoft Edge 则到 2020 年已采用了它。第二次浏览器大战期间，现代 Web 的许多功能得到了发展，包括 AJAX¹³ 请求的广泛使用、`<canvas>` 等 HTML5 特性，以及第三方 JavaScript 库和框架的爆发式增长。

## ii.4 Web 标准

与这些发展并行的是另一项同样重要的进展——Web API 的标准化。1994 年 10 月，万维网联盟（World Wide Web Consortium，W3C）[46] 成立，为 Web 特性提供监督和标准。在此之前，浏览器常常会引入新的 HTML 元素或 API，而与之竞争的浏览器不得不复制它们。有了标准组织，这些元素和 API 随后就可以协商达成一致并写入规范（specification）文档。（如今，任何新功能在推出之前都要先经过讨论、设计和规范制定。）后来，HTML 规范最终转移到了另一个名为 WHATWG [47] 的标准组织，但 CSS [48] 及其他特性仍在 W3C 标准化。JavaScript 在 TC39 [49]（ECMA [50] 下属的"第 39 技术委员会"（Technical Committee 39），又一个标准组织）标准化。HTTP [51] 由 IETF [52] 标准化。这里想说的是：20 世纪 90 年代中期建立起来的标准制定流程一直沿用至今。

在 Web 诞生的最初几年，浏览器能否保持标准化、会不会有某款浏览器最终"获胜"并变成又一个专有软件平台，当时还远不明朗。这种局面最终没有发生，原因有很多，其中包括计算机社区的平等主义风气，以及 W3C 的存在与影响力。另一个重要原因是 Web 的网络化本质：正因为如此，Web 开发者必须确保自己的页面在大多数乃至全部浏览器中都能正常工作（否则就会流失用户），这促使他们避免使用专有扩展。反过来，各浏览器也努力精确复现彼此那些未写入文档的行为——甚至是 bug——以确保自己能继续支持整个 Web。

尽管人们一度担心会发生这种事，¹⁴ 但从未有哪款浏览器真的公然试图背离标准。相反，对市场份额的激烈竞争被引导成了极快的创新速度和不断扩展的 Web API 与能力集合——如今我们称之为"Web 平台"（web platform），而不只是"万维网"。这承认了一个事实：Web 已不再是一种文档查看机制，而是演化成了一个完全成型的计算平台与生态系统。¹⁵

鉴于最终的结果——多款相互竞争的浏览器和完善的标准——回过头看，哪款浏览器在哪场浏览器"大战"中"赢"或"输"其实并不那么重要。每一场大战中获胜的都是 Web，因为它赢得了用户，能力也不断增长。

## ii.5 开源

第二次浏览器大战另一个重要而有趣的结果是：今天所有主流浏览器¹⁶ 都建立在三个开源的 Web 渲染/JavaScript 引擎之上：Chromium、Gecko 和 WebKit。¹⁷ 由于 Chromium 和 WebKit 拥有共同的祖先代码库，而 Gecko 是 Netscape 的开源后裔，这三者都可以追溯到 20 世纪 90 年代——几乎与 Web 的开端同时。这并非偶然，事实上它揭示了一件颇有意思的事情：当基于一套大众化的平台 API 来实现渲染引擎时，怎样做才是最具成本效益的方式。例如，常常有独立开发者（他们并不受雇于名义上掌控该浏览器的公司）为浏览器贡献代码和功能。甚至还有专门以实现浏览器功能为业的公司和个人！一个浏览器的某项功能复制另一个浏览器的代码，也很常见。而所有主流浏览器皆为开源这一点，又反过来作用于标准制定流程，强化了 Web 的去中心化本质。

## ii.6 小结

总结一下，这段历史的脉络如下：

1. 对信息的表示与探索方式开展了基础研究。
2. 一旦必要的技术足够成熟，严格意义上的 Web 便被提出并实现。
3. Web 相当快地流行起来，许多浏览器为了抓住 Web 带来的机遇而涌现。
4. 引入标准组织，以便在浏览器之间进行协调并避免专有控制。
5. 浏览器之间的竞争使其能力和复杂度以极快的速度增长。
6. 浏览器出现在从桌面到移动再到嵌入式的一切设备与操作系统上。
7. 最终，所有 Web 渲染引擎都实现了开源，这是对"它们是一项超越任何单一实体的共同事业"的认可。

Web 已经走过漫漫长路！但有一点似乎很清楚：它尚未完工。

## ii.7 练习

### 练习 ii.1：接下来会怎样？

根据你了解到的 Web 诞生历程及其当前形态的由来，你预测它未来演变会呈现哪些趋势？例如，你认为它能否与其他非 Web 的技术和平台有效竞争？

### 练习 ii.2：最初的构想后来怎样了？

实际运作中的 Web 与 memex 有显著差异；一个关键差异在于，Web 用户并没有一种内置的方法在页面之间添加链接或加以批注。你认为为什么会这样？你还能想到最初那些工作中其他至今尚未实现的目标吗？

## 脚注

¹ 例如，关于标准通用标记语言（Standard Generalized Markup Language，SGML [1]）以及 HTML 的其他前身，本书几乎未作介绍。（除了在这条脚注里！）

² Google 那句广为人知的使命宣言 [7]——"整合全球信息，使人人皆可访问并从中受益"——与此几乎完全一致。这并非巧合——搜索引擎这一概念与 Web 有着内在的联系，其灵感正来自 Web 及其前身的设计。

³ 他的灵感来自学术界与文学界源远流长的引用与批评传统。Xanadu 计划的研究论文在很大程度上正是以这一用例为出发点。

⁴ 其后继者"超文本编辑系统"（Hypertext Editing System）[11] 率先引入了后退按钮——如今所有浏览器都有这个按钮。由于该系统只有文本，这个"按钮"本身就是一段文本。

⁵ 那场演示甚至还不止于此。其中有些部分至今尚未在任何计算机系统中实现。去看看吧！

⁶ 如今，万维网（World Wide Web）被简称为"Web"，或者"Web 生态系统"（web ecosystem）——"生态系统"是表达"World Wide"这一概念的另一种方式。最初的措辞仍保留在许多网站域名的 "www" 之中。

⁷ 正如 Web 本身是此前雄心与梦想的实现，今天我们仍在努力实现 Web 所描绘的愿景。（不，它还没有完成！）

⁸ 没有 CSS！没有 JS！甚至连图片都没有！

⁹ 例如，第一个浏览器包含"索引页"（index page）的概念，用于在站点内进行搜索（这一痕迹至今仍体现在 URL 路径以 / 结尾时使用 "index.html" 的惯例之中），并且还有一个"所见即所得"（WYSIWYG）的网页编辑器（DOM 元素上的 "contenteditable" HTML 属性（见第 16 章）有着类似的语义行为，但内置的文件保存功能已不复存在）。如今，索引已被搜索引擎取代，而网页编辑器作为一个概念，由于当今网页渲染的高度动态特性，也已多少有些过时了。

¹⁰ 另外，你可以清楚地看到，那个网页在此期间从未更新过，至今保留着最初的风貌！

¹¹ Enquire 是一个类 Web 的数据库系统前身，同样出自 Berners-Lee 之手。

¹² 说"商用"，我指的是由营利性实体开发。Netscape 的早期版本也不是免费软件——你得去商店购买，售价约 50 美元。

¹³ 即 Asynchronous JavaScript and XML（异步 JavaScript 与 XML），其中 XML 代表 eXtensible Markup Language（可扩展标记语言）。

¹⁴ Web 最接近碎片化的时刻，也许是 20 世纪 90 年代末 DHTML [53] 相关功能的引入——那正是你将在本书中学到的文档对象模型（Document Object Model）的早期版本。Netscape 和 Internet Explorer 起初对这些功能的实现互不兼容，历经数年、在制定出共同规范并对浏览器厂商施加了相当大的压力之后，标准化才得以实现。Jay Hoffman [54] 对这段故事的叙述要深入得多，值得一读。

¹⁵ 甚至还出现过围绕 Web 构建的操作系统！例子包括曾为部分 Palm 智能手机提供动力的 webOS [55]、Firefox OS [56]（如今存活于基于 KaiOS [57] 的手机中），以及作为桌面操作系统的 ChromeOS [58]。所有这些操作系统都以 Web 作为全部应用的 UI 层，并在其上提供一些经由 JavaScript 暴露的系统集成 API。

¹⁶ 基于 Chromium 的浏览器包括 Chrome、Edge、Opera（于 2013 年从 Presto [59] 引擎转向 Chromium）、Samsung Internet、Yandex Browser、UC Browser 和 Brave。此外，还有许多基于这三种引擎之一的"嵌入式"浏览器，用于各式各样的汽车、手机、电视及其他电子设备。

¹⁷ 实际上，JavaScript 引擎位于不同的代码仓库中（许多其他子组件也是如此），它们可以也确实在浏览器之外被用作 JavaScript 虚拟机。一个重要的应用就是用 V8 [60] 来驱动 node.js [61]。不过，这三个渲染引擎各自都有对应的 JavaScript 实现，因此把两者混为一谈也说得过去。

## 链接

[1] https://en.wikipedia.org/wiki/Standard_Generalized_Markup_Language
[2] https://en.wikipedia.org/wiki/As_We_May_Think
[3] https://en.wikipedia.org/wiki/Memex
[4] https://www.flickr.com/people/79255326@N00
[5] https://commons.wikimedia.org/wiki/File:The_Memex_(3002477109).jpg
[6] https://creativecommons.org/licenses/by/2.0/legalcode
[7] https://about.google/
[8] https://en.wikipedia.org/wiki/Hypertext
[9] https://en.wikipedia.org/wiki/Hyperlink#History
[10] https://en.wikipedia.org/wiki/Project_Xanadu
[11] https://en.wikipedia.org/wiki/Hypertext_Editing_System
[12] https://commons.wikimedia.org/wiki/File:HES_IBM_2250_Console_grlloyd_Oct1969.png
[13] https://creativecommons.org/licenses/by-sa/4.0/deed.en
[14] https://en.wikipedia.org/wiki/The_Mother_of_All_Demos
[15] https://www.dougengelbart.org/content/view/374/464/
[16] http://www.cs.umd.edu/hcil/hyperties/
[17] https://cacm.acm.org/
[18] https://en.wikipedia.org/wiki/HyperCard
[19] http://info.cern.ch/hypertext/WWW/TheProject.html
[20] https://www.w3.org/People/Berners-Lee/1991/08/art-6484.txt
[21] https://dl.acm.org/doi/10.1145/179606.179671
[22] https://www.w3.org/DesignIssues/TimBook-old/History.html
[23] https://home.cern/
[24] https://en.wikipedia.org/wiki/Network_effect
[25] https://en.wikipedia.org/wiki/ViolaWWW
[26] https://en.wikipedia.org/wiki/Mosaic_(web_browser)
[27] https://en.wikipedia.org/wiki/Netscape_Navigator
[28] https://lettersofnote.com/2011/07/22/the-internet-tidal-wave/
[29] https://web.archive.org/web/20200706084621/http://viola.org/viola/book/preface.html
[30] https://commons.wikimedia.org/wiki/File:NCSA_Mosaic_Browser_Screenshot.png
[31] https://creativecommons.org/publicdomain/zero/1.0/legalcode
[32] https://en.wikipedia.org/wiki/File:Navigator_1-22.png#filehistory
[33] https://en.wikipedia.org/wiki/File:Internet_Explorer_1.0.png
[34] https://www.microsoft.com/en-us/legal/copyright/permissions
[35] https://en.wikipedia.org/wiki/Browser_wars#First_Browser_War_(1995%E2%80%932001)
[36] https://en.wikipedia.org/wiki/Internet_Explorer
[37] https://en.wikipedia.org/wiki/Opera_(web_browser)
[38] https://en.wikipedia.org/wiki/WebKit
[39] https://en.wikipedia.org/wiki/Safari_(web_browser)
[40] https://www.chromium.org/
[41] https://en.wikipedia.org/wiki/Microsoft_Edge
[42] https://en.wikipedia.org/wiki/Gecko_(software)
[43] https://en.wikipedia.org/wiki/Firefox
[44] https://en.wikipedia.org/wiki/Browser_wars#Second_Browser_War_(2004%E2%80%932017)
[45] https://en.wikipedia.org/wiki/Blink_(browser_engine)
[46] https://www.w3.org/Consortium/facts
[47] https://whatwg.org/
[48] https://drafts.csswg.org/
[49] https://tc39.es/
[50] https://www.ecma-international.org/about-ecma/history/
[51] https://tools.ietf.org/html/rfc2616
[52] https://www.ietf.org/about/
[53] https://en.wikipedia.org/wiki/Dynamic_HTML
[54] https://css-tricks.com/chapter-7-standards/
[55] https://en.wikipedia.org/wiki/WebOS
[56] https://en.wikipedia.org/wiki/Firefox_OS
[57] https://en.wikipedia.org/wiki/KaiOS
[58] https://en.wikipedia.org/wiki/Chrome_OS
[59] https://en.wikipedia.org/wiki/Presto_(browser_engine)
[60] https://en.wikipedia.org/wiki/V8_(JavaScript_engine)
[61] https://en.wikipedia.org/wiki/Node.js
