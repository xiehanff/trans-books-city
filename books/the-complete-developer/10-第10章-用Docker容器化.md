# 第 10 章 用 Docker 容器化

*Chapter 10: Containerization with Docker*

专业全栈开发人员经常要与 Docker 以及更广义的容器（container）打交道。Docker 是一个开源的容器化（containerization）平台，它解决了三个常见问题。

首先，它让我们可以为每个项目运行特定版本的软件，例如 Node.js。其次，它把开发环境与本地机器解耦，提供了一种可复现的应用运行方式。第三，与传统虚拟机不同，Docker 容器运行在共享的宿主机上。因此，它们的体积更小，内存消耗也低于经典虚拟机——后者要模拟完整的系统，而且往往依赖特定硬件。正因如此，基于容器的应用轻量且易于扩展。这些优势使 Docker 成为近年来最受推崇的开发平台。

本章介绍 Docker 的基础知识。我们首先逐步讲解容器化 Next.js 应用所需的步骤：创建一个运行最新版 Node.js 的 Docker 容器，并从容器内部对外提供应用服务。然后我们探讨微服务架构的概念，并用 Docker 创建两个微服务。

## 容器化架构

在日常工作中，开发人员经常需要在多个应用之间来回切换，而这些应用可能依赖同一个库的不同版本。例如，专注于 JavaScript 的开发人员可能每个项目都需要不同的 Node.js 或 TypeScript 版本。当然，每当要投入另一个项目时，他们可以用 nvm 之类的工具切换本地机器上安装的 Node.js 版本。但与其诉诸这些粗笨的变通办法，不如选择一个更优雅的方案。

借助 Docker，我们可以把应用或其各个服务拆分到相互独立的容器中，每个容器都提供一个针对特定服务的环境。这些容器运行在我们选择的操作系统上（通常是 Debian、Ubuntu 或 Alpine），只安装了这个特定应用所必需的依赖。容器之间彼此隔离，通过定义好的 API 通信。

在开发过程中使用 Docker 容器，能让应用后续的部署更加顺利。毕竟，容器提供了一个与位置无关、平台无关的应用版本。因此，我们已经确知应用能够在已安装的依赖下正常工作，不会出现冲突，也不需要额外的安装步骤。我们不必先在远程服务器上装好所需软件、然后再部署并测试应用，只需把 Docker 容器搬到服务器上启动即可。

当我们需要迁移到另一台服务器、扩展应用、增加额外的数据库服务器，或者把实例分布到多个地点时，Docker 让我们可以用同样简单直接的流程部署应用。我们无需管理不同的主机和各种配置，而是能够真正构建一个平台无关的应用，并在任何地方运行相同的容器。

## 安装 Docker

要检查是否已经安装了 Docker，请打开命令行并运行 `docker -v`。如果看到的版本号高于 20，就应该可以顺利完成本章的示例。否则，你需要从 Docker Inc. 安装最新版的 Docker。访问 https://www.docker.com/products/docker-desktop/，选择适合你操作系统的 Docker Desktop 安装程序并下载。运行安装程序，然后在命令行检查 Docker 版本号，它应当与你下载的版本一致。

## 创建 Docker 容器

Docker 由若干组件构成。运行 Docker 守护进程（daemon）的物理机或虚拟机称为宿主系统（host system）。在本地开发应用时，宿主机就是你的物理机；而在部署容器时，宿主机则是运行应用的那台服务器。

我们通过宿主系统上的 Docker 守护进程服务与 Docker 平台的所有组件交互。守护进程通过 API 提供 Docker 的各项功能，它就是实际安装在我们机器上的 Docker 应用。可以在命令行中使用 `docker` 命令访问守护进程。运行 `docker --help` 可以显示所有可用的交互方式。

我们使用 Docker 容器来运行容器化后的应用。这些容器是某个特定 Docker 镜像（image）的运行实例，而镜像就是包含应用本身的制品。每个 Docker 镜像都依赖一个 Dockerfile，由它定义 Docker 镜像的配置和内容。

### 编写 Dockerfile

Dockerfile 是一个文本文件，包含我们搭建 Docker 镜像所需的信息。它通常建立在某个现有的基础镜像之上，比如一台安装了额外软件的极简 Linux 机器，或是一个预先配置好的环境。例如，我们可以使用一个已装好 Node.js、MongoDB 以及所有相关依赖的 Linux 镜像。

很多时候，我们可以基于官方镜像来构建。例如，代码清单 10-1 展示了我们用来容器化重构版 Next.js 应用的基础 Dockerfile。Dockerfile 由关键字加命令构成，这里我们使用 `FROM` 关键字来选择官方的 Node.js Docker 镜像。请在项目根目录（`package.json` 文件旁边）创建一个名为 Dockerfile 的文件，并把代码清单 10-1 中的代码添加进去。

```dockerfile
FROM node:current

WORKDIR /home/node
COPY package.json package-lock.json /home/node/
EXPOSE 3000
```

*代码清单 10-1：一个适用于典型 Node.js 应用的简单 Dockerfile*

我们选择的镜像包含一个预配置好的、运行在 Debian 上的 Node.js 系统。版本标签 `current` 表示最新的 Node.js 版本；我们也可以在这里填写特定的版本号。因此，如果你需要把某个应用锁定在特定的 Node.js 版本上，就在这一行设置。你还可以使用更精简的 `node:current-slim` 镜像，它是一个轻量级 Debian 发行版，只包含运行 Node.js 所必需的软件包。但由于我们需要 MongoDB 的内存服务器（in-memory server），所以还是选择常规镜像。可用镜像列表见 https://hub.docker.com。在你的职业生涯中可能会用到的其他镜像还包括 WordPress、MySQL、Redis、Apache 和 NGINX 的镜像。

最后，我们使用 `WORKDIR` 关键字把 Docker 镜像内的工作目录设置为用户主目录。之后的所有命令都会在这个目录中执行。我们使用 `COPY` 关键字把 `package.json` 和 `package-lock.json` 文件添加到工作目录。Node.js 应用默认运行在 3000 端口，所以我们使用 `EXPORT` 关键字选择 3000 端口用于 TCP 连接。这条连接让容器外部可以访问应用。

### 构建 Docker 镜像

要从 Dockerfile 创建 Docker 镜像，我们使用 `docker image build` 命令。在构建过程中，Docker 守护进程会读取 Dockerfile 并执行其中定义的命令，以下载并安装软件、把本地文件复制进镜像，并配置好环境。在 Dockerfile 所在目录下运行以下命令，即可从中构建镜像：

```bash
$ docker image build --tag nextjs:latest .
[+] Building 11.9s (10/10) FINISHED
 => [internal] load build definition from Dockerfile                   0.1s
 => => transferring dockerfile: 136B                                   0.0s
 => [1/2] FROM docker.io/library/node:current-alpine@sha256:HASH 0.0s
 => [2/2] WORKDIR /home/node                                           0.0s
 => => naming to docker.io/library/ nextjs:latest
```

`--tag` 标志为镜像指定名称 `nextjs`，并把版本设为 `latest`。这样我们以后就能方便地引用这个特定镜像。命令末尾的句点（`.`）用来设置构建上下文（build context），它把 `docker build` 命令可访问的文件限制在当前目录内。从输出中可以看到，Docker 守护进程提示已成功构建了这个带标签的镜像。

现在，为了确认我们可以访问该镜像，请运行以下命令。这条命令会列出本地所有可用的 Docker 镜像：

```bash
$ docker image ls
REPOSITORY    TAG        IMAGE
nextjs        latest     98b28358e19a
```

和预期一样，我们新建的镜像有了一个随机 ID（98b28358e19a），标签为 `nextjs`，版本为 `latest`。Docker 守护进程还可能显示额外的信息，比如镜像的大小和创建时间，这些目前与我们无关。

Docker 还提供了其他用于管理本地和远程镜像的命令。运行 `docker image --help` 可以查看所有可用命令的列表。例如，要从本地机器上删除一个已有的镜像，使用 `docker image rm`：

```bash
$ docker image rm <name:version or ID>
```

过一段时间后，你会发现自己积累了不少不再使用或已过时的镜像版本，因此用 `docker image prune` 删除它们、为机器腾出空间是一个好习惯。

### 从 Docker 容器提供应用服务

Docker 容器是 Docker 镜像的运行实例。你可以用同一个 Docker 镜像启动多个容器，每个容器都有唯一的名称或 ID。容器运行起来之后，你可以把本地文件同步到其中。容器会在一个暴露出来的 TCP 或 UDP 端口上监听，因此你可以连接它，并使用 SSH 在容器内部执行命令。

现在来容器化我们的应用。我们将从镜像启动 Docker 容器，把本地的 Next.js 文件映射到工作目录，发布暴露的端口，最后启动 Next.js 开发服务器。所有这些都可以用 `docker container run` 完成：

```bash
$ docker container run \
--name nextjs_container \
--volume ~/nextjs_refactored/:/home/node/ \
--publish-all \
nextjs:latest npm run dev
> refactored-app@0.1.0 dev
> next dev

ready - started server on 0.0.0.0:3000, url: http://localhost:3000
event - compiled client and server successfully in 10.9s (208 modules)
```

乍一看这条命令可能有点复杂，但只要仔细研究一下，你就能轻松理解它在做什么。我们向它传入了几个标志，首先是 `--name` 标志，它为运行中的容器分配一个唯一的名称。之后我们会用这个名称来标识容器。

接着我们使用 `--volume` 标志创建一个 Docker 卷（volume）。卷是在容器之间共享数据的一种简单方式。它们由 Docker 自行管理，让我们可以把应用文件同步到容器内的 `/home/node/` 目录。我们使用 `source:destination`（源:目标）格式来定义卷；根据你的文件结构，可能需要调整这个文件夹的绝对路径。在本例中，我们把用户主文件夹下的 `/nextjs_refactored/` 映射进容器。

`--publish-all` 标志会发布所有导出的端口，并把它们分配到宿主系统上的随机端口。稍后我们会用 `docker container ls` 来查看应用的端口。最后两个参数很直观：`nextjs:latest` 指向我们要用于容器的 Docker 镜像，`npm run dev` 则像往常一样启动 Next.js 开发服务器。控制台输出表明，容器内的 Node.js 应用正在运行并监听 3000 端口。

### 定位暴露的 Docker 端口

不巧的是，当我们尝试通过 3000 端口访问 Next.js 应用时，浏览器却告诉我们无法访问——没有任何应用在监听这个端口。问题在于，我们没有把暴露的 Docker 端口 3000 映射到宿主机的 3000 端口，而是使用了 `--publish-all` 标志，给暴露的 Docker 端口分配了一个随机端口。

我们来运行 `docker container ls`，查看所有正在运行的 Docker 容器的详细信息：

```bash
$ docker container ls
CONTAINER ID   IMAGE             PORTS                     NAMES
dff681898013   nextjs:latest     0.0.0.0:55000->3000/tcp   nextjs_container
```

找到我们为容器指定的名称 `nextjs_container`，注意宿主机上的 55000 端口映射到了 Docker 的 3000 端口。因此，我们可以通过 http://localhost:55000 访问应用。在浏览器中打开这个 URL，你应该能看到 Next.js 应用。

看一眼地址栏你会发现，现在访问应用所用的端口与前面几章不同，因为它已经运行在 Docker 容器之中了。在进入下一节之前，试着访问一下我们之前创建的所有页面和 API。

### 与容器交互

运行 `docker container --help` 可以查看与容器交互的所有 Docker 命令列表。不过在大多数场景下，只需掌握其中几个就够用了。例如，使用 `exec` 可以在一个已经在运行的 Docker 容器内执行命令。我们可以给 `exec` 传入 `-it` 标志和 shell 的路径（如 `/bin/sh`），从而连接到容器内部的 shell。`-i` 标志是 `--interactive` 的缩写，而 `-t` 则会运行一个伪终端（pseudoterminal）。交互选项让我们能够与容器交互，而 tty 伪终端保持 Docker 容器持续运行，这样我们才能真正与它交互：

```bash
$ docker container exec -it <container ID or name> /bin/sh
```

`kill` 命令用于停止一个正在运行的 Docker 容器：

```bash
$ docker container kill <containerid or name>
```

我们可以按名称选择容器，也可以使用本地运行容器列表中显示的容器 ID。

## 使用 Docker Compose 创建微服务

Docker 为我们提供了一种把应用拆分成小型自治单元的方法，这些单元称为微服务（microservice）。微服务驱动的架构把应用拆分为一组自成体系的服务，它们通过定义良好的 API 相互通信。这是一个相对较新的架构概念，在 2000 年代末到 2010 年代初开始流行，当时 Docker 以及其他能够更轻松地划分和编排服务器资源的工具相继问世。这些工具构成了微服务架构的技术基础。

微服务有若干优势。首先，每个独立的服务只服务于单一目标，这降低了它的复杂度，因此也更容易测试和维护。我们还可以单独部署各个微服务，为单个微服务启动多个实例以提升其性能，或者把它整体替换掉而不影响整个应用。对比一下传统的单体应用（monolithic application）：它的用户界面、中间件和数据存储都存在于由单一代码库构建的一个程序之中。即使单体应用采用更模块化的方式，代码库也会把它们紧密耦合在一起，你无法轻松替换其中的元素。

微服务的另一个特征是，专门的团队可以只负责单个服务及其代码库。这意味着他们可以按服务为单位选择合适的工具、框架和编程语言。相比之下，编写单体应用时通常只会使用一种核心语言。

既然你已经知道如何从零创建单个容器，接下来我们将练习创建多个容器，每个容器负责应用的一部分。使用微服务的一种方式是为前端创建一个服务，再为后端创建另一个服务。我们将在第 II 部分创建的 Food Finder 应用就会采用这种结构。这种方式的主要好处是，数据库可以直接使用预配置好的 MongoDB 镜像。在本章的示例中，我们将创建第二个服务，它负责监视我们的天气服务，并在文件一有变动时就重新运行其测试套件。为此，我们将使用 Docker Compose 接口，并在 docker-compose.yml 文件中定义我们的微服务架构。

### 编写 docker-compose.yml 文件

我们在 docker-compose.yml 中定义所有服务，这是一个 YAML 格式的文本文件。这个文件还会为每个服务设置属性、依赖和卷。大多数属性都类似于创建 Docker 镜像和容器时指定的命令行标志。请在应用的根文件夹中创建该文件，并把代码清单 10-2 中的代码添加进去。

```yaml
version: "3.0"
services:
    application:
        image:
            nextjs:latest
        ports:
            - "3000:3000"
        volumes:
            - ./:/home/node/
        command:
            "npm run dev"
    jest:
        image:
            nextjs:latest
        volumes:
            - ./:/home/node/
        command:
            "npx jest ./__tests__/mongoose/weather/services.test.ts --watchAll"
```

*代码清单 10-2：一个定义了 application 服务与 Jest 服务的基础 docker-compose.yml 文件*

每个 docker-compose.yml 文件都从设置所使用的 Docker Compose 规范版本开始。根据版本不同，我们可以使用的属性和取值也不同。接着，我们在 `services` 下把每个服务定义为单独的属性。如前所述，我们要有两个服务：运行在 3000 端口上的 Next.js 应用，以及 Jest 服务——它监视我们在第 8 章创建的 services.test.ts 文件，并在文件一有改动时就重新运行测试。我们把监视命令限制为只重新测试 services 这一部分。这缩小了练习的范围，但当然，如果你愿意，也可以重新运行所有测试。

每个服务的结构大致相同。首先我们定义 Docker Compose 创建各个容器所依据的镜像。它可以是官方发行的镜像，也可以是本地构建的镜像。两个服务我们都使用最新版本的 `nextjs` 镜像。然后，我们不使用 `--publishAll` 标志，而是直接把端口从 3000 映射到 3000。这样，我们就能从宿主机的 3000 端口连接到应用的 3000 端口。

通过 `volumes` 属性，我们把宿主系统中的文件和路径同步进容器。这类似于我们在 `docker run` 命令中用过的映射，但这里的源可以使用相对路径，而不必提供绝对路径。这里我们把整个本地目录 `./` 映射到容器的工作目录 `/home/node`。和之前一样，我们可以在本地编辑 TypeScript 文件，容器内的应用始终会使用这些文件的最新版本。

到目前为止，这些属性都与我们在 `docker run` 命令中用过的命令行参数相对应。现在我们加上 `command` 属性，它指定每个容器在启动时执行的命令。对于 application 服务，我们用惯用的 `npm run dev` 命令启动 Next.js；而 Jest 服务则通过 `npx` 直接调用 Jest。提供测试文件的路径并加上 `--watchAll` 标志后，只要源代码发生变化，Jest 就会重新运行测试。

### 运行容器

使用 `docker compose up` 命令启动这个多容器应用。输出应该与下面所示类似：

```bash
$ docker compose up
 [+] Running 2/2
 ⠿ Container application-1     Created                       0.0s
 ⠿ Container jest-1  Recreated                               0.4s
Attaching to application-1, jest-1
application-1     |
application-1     | > refactored-app@0.1.0 dev
application-1     | > next dev
application-1     |
application-1     | ready - started server on 0.0.0.0:3000, URL:
application-1     | http://localhost:3000
jest-1            | PASS __tests__/mongoose/weather/services.test.ts
jest-1            |  the weather services
jest-1            |     API storeDocument
jest-1            |       ✓ returns true  (9 ms)
jest-1            |       ✓ passes the document to Model.create()  (6 ms)
jest-1            |     API findByZip
jest-1            |       ✓ returns true  (1 ms)
jest-1            |       ✓ passes the zip code to Model.findOne()  (1 ms)
jest-1            |     API updateByZip
jest-1            |       ✓ returns true  (1 ms)
jest-1            |       ✓ passes the zip code and the new data to
jest-1            |         Model.updateOne()  (1 ms)
jest-1            |     API deleteByZip
jest-1            |       ✓ returns true  (1 ms)
jest-1            |       ✓ passes the zip code Model.deleteOne()  (1 ms)
jest-1            |
jest-1            | Test Suites: 1 passed, 1 total
jest-1            | Tests:       8 passed, 8 total
jest-1            |    0 total
jest-1            | Time:        4.059 s
jest-1            | Ran all test suites matching
jest-1            |    /.\/__tests__\/mongoose\/weather\/services.test.ts/i.
```

Docker 守护进程会把所有服务都启动起来。应用一就绪，我们就能看到 Express.js 服务器的状态消息，并可以通过暴露的 3000 端口连接它。与此同时，Jest 容器会运行天气服务的测试，并报告所有测试全部成功。

### 重新运行测试

Docker 环境已经启动，现在让我们验证“监视代码变更并重新运行测试”的命令是否按预期工作。为此，我们需要修改源代码来触发 Jest。我们打开 mongoose/weather/service.ts 文件，添加一个空行然后保存文件，以此修改其内容。Jest 应该会在容器内重新运行测试，输出见代码清单 10-3。

```bash
jest-1            | Ran all test suites matching
jest-1            |    /.\/__tests__\/mongoose\/weather\/services.test.ts/i.
jest-1            |
jest-1            | PASS __tests__/mongoose/weather/services.test.ts
jest-1            |   the weather services
jest-1            |     API storeDocument
jest-1            |       ✓ returns true  (9 ms)
jest-1            |       ✓ passes the document to Model.create()  (6 ms)
jest-1            |     API findByZip
jest-1            |       ✓ returns true  (1 ms)
jest-1            |       ✓ passes the zip code to Model.findOne()  (1 ms)
jest-1            |     API updateByZip
jest-1            |       ✓ returns true  (1 ms)
jest-1            |       ✓ passes the zip code and the new data to
jest-1            |         Model.updateOne()  (1 ms)
jest-1            |     API deleteByZip
jest-1            |       ✓ returns true  (1 ms)
jest-1            |       ✓ passes the zip code Model.deleteOne()  (1 ms)
jest-1            |
jest-1            | Test Suites: 1 passed, 1 total
jest-1            | Tests:       8 passed, 8 total
jest-1            |    0 total
jest-1            | Time:        7.089 s
jest-1            | Ran all test suites matching
jest-1            |    /.\/__tests__\/mongoose\/weather\/services.test.ts/i
```

*代码清单 10-3：使用 jest --watchAll 在文件变更后重新运行测试*

所有测试依然全部通过。连接 http://localhost:3000，确认浏览器仍然能够正常渲染应用。

### 与 Docker Compose 交互

Docker Compose 提供了一套完整的、用于管理微服务应用的接口。运行 `docker compose --help` 可以查看可用命令列表。以下几个最为关键。

我们使用 `docker compose ls` 获取本地所有由 docker-compose.yml 文件定义、正在运行的 Docker 应用列表。该命令会返回应用的名称和状态：

```bash
$ docker compose ls
```

要关闭当前目录中 docker-compose.yml 文件里定义的所有正在运行的服务，运行 `docker compose kill`，它会向每个容器内的主进程发送 SIGKILL 命令：

```bash
$ docker compose kill
```

要用更温和的 SIGTERM 命令终止服务，使用以下命令：

```bash
$ docker compose down
```

这条命令并不强制关闭，而是优雅地移除 `docker compose up` 创建的所有进程、容器、网络（network）和卷（volume）。

## 总结

使用 Docker 容器化平台可以轻松部署应用并采用微服务架构。本章介绍了 Docker 生态的基本构件：宿主机、Docker 守护进程、Dockerfile、镜像和容器。借助 Docker Compose 和 Docker 卷，你把应用拆分成了一个个自成体系的服务。

要充分释放 Docker 的潜力，可以阅读官方教程 https://docs.docker.com/get-started/ 或 https://docker-curriculum.com 上的教程。在下一章中，你将开始构建 Food Finder 应用。这个全栈 Web 应用会建立在你在前面所有章节中学到的知识之上。
