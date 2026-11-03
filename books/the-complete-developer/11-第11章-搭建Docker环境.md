# 第二部分 全栈应用
*Part II: The Full-Stack Application*

---

# 第 11 章 搭建 Docker 环境
*Chapter 11: Setting Up the Docker Environment*

在本书的这一部分，你将运用迄今为止学到的知识，从零开始构建一个全栈应用。前面的章节已经讲解了技术栈的各个组成部分，而接下来的章节将更加深入地聚焦于代码本身。

本章将介绍你要构建的应用，并带你一步步使用 Docker 配置环境。我建议你在开始编写代码之前先阅读前面的章节，不过真正的硬性要求只有一个：在继续之前，你已经安装并启动了 Docker。具体做法可参考第 10 章。

> **NOTE**
>
> 你可以从 http://www.usemodernfullstack.dev/downloads/food-finder 下载 Food Finder 应用的完整源代码，并从 http://www.usemodernfullstack.dev/downloads/assets 下载一个仅包含所需资源的 ZIP 文件。

## Food Finder 应用

Food Finder 应用展示一个餐厅及其位置的列表。用户可以点击这些列表项，查看每个位置的更多详情。此外，用户还可以借助 OAuth 使用自己的 GitHub 账号登录应用，从而维护一份心仪位置的愿望清单（wish list）。

在幕后，我们将用 TypeScript 编写这个简单的单页应用（single-page application）。搭建好本地环境之后，我们会用 Next.js、Mongoose 和 MongoDB 构建后端与中间件（middleware），并向 MongoDB 填充初始数据。然后我们会加入 GraphQL，对外暴露一个 API 层，通过它访问用户的愿望清单。前端部分，我们将运用关于 React 组件、Next.js 页面和路由的知识来构建。我们还会通过 next-auth 添加 OAuth 授权流程，让用户能够用 GitHub 登录。最后，我们将用 Jest 编写自动化测试，以验证应用的完整性与稳定性。

## 用 Docker 搭建本地环境

Docker 将开发环境与我们的本地机器解耦。我们将用它为应用的每个部分创建自包含的服务（service）。在 docker-compose 文件中，我们会添加两个服务：一个用于提供 MongoDB 数据库的后端，另一个用于运行承载前端和中间件的 Next.js 应用。

开发工作从一个新的空文件夹 `code` 开始。这个文件夹将作为应用的根目录，容纳 Food Finder 应用的全部代码。本章稍后我们会用 create-next-app 辅助命令向其中添加文件。

接下来，在这个根文件夹中创建一个空的 docker-compose.yml 文件和一个 `.docker` 文件夹。我们将在该文件中定义环境所需的两个服务，并在该文件夹中存放创建容器（container）所需的种子数据（seed data）。

### 后端容器

后端容器只提供应用的 MongoDB 实例，仅此而已。因此，我们可以直接使用 Docker registry 中的官方 MongoDB 镜像（image）——Docker 会自动下载它——而无需创建自定义的 Dockerfile。

#### 填充数据库

我们希望 MongoDB 启动时就带有一个预先填充了有效初始数据集的数据库。这个过程称为数据库填充（seeding），我们可以把种子脚本 seed-mongodb.js 在启动时复制到容器的 /docker-entrypoint-initdb.d/ 目录来实现自动化。只要容器在启动时其 /data/db 目录中没有数据，MongoDB 镜像就会针对 MONGO_INITDB_DATABASE 环境变量（environment variable）所定义的数据库执行该文件夹中的脚本。

在 `.docker` 文件夹中新建一个名为 foodfinder-backend 的文件夹，然后把之前下载的 assets.zip 中的 seed-mongodb.js 文件复制到这个新建文件夹里。种子文件的内容应当与 代码清单 11-1 类似。

```javascript
db.locations.insert([
    {
        address: "6220 Avenue U",
        zipcode: "NY 11234",
        borough: "Brooklyn",
        cuisine: "Cafe",
        grade: "A",
        name: "The Roasted Bean",
        on_wishlist: [],
        location_id: "56018",
    },
--snip--
    {
        address: "405 Lexington Avenue",
        zipcode: "NY 10174",
        borough: "Manhattan",
        cuisine: "American",
        grade: "A",
        name: "The Diner At The Corner",
        on_wishlist: [],
        location_id: "63426",
    }
]);

```

*代码清单 11-1：seed-mongodb.js 文件*

可以看到，该脚本直接与我们将在下一节搭建的 MongoDB 实例中的集合（collection）交互。我们使用 MongoDB 的 insert 方法，用这些文档填充数据库的 location 集合。注意，我们使用的是原生 MongoDB 驱动来插入文档，而不是 Mongoose。这样做是因为默认的 MongoDB Docker 镜像中没有安装 Mongoose，而插入文档本身是一项相对简单的任务。虽然填充数据库时没有使用 Mongoose，但我们插入的文档必须与稍后用 Mongoose 定义的模式（schema）相匹配。

#### 创建后端服务

现在我们可以在 Docker 配置中定义后端服务了。把 代码清单 11-2 中的代码添加到之前创建的空 docker-compose.yml 文件中。

```yaml
version: "3.0"
services:
    backend:
        container_name: foodfinder-backend
        image: mongo:latest
        restart: always
        environment:
            DB_NAME: foodfinder
            MONGO_INITDB_DATABASE: foodfinder
        ports:
            - 27017:27017
        volumes:
            - "./.docker/foodfinder-backend/seed-mongodb.js:
/docker-entrypoint-initdb.d/seed-mongodb.js"
            - mongodb_data_container:/data/db

volumes:
    mongodb_data_container:

```

*代码清单 11-2：包含后端服务的 docker-compose.yml 文件*

我们首先定义容器的名称，以便日后方便地引用它。如前所述，我们使用官方 MongoDB 镜像的最新版本，并指定该容器一旦停止就始终自动重启。接下来，我们用环境变量定义将与 MongoDB 一起使用的集合。这里定义了两个：DB_NAME 指向我们将配合 Mongoose 使用的集合，MONGO_INITDB_DATABASE 指向种子脚本所用的集合。/docker-entrypoint-initdb.d/ 中的脚本默认使用后者。

我们希望这个脚本填充的是应用所用的数据库，因此把两个变量都设为同一个名称 foodfinder，这样我们就得到了一个为 Mongoose 模型预先填充好数据的数据库。

然后，我们把容器内部的 27017 端口映射并暴露到主机的 27017 端口，这样应用就能通过 mongodb://backend:27017/foodfinder 访问 MongoDB 实例。注意，这个连接字符串包含服务名、端口和数据库名。稍后我们会把这个连接字符串存入环境变量，并从中间件用它连接数据库。最后，我们把种子脚本映射并复制到配置位置，并把 /data/db 中的数据库数据保存到 Docker 卷（volume） mongodb_data_container 中。由于我们想把字符串拆成两行书写，按照 YAML 的书写惯例，需要用双引号（"）把它包裹起来。

现在，用 docker compose up 完成 Docker 环境的搭建：

```bash
$ docker compose up
[+] Running 2/2
 ⠿ Network foodfinder_default                      Created                 0.1s
 ⠿ Container foodfinder-backend                    Created                 0.3s
Attaching to foodfinder-backend

foodfinder-backend  | /usr/local/bin/docker-entrypoint.sh: running /docker
                    /entrypoint-initdb.d/seed-mongodb.js

```

输出表明，Docker 守护进程（daemon）成功创建了 foodfinder-backend 容器，并且种子脚本已在启动期间执行。我们无须经历在本地安装和维护 MongoDB 的麻烦，也无须寻找免费或低价的云端实例，仅凭 docker-compose 文件中的几行代码，就把 MongoDB 加入了项目。

用 Ctrl-C 停止容器，再用 docker compose down 移除它：

```bash
$ docker compose down
[+] Running 2/2
 ⠿ Container foodfinder-backend                     Removed                 0.0s
 ⠿ Network foodfinder_default                       Removed

```

现在我们可以添加前端容器了。

### 前端容器

现在我们来为前端和中间件创建容器化的基础设施。我们的做法与第 5 章一样，使用 create-next-app 搭建 Next.js 应用的脚手架，并依靠官方的 Node.js Docker 镜像把应用与任何本地 Node.js 安装解耦。

由于所有与 Node.js 相关的命令都会在这个容器内执行，严格来说，本地机器甚至根本不需要安装 Node.js，我们也不必确保所用的 Node.js 版本符合 Next.js 的要求。此外，npm 安装的软件包可能会针对其运行所在的操作系统进行优化，因此在容器内使用 npm，可以确保 npm 安装的是适用于 Linux 的正确版本。

尽管如此，我们还是希望 Docker 把 Node.js 模块文件夹同步到本地系统，这样 IDE 就能自动使用已安装的依赖，比如 TypeScript 编译器和 ESLint。我们先来创建一个最小化的 Dockerfile。

#### 创建应用服务

我们把合并的前端与中间件服务加入 Docker 配置：把 代码清单 11-3 中的代码放入项目 docker-compose.yml 文件的 services 属性中。

```yaml
--snip--
services:

    application:
        container_name: foodfinder-application
        image: node:lts-alpine
        ports:
            - "3000:3000"
        volumes:
            - ./code:/home/node/code
        working_dir: /home/node/code/
        depends_on:
            - backend
        environment:
            - HOST=0.0.0.0
            - CHOKIDAR_USEPOLLING=true
            - CHOKIDAR_INTERVAL=100
        tty: true
    backend:
--snip--

```

*代码清单 11-3：包含后端服务与应用服务的 docker-compose.yml 文件*

Food Finder 应用的服务与后端服务遵循相同的结构。首先设置容器的名称，然后为这个特定服务定义所用的镜像。后端服务使用的是官方 MongoDB 镜像，而这里我们使用运行在 Alpine Linux 上的官方 Node.js 镜像的当前 LTS 版本——Alpine Linux 是一个轻量级 Linux 发行版，其内存占用远低于基于 Debian 的镜像。

接着，我们暴露并映射 3000 端口，使应用可以通过 http://localhost:3000 访问，并把本地应用的代码目录映射进容器。然后，我们把工作目录设置为 code 目录。我们声明该容器需要一个正在运行的后端服务，因为 Next.js 应用需要一个可用的 MongoDB 实例连接。此外，我们还添加了环境变量；特别是，chokidar 为 Next.js 代码提供热重载（hot-reloading）支持。最后，把 tty 属性设为 true，容器就会提供一个交互式 shell 而不是直接关闭。我们之后要靠这个 shell 在容器内执行命令。

#### 安装 Next.js

两个服务都就位后，现在可以在容器内安装 Next.js 了。为此，需要用 docker compose up 启动容器：

```bash
$ docker compose up

[+] Running 3/3
 ⠿ Network foodfinder_default                      Created                 0.1s
 ⠿ Container foodfinder-backend                    Created                 0.3s
 ⠿ Container foodfinder-application                Created                 0.3s
Attaching to foodfinder-application, foodfinder-backend
--snip--
foodfinder-application  | Welcome to Node.js ...
--snip--

```

把这份命令行输出与之前 docker compose up 的输出比较一下。你会看到应用容器成功启动，并且其中运行着一个 Node.js 交互式 shell。

现在我们可以使用 docker exec 在运行中的容器内执行命令。这样做有两大好处：其一，本地机器不需要任何特定版本的 Node.js（甚至完全不需要安装 Node.js）；其二，我们在 Node.js Linux Alpine 镜像上运行 Node.js 应用和 npm 命令，这样依赖包会针对 Alpine 优化，而不是针对我们的宿主系统（host system）。

要在容器内运行 npm 命令，请使用 docker exec -it foodfinder-application，后面跟上要执行的命令。Docker 守护进程会连接到容器内的终端，并在应用容器的工作目录 /home/node/code（即我们之前设置的目录）中执行给定的命令。我们用第 5 章讨论过的 npx 命令在这个目录中安装 Next.js 应用：

```bash
/home/node/code# docker exec -it foodfinder-application \
npx create-next-app@latest foodfinder-application \
--typescript --use-npm
Need to install the following packages:
  create-next-app
Ok to proceed? (y)
✔ Would you like to use ESLint with this project? ... No / Yes
Creating a new Next.js app in /home/node/code/foodfinder-application.

Success! Created foodfinder-application at /home/node/code/foodfinder-application

```

我们把项目名设为 foodfinder-application，并接受默认选项。其余输出你应该已经熟悉了。

脚手架搭建完成后，就可以用 npm run dev 启动 Next.js 应用了。在浏览器中访问 http://localhost:3000，你应该能看到熟悉的 Next.js 欢迎页面。foodfinder-application 文件夹应已映射到本地 code 文件夹中，因此我们可以在本地编辑与 Next.js 相关的文件。

#### 调整应用服务以支持重启

目前，每次通过 docker compose up 重启之后，连接应用容器都需要执行一次 docker exec，然后手动调用 npm run dev。我们对应用服务做两处小调整，让这一流程更加便捷。请修改文件，使其与 代码清单 11-4 一致。

```yaml
--snip--
services:
--snip--
    application:
--snip--
        volumes:
            - ./code:/home/node/code
        working_dir: /home/node/code/foodfinder-application
--snip--
        command: "npm run dev"
--snip--

```

*代码清单 11-4：自动启动 Next.js 的 docker-compose.yml 文件*

第一处是修改 working_dir 属性。因为我们使用的是 Next.js，所以把它设置为 Next.js 应用的根目录 /home/node/code/foodfinder-application，其中包含 package.json 文件。第二处是添加 command 属性，值为 npm run dev。完成这两处修改后，每次调用 docker compose up 都应立即启动 Next.js 应用。试着用 docker compose up 启动容器，控制台输出应显示 Next.js 正在运行，并且可通过 http://localhost:3000 访问：

```bash
$ docker compose up
[+] Running 3/3
 ⠿ Network foodfinder_default                      Created    0.1s
 ⠿ Container foodfinder-backend                    Created    0.3s
 ⠿ Container foodfinder-application                Created    0.3s
Attaching to foodfinder-application, foodfinder-backend
foodfinder-application  |
foodfinder-application  | > foodfinder-application@0.1.0 dev
foodfinder-application  | > next dev
foodfinder-application  |
foodfinder-application  | ready - started server on 0.0.0.0:3000,
foodfinder-application  | url: foodfinder-application  | http://localhost:3000
foodfinder-application  | info  - Loaded env from /home/node/code/foodfinder-
foodfinder-application  | application/.env.local

```

此时在浏览器中访问 http://localhost:3000，无须手动启动 Next.js 应用，你就应该能看到 Next.js 欢迎页面。

请注意，如果你在 Linux 或 macOS 上使用的不是管理员或 root 用户，就需要调整应用服务和启动命令。由于 Docker 守护进程默认以 root 用户身份运行，它创建的所有文件都需要 root 权限才能访问，而你的普通用户没有这些权限，无法访问这些文件。为避免这些潜在问题，请修改配置，让 Docker 守护进程把文件所有权转移给你的用户。首先，把 代码清单 11-5 中的代码添加到 docker-compose 文件的应用服务中。

```yaml
services:
--snip--
    application:
--snip--
        user: ${MY_USER}
--snip--

```

*代码清单 11-5：包含 user 属性的 docker-compose.yml 文件*

我们在应用服务中添加 user 属性，并把环境变量 MY_USER 用作该属性的值。然后修改 docker compose 命令，在启动时把当前用户的用户 ID 和组 ID 赋给这个环境变量。我们不再直接调用 docker compose up，而是使用如下命令：

```bash
MY_USER=$(id -u):$(id -g) docker compose up
```

我们使用 id 辅助程序，以 userid:groupid 的格式把用户 ID 和组 ID 保存到环境变量中，docker-compose 文件随后会读取该变量。其中 -u 标志返回用户 ID，-g 标志返回组 ID。

## 总结

我们用 Docker 容器搭建好了本地开发环境。借助本章创建的 docker-compose.yml 文件，我们把应用开发与本地宿主系统解耦了。现在我们可以随意更换宿主系统，同时确保 Food Finder 应用始终以相同的 Node.js 版本运行。此外，我们还添加了一个运行 MongoDB 服务器的容器，下一章实现应用中间件时将连接到它。
