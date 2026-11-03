# 第一部分 技术栈
*Part I: The Technology Stack*

---

# 第 1 章 Node.js
*Chapter 1: Node.js*

Node.js 是一个开源的运行时环境（runtime environment），可以在 Web 浏览器之外执行 JavaScript 代码。例如，你可以把它当作脚本语言（scripting language）来完成各种杂务，比如删除和移动文件、在服务器端记录日志数据，甚至创建自己的 Web 服务器（我们将在本章的实践练习中这么做）。

学会使用 Node.js，重点并不在于逐条记住命令或各个包（package），因为它依赖的是标准 JavaScript，语法和参数的细节随时可以查阅文档。真正重要的是，每位开发者都应努力理解 Node.js 生态系统（ecosystem），并善加利用。本章将带你认识它。

## 安装 Node.js

首先，在命令行中运行 node 命令，检查本地机器上是否已经装好 Node.js。版本标志（-v）应能返回当前的 Node.js 版本：

```bash
$ node -v
```

如果输出了版本号，说明 Node.js 已经安装。如果没有输出，或者版本低于 https://nodejs.org 上列出的当前推荐稳定版本，你就应该安装这个稳定版本。

要在本地安装 Node.js，请访问 https://nodejs.org/en/download 并选择适用于你操作系统的安装程序。我建议安装 Node.js 的长期支持（long-term support，LTS）版本，因为许多 Node.js 模块都要求使用这一版本。运行 Node.js LTS 与 npm 的安装包，然后再次检查版本号，它应该与你刚安装的版本一致。

接下来，我们将了解 Node.js 运行环境的基本命令和特性。如果你不想安装 Node.js，也可以在 https://codesandbox.io/s/new 和 https://stackblitz.com 这些在线演练场（playground）中运行本章的 Node.js 命令行示例和 JavaScript 代码。

## 使用 npm

Node.js 默认的包管理器（package manager）是 npm。你可以在其中找到满足各种任务需求的模块（module），它们都来自 https://www.npmjs.com 这个在线注册表（registry）。在命令行中运行以下命令，确认本地机器上 npm 可用：

```bash
$ npm -v
```

如果没有显示版本号，或版本低于当前发行版，请安装最新的 Node.js LTS 版本，其中包含 npm。

请注意，https://www.npmjs.com 上没有任何审查流程或质量控制。任何人都可以发布包，网站依靠社区来举报恶意或有缺陷的包。

运行以下命令会列出所有可用的命令：

```bash
$ npm
```

注意：npm 最流行的替代品是 yarn，它同样使用 https://www.npmjs.com 注册表，并且与 npm 完全兼容。

## package.json 文件

package.json 文件是每个基于 Node.js 的项目的关键组成部分。node_modules 文件夹存放的是实际代码，而 package.json 文件保存着项目的全部元数据（metadata）。它位于项目根目录下，必须包含项目名称和版本号；此外还可以包含可选信息，比如项目描述、许可证（license）、脚本（scripts）等更多细节。

我们来看一下你将在练习 1（第 13 页）中创建的 Web 服务器的 package.json 文件。它应该与清单 1-1 所示的文件类似。

```json
{
    "name": "sample-express",
    "version": "1.0.0",
    "description": "sample express server",
    "main": "index.js",
    "scripts": {
        "test": "echo \"Error: no test specified\" && exit 1",
        "run": "node index.js"
    },
    "author": "",
    "license": "ISC",
    "dependencies": {
        "express":"^4.18.2"
    }
}
```

*清单 1-1：练习 1 中 Express.js 服务器项目的 package.json 文件*

package.json 文件包含了其他人在自己机器上安装所需模块并运行应用所需的全部信息。因此，你永远不必在代码仓库（repository）中发布或存储 node_modules 文件夹，从而把仓库体积降到最低。下面我们来详细了解 package.json 文件。

### 必填字段

package.json 文件必须包含 name 字段和 version 字段，其余字段都是可选的。name 字段存放包的名称，必须是一个小写单词，但可以包含连字符和下划线。

version 字段必须遵循语义化版本控制（semantic versioning）规范，其格式为 major.minor.patch，例如 1.2.3。之所以叫语义化版本，是因为每个数字都有各自的含义：主版本号（major）意味着引入了不兼容的 API 变更。一般来说，切换到另一个主版本时要格外小心，因为无法保证应用能毫无问题地运行。次版本号（minor）的变更以向后兼容（backward-compatible）的方式添加新功能，因此不会给你的应用带来问题。修订号（patch）用于向后兼容的缺陷修复，你应该始终保持更新到最新。

注意：你可以在 https://semver.org 上进一步了解语义化版本控制，以及如何定义各种版本范围。

### 依赖

最重要的可选字段用于指定依赖（dependencies）和开发依赖（development dependencies）。dependencies 字段列出运行项目所需的全部依赖及其要求的版本范围，遵循语义化版本语法。默认情况下，npm 只约束主版本号，让次版本号和修订号的范围保持灵活。这样，npm 总能用最新的兼容版本来初始化你的项目。

这些依赖是你打包的应用程序的一部分。当你在新机器上安装一个项目时，package.json 文件中列出的所有依赖都会被安装，并放入 package.json 旁边的 node_modules 文件夹。

你的应用可能需要各种各样的依赖，比如框架（framework）和辅助模块。例如，我们将在第二部分构建的 Food Finder 应用至少要包含 Next.js 作为单页应用（single-page application）框架，以及配合 MongoDB 使用的 Mongoose 作为数据库层。

### 开发依赖

devDependencies 字段列出开发项目所需的全部依赖及其版本。同样，这里也只固定主版本号。这些依赖只在开发时需要，运行应用时并不需要。因此，打包脚本会忽略它们，它们也不会成为已部署应用的一部分。当你在新机器上安装项目时，package.json 文件中列出的所有开发依赖都会被安装，并放入 package.json 旁边的 node_modules 文件夹。对我们的 Food Finder 应用来说，开发依赖将包括 TypeScript 的类型定义（type definitions）。其他常见的条目还有测试框架、代码检查工具（linter），以及 webpack 和 Babel 之类的构建工具。

## package-lock.json 文件

npm 包管理器会为每个项目自动生成 package-lock.json 文件。这个锁定文件（lock file）解决了依赖采用语义化版本控制带来的一个问题。如前所述，npm 默认只定义主版本号，并使用可用的最新次版本和修订版本。这虽然能确保你的应用包含最新的缺陷修复，却也带来了新问题：没有精确的版本号，构建（build）就无法复现。由于 npm 注册表没有质量控制，哪怕一次修订版本或次版本的更新，也可能引入本应属于主版本变更的不兼容 API 改动。因此，版本之间的细微偏差就可能导致构建失败。

package-lock.json 文件通过记录每个包及其依赖的确切版本来解决这个问题。这个文件通常相当大，但对于你将在本章末尾创建的 Web 服务器来说，其中的条目与清单 1-2 类似。

```json
{
    "name": "sample-express",
    "lockfileVersion": 2,
    "requires": true,
    "packages": {
        "": {
            "dependencies": {
                "express": "^4.18.2"
            }
        },
        "node_modules/accepts": {
            "version": "1.3.8",
            "resolved": "https://registry.npmjs.org/accepts/-/accepts-1.3.8.tgz",
            "integrity": "sha512-PYAthTa2m2VKxuvSD3DPC/Gy+U+sOA1LAuT8mkmRuvw+NACSaeXEhosdQ==",
            --snip--
        },
        --snip--
        "node_modules/express": {
            "version": "4.18.2",
            "resolved": "https://registry.npmjs.org/express/-/express-4.18.2.tgz",
            "integrity": "sha512-5/PsL6iGPdfQ/lKM1UuielYgv3BUoJfz1aUwU9vHZ+J7gyvwdQXFEBIEI==",
            "dependencies": {
                "accepts": "~1.3.8",
                --snip--
                "vary": "~1.1.2"
            },
            "engines": {
                "node": ">= 0.10.0"
            }
        },
        --snip--
        "vary": {
            "version": "1.1.2",
            "resolved": "https://registry.npmjs.org/vary/-/vary-1.1.2.tgz",
            "integrity": "sha512-BNGbWLfd0eUPabhkXUVm0j8uuvREyTh5ovRa/dyow/BqAbZJyC+bfhskkh=="
        }
    }
}
```

*清单 1-2：练习 1 的 package-lock.json 文件*

锁定文件包含对项目的引用，并列出对应 package.json 文件中的信息，然后列出项目的全部依赖；对我们来说，唯一的依赖就是固定了版本的 Express.js。（我们将在练习 1 中介绍 Express.js。）此外，该文件还列出了所用 Express.js 版本的全部依赖，这里是 accepts 和 vary 包。存储的构件（artifact）的 SHA 哈希值让 npm 能够在下载资源后验证其完整性。

现在，所有模块的版本都被锁定，每次运行 npm install 命令都会精确复刻出最初的安装配置。与 package.json 一样，package-lock.json 文件也是代码仓库的一部分。

## 创建项目

下面按照创建和维护项目的合理顺序，介绍日常工作里最重要的命令。完成这些步骤后，你将得到一个 package.json 文件和一个可用于生产环境的项目文件夹，其中安装了一个包：Express.js。

### 初始化新模块或项目

要启动一个新项目，运行 npm init 来初始化一个新模块。它会启动一个交互式向导，你将根据自己的输入逐步填写项目的 package.json 文件：

```bash
$ mkdir sample-express
$ cd sample-express
$ npm init
This utility will walk you through creating a package.json file.
It only covers the most common items, and tries to guess sensible defaults.
--snip--
Is this OK? (yes)
```

每个项目开始时，你都需要在一个空文件夹（这里用 mkdir sample-express 创建）中用 npm init 初始化一个新的 Node.js 环境。为简单起见，保留向导给出的默认建议即可。该向导会在项目文件夹中创建一个基础的 package.json 文件，它应该与清单 1-3 类似。

```json
{
    "name": " sample-express",
    "version": "1.0.0",
    "description": "",
    "main": "index.js",
    "scripts": {
      "test": "echo \"Error: no test specified\" && exit 1"
    },
    "author": "",
    "license": "ISC"
}
```

*清单 1-3：默认的 package.json 文件*

把这个文件与清单 1-1 所示的文件对比，可以发现二者相当相似，差别只在于依赖和开发依赖。package.json 文件准备好之后，我们现在可以用 npm install 安装这些依赖了。

### 安装依赖

Node.js 提供的模块可用于访问文件系统的输入输出、使用网络协议（如 DNS、HTTP、TCP、TLS/SSL 和 UDP）以及处理二进制数据等任务。它还提供密码学模块、处理数据流（stream）的接口等更多功能。

运行 npm install <package> 会下载指定的包，放入 package.json 文件旁边的 node_modules 文件夹，并把它添加到 package.json 的 dependencies 列表中。每当你需要添加运行应用所需的新模块时，都应使用这条命令。

假设你想创建一个基于 Express.js 的新服务器，就需要从 https://npmjs.com 安装 Express.js 包。这里我们安装一个特定版本；如果要安装最新版本，省略版本号，直接使用 npm install express 即可：

```bash
$ npm install express@4.18.2
added 57 packages, and audited 58 packages in 1s
found 0 vulnerabilities
```

现在 node_modules 文件夹中多了一个 express 文件夹，以及存放其依赖的其他文件夹。同时，Express.js 也作为依赖列入了 package.json，如清单 1-4 所示。

```json
{
    "name": " sample-express",
    "version": "1.0.0",
    "description": "",
    "main": "index.js",
    "scripts": {
      "test": "echo \"Error: no test specified\" && exit 1"
    },
    "author": "",
    "license": "ISC",
    "dependencies": {
      "express": "^4.18.2"
    }
}
```

*清单 1-4：将 Express.js 列为依赖的默认 package.json 文件*

我们已成功把 Express.js 添加为依赖。

### 安装开发依赖

再假设你想用一个叫 karma 的包对服务器进行端到端测试（end-to-end testing）。这个包不像 Express.js 那样属于依赖，它只在开发期间使用，运行实际应用时并不需要。

这种情况下，你应该运行 npm install --save-dev package 来下载该包，并把它添加到本地 package.json 文件的 devDependencies 列表中：

```bash
$ npm install --save-dev karma@5.0.0
added 128 packages, and audited 186 packages in 3m
9 vulnerabilities (1 moderate, 4 high, 4 critical)

To address issues that do not require attention, run:
  npm audit fix

To address all issues (including breaking changes), run:
  npm audit fix --force

Run `npm audit` for details.
```

注意，安装 karma 包之后，npm 提示该版本存在已知漏洞（vulnerability）。尽管如此，它还是被加入了 node_modules 文件夹，并在 package.json 中列为 devDependency。我们稍后会按照提示修复这些问题。

### 审计 package.json 文件

安装过程中，npm 提示 karma 存在漏洞，我们来验证一下。npm audit 命令会检查本地 package.json 文件，查找所有已知漏洞：

```bash
$ npm audit

# npm audit report
--snip--
karma  <=6.3.15
Severity: high
Open redirect in karma - https://github.com/advisories/GHSA-rc3x-jf5g-xvc5
Cross-site Scripting in karma - https://github.com/advisories/GHSA-7x7c-qm48-pq9c
Depends on vulnerable versions of log4js
Depends on vulnerable versions of ua-parser-js
fix available via `npm audit fix --force`
Will install karma@6.4.1, which is a breaking change
--snip--
9 vulnerabilities (1 moderate, 4 high, 4 critical)

To address issues that do not require attention, run:
  npm audit fix

To address all issues (including breaking changes), run:
  npm audit fix --force
```

这条命令会返回一份详细报告，说明每个问题包的版本和严重程度，并汇总当前安装的 Node.js 模块中发现的全部问题。

npm 包管理器还提示，这些问题可以用 npm audit fix 自动修复。不过它也警告我们，最新的 karma 版本包含破坏性变更（breaking change）。为了接受这些变更，我们需要使用 --force 标志。我建议每隔几个月运行一次 npm audit，同时配合 npm update，以免使用过时的依赖、埋下安全隐患：

```bash
$ npm audit fix --force
added 13 packages, removed 41 packages, changed 27 packages, and audited 158 packages in 5s
```

现在可以看到 package.json 的 devDependencies 列表已是最新版 karma，再运行一次 npm audit 也显示已安装的包不再有已知漏洞。

### 清理 node_modules 文件夹

运行 npm prune 会检查本地 package.json 文件，与本地 node_modules 文件夹对比，并移除所有多余的包。在开发过程中增删包之后，或者做日常清理时，都应该使用这条命令。

我们来检查一下刚才的修复操作有没有安装多余的包：

```bash
$ npm prune
up to date, audited 136 packages in 1s

found 0 vulnerabilities
```

输出看起来没问题，我们的包没有发现任何异常。

### 更新所有包

运行 npm update 会把所有已安装的包更新到各自允许的最新版本。你应经常使用这条命令，避免依赖过时带来安全风险：

```bash
$ npm update
added 1 package, removed 1 package, changed 1 package, and audited 158 packages in 8s

found 0 vulnerabilities
```

如你所见，npm update 会显示更新摘要。

### 移除依赖

运行 npm uninstall package 会从本地 node_modules 文件夹和 package.json 文件中移除该包及其依赖。不再需要的模块就该用这条命令删除。假设你决定不再用 karma 做端到端测试：

```bash
$ npm uninstall karma
removed 71 packages, and audited 138 packages in 3s

found 0 vulnerabilities
```

命令输出显示了对 node_modules 文件夹所做的改动，同时该包也从 package.json 中被移除。

### 恢复安装依赖

运行 npm install 会从 npm 注册表下载所有依赖和开发依赖，并放入 node_modules 文件夹。在新机器上安装现有项目时使用这条命令。例如，要在新文件夹中安装一份 Express.js 项目副本，你可以新建一个空文件夹，只把 package.json 和 package-lock.json 两个文件复制进去，然后在该文件夹中运行 npm install 命令：

```bash
$ npm install
added 137 packages, and audited 138 packages in 3s

found 0 vulnerabilities
```

每当你克隆（clone）代码仓库，或者基于 package.json 文件创建新项目时，都应运行 npm install。与前面的所有命令一样，npm 会显示状态报告，列出存在的漏洞。

### 使用 npx 执行一次性脚本

安装 Node.js 时，npx 也一并装好了，它是 node package execute 的缩写。这个工具让你无需预先安装，就能执行注册表中的任何包。当你只需要运行一次某段代码时，它非常有用。比如，你可能要用一个初始化项目的脚手架（scaffolding）脚本，它既不是依赖也不是开发依赖。

npx 的工作方式是：先检查你要运行的可执行文件能否通过 $PATH 环境变量或本地项目二进制文件找到；如果找不到，npx 会把包安装到中央缓存（cache），而不是本地 node_modules 文件夹。假设你想检查 package.json 有没有语法错误，可以使用 jsonlint 包。由于这个包既不是运行项目所必需的，也不属于你的开发流程，你不会想把它装进 node_modules 文件夹：

```bash
$ npx jsonlint package.json
Need to install the following packages:
  jsonlint
Ok to proceed? (y) y
{
    "name": " sample-express",
    "version": "1.0.0",
    "description": "",
    "main": "index.js",
    "scripts": {
        "test": "echo \"Error: no test specified\" && exit 1"
    },
    "author": "",
    "license": "ISC",
    "dependencies": {
        "express": "^4.18.2"
    }
}
```

这条命令通过 npx 调用 jsonlint 来校验我们的 package.json 文件。npx 先把包安装到全局缓存文件夹，然后运行 jsonlint。它打印出 package.json 文件的内容，且未报告任何错误。查看你的 node_modules 文件夹，里面并没有安装 jsonlint。但在之后每次调用 npx 时，你都会发现 jsonlint 是可用的。

## 实践练习 1：构建"Hello World" Express.js 服务器

Express.js 是一个构建在 Node.js 之上的免费开源后端（backend）框架。它专为构建 Web 应用和 API 而设计，是 Node.js 生态系统中事实上的标准服务器框架，也是全栈（full-stack）Web 开发的基础。

Express.js 提供了 HTTP 服务器常用的中间件（middleware），可用于缓存、内容协商（content negotiation）、Cookie 处理、跨源请求（cross-origin request）处理、重定向等众多任务。

注意：Next.js 使用自己的内置服务器，它在很大程度上借鉴了 Express.js。对于你将在本书第二部分构建的 Food Finder 应用，Next.js 将是你所用中间件的基础。由于 Next.js 为你抽象了这些中间件，在那里你不需要直接与服务器打交道。

下面我们来构建一个基于 Express.js 的简单 Node.js 服务器，练一练 Node.js 技能。

### 准备工作

如果你已经跟着本章内容创建了 sample-express 文件夹和 package.json 文件，可以跳过这一步。否则，请新建一个名为 sample-express 的文件夹并切换进去。然后，在命令行中运行 npm init 来初始化一个新的 Node.js 项目。交互式向导会询问一些细节，比如应用的名称和版本。暂时接受默认值即可。

接下来，你要用 Express.js 包作为服务器的基础。运行 npm install express@4，安装主版本 4 的最新发行版。你会看到 package.json 文件中已将 express 列为依赖。

### 编写服务器代码

在 sample-express 文件夹中创建 index.js 文件，加入清单 1-5 中的代码。

```javascript
const express = require('express');
const server = express();
const port = 3000;

server.get('/hello', function (req, res) {
    res.send('Hello World!');
});

server.listen(port, function () {
    console.log('Listening on ' + port);
});
```

*清单 1-5：一个基础的 Express.js 服务器*

首先，我们把 express 包加载进文件，实例化应用，并定义一个表示所用端口的常量。接着为服务器创建一个路由（route），让它对发送到 /hello 基础 URL 的每个 GET 请求都用 Hello World! 作出响应。我们使用 Express.js 的 get 方法，把 /hello 作为第一个参数，把一个回调函数（callback function）作为第二个参数。此后，对发送到 /hello 端点（endpoint）的每个 GET 请求，服务器都会运行这个回调函数，把 Hello World! 作为响应发送给浏览器。最后，我们用 Express.js 的 listen 方法启动 Web 服务器，让它在 3000 端口上监听。

在命令行中启动服务器：

```bash
$ node index.js
Listening on 3000
```

现在在浏览器中访问 http://localhost:3000/hello，你应该能看到 Hello World! 消息。恭喜！你刚刚用 JavaScript 写出了自己的第一个 Node.js Web 服务器。

## 小结

本章教你如何借助 Node.js 及其模块生态系统，在浏览器之外运行 JavaScript 代码。你学会了如何用 npm 命令在全栈应用中使用、添加和移除模块，也学会了如何阅读和使用 package.json 与 package-lock.json 文件。最后，你初步认识了全栈开发的事实标准服务器 Express.js，并用短短几行代码构建了一个示例 Node.js 服务器。

本章只是浅尝了 Node.js。如果你想挖掘它的全部潜力，我推荐 W3Schools 的 Node.js 教程（https://www.w3schools.com/nodejs/），以及 https://www.udemy.com/course/expressjs-fundamentals/ 上的免费课程 ExpressJS Fundamentals。

下一章将带你认识 JavaScript 的最新版本 ES.Next，并掌握它带来的各种现代特性。
