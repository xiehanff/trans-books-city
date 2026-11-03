# 第 16 章 在 Docker 中运行自动化测试

*Chapter 16: Running Automated Tests in Docker*

在这最后一个简短的章节中，你将编写几个自动化测试（automated test）来验证 Food Finder 应用程序的状态，然后配置一个 Docker 服务来持续运行它们。

我们将重点介绍如何使用快照测试（snapshot test）并通过模拟（mock）用户会话来评估应用程序的页头（Header）。我们不会为其他组件以及中间件、服务或 API 创建测试，但我鼓励你自行构建这些测试。可以尝试使用基于浏览器的端到端（end-to-end）测试，配合 Cypress 或 Playwright 等专门框架来测试完整的页面。你可以在 https://nextjs.org/docs/testing 找到这两个框架的安装说明和示例。

## 将 Jest 添加到项目中

使用 npm 安装 Jest 相关库：

```bash
$ docker exec -it foodfinder-application npm install --save-dev jest \
jest-environment-jsdom @testing-library/react @testing-library/jest-dom
```

接下来，创建一个名为 jest.config.js 的新文件，写入清单 16-1 中的代码，让 Jest 与我们的 Next.js 设置协同工作。将该文件保存在应用程序的根目录下。

```javascript
const nextJest = require("next/jest");

const createJestConfig = nextJest({
    dir: "./",
});

const customJestConfig = {
    moduleDirectories: ["node_modules", "<rootDir>/"],
    testEnvironment: "jest-environment-jsdom",
};

module.exports = createJestConfig(customJestConfig);
```

*清单 16-1：jest.config.js 文件*

我们借助了 Next.js 内置的 Jest 配置，因此需要配置项目的基础目录，以便把配置文件和 .env 文件加载到测试环境中。随后设置模块目录的位置和全局测试环境。这里之所以使用全局设置，是因为我们的快照测试需要 DOM 环境。

现在我们希望能够通过 npm 命令来运行测试。为此，把清单 16-2 中的两条命令添加到项目 package.json 文件的 scripts 属性中。

```javascript
    "test": "jest ",
    "testWatch": "jest --watchAll"
```

*清单 16-2：添加到 package.json 文件 scripts 属性中的两条命令*

第一条命令会一次性执行所有可用的测试；第二条命令则持续监视文件变动，并在检测到变化时重新运行测试。

## 设置 Docker

要用 Docker 来运行测试，需要在 docker-compose.yml 中再添加一个使用 Node.js 镜像的服务。该服务在启动时会运行 npm run testWatch，也就是我们刚刚定义的命令。这样，我们就能持续运行测试，并即时获得应用程序状态的反馈。请修改该文件，使其与清单 16-3 中的代码一致。

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

    application:
        container_name: foodfinder-application
        image: node:lts-alpine
        working_dir: /home/node/code/foodfinder-application
        ports:
            - "3000:3000"
        volumes:
            - ./code:/home/node/code
        depends_on:
            - backend
        environment:
            - HOST=0.0.0.0
            - CHOKIDAR_USEPOLLING=true
            - CHOKIDAR_INTERVAL=100
        tty: true
        command: "npm run dev"

    jest:
        container_name: foodfinder-jest
        image: node:lts-alpine
        working_dir: /home/node/code/foodfinder-application
        volumes:
            - ./code:/home/node/code
        depends_on:
            - backend
            - application
        environment:
            - NODE_ENV=test
        tty: true
        command: "npm run testWatch"

volumes:
    mongodb_data_container:
```

*清单 16-3：修改后的 docker-compose.yml 文件，包含 jest 服务*

这个名为 jest 的小型服务使用的是我们之前用过的官方 Node.js Alpine 镜像。我们设置了工作目录，并通过 volumes 属性让代码在这个容器中同样可用。不过，与应用程序服务不同的是，jest 服务把 Node.js 环境设置为 test，并运行 testWatch 命令。

重启 Docker 容器，控制台应当会显示 Jest 正在监视我们的文件。

## 为页头元素编写快照测试

与第 8 章一样，在应用程序的根目录下创建 __tests__ 文件夹来存放测试文件。然后添加 header.snapshot.test.tsx 文件，内容如清单 16-4 所示。

```typescript
import {act, render} from "@testing-library/react";
import {useSession} from "next-auth/react";
import Header from "components/header";

jest.mock("next-auth/react");
describe("The Header component", () => {
    it("renders unchanged when logged out", async () => {
        (useSession as jest.Mock).mockReturnValueOnce({
            data: {user: {}},
            status: "unauthenticated",
        });
        let container: HTMLElement | undefined = undefined;
        await act(async () => {
            container = render(<Header />).container;
        });
        expect(container).toMatchSnapshot();
    });

    it("renders unchanged when logged in", async () => {
        (useSession as jest.Mock).mockReturnValueOnce({
            data: {
                user: {
                    name: "test user",
                    fdlst_private_userId: "rndmusr",
                },
            },
            status: "authenticated",
        });
        let container: HTMLElement | undefined = undefined;
        await act(async () => {
            container = render(<Header />).container;
        });
        expect(container).toMatchSnapshot();
    });
});
```

*清单 16-4：__tests__/header.snapshot.test.tsx 文件*

这些测试应当与你第 8 章编写的测试类似。注意，我们先从 next-auth/react 导入 useSession 钩子（hook），然后在每个测试的准备（arrange）步骤中使用 jest.mock 将其替换。通过用一个返回指定状态的模拟会话取而代之，我们可以验证页头组件在面对已登录和未登录用户时的行为都符合预期。我们按照准备（arrange）、执行（act）、断言（assert）模式来描述 Header 组件的测试套件，并验证渲染出来的组件与已存储的快照一致。

第一个测试用例使用空会话和 unauthenticated 状态，把页头渲染成未登录的状态。第二个测试用例使用仅含最少数据的会话，并把用户状态设置为 authenticated。这样我们就能验证：已有会话显示的用户界面与空会话显示的有所不同。

如果你编写了更多测试，请务必把它们添加到 __tests__ 文件夹中。

## 总结

你已经成功添加了几个简单的快照测试，用于验证 Food Finder 应用程序按预期运行。借助这个额外的 Docker 服务，你可以持续验证后续的开发不会破坏应用程序。

恭喜！你已经使用 TypeScript、React、Next.js、Mongoose 和 MongoDB 成功创建了第一个全栈应用程序。你用 Docker 将应用程序容器化，并用 Jest 对它进行了测试。通过本书及其练习所掌握的知识，你已经为自己的全栈开发职业生涯打下了基础。
