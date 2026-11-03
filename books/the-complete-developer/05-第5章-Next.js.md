# 第 5 章 Next.js

*Chapter 5: Next.js*

在第 4 章中，你使用 React 创建了具备响应能力的用户界面组件。但 React 只是一个库，要构建全栈应用还需要额外的工具。本章我们使用 Next.js——构建在 React 之上、处于领先地位的 Web 应用框架。用 Next.js 创建应用，你只需掌握几个核心概念，本章就将介绍这些概念。

Next.js 简化了应用前端、中间件和后端的创建。在前端，它使用 React，并额外提供原生 CSS 模块来定义样式，以及自定义的 Next.js 模块来执行路由、图片处理和其他前端任务。至于中间件和后端，Next.js 使用一个内置服务器来提供 HTTP 请求的入口点，并提供了干净的 API 来操作请求和响应对象。

我们将介绍它基于文件的路由（file-based routing）方式，讨论构建和渲染交付给客户端的 Web 页面的各种方法，探索如何添加 CSS 文件为页面设置样式，并把我们的 Express.js 服务器重构为与 Next.js 配合使用。本章使用传统的 pages 目录来讲解这些基础概念。要了解 Next.js 另一种可选的 app 目录（App Router），请参见附录 B。

## 搭建 Next.js

Next.js 是 npm 生态系统的一员。你当然可以手动运行 npm install next react react-dom 安装所有必需的模块，然后亲自创建项目的全部文件和文件夹，但有一种简便得多的搭建方式：运行 create-next-app 命令。

我们来创建一个贯穿本章使用的示例应用。按照以下步骤新建一个名为 sample-next 的空文件夹，并在其中构建你的第一个 Next.js 应用。保留安装向导的默认回答，并选择使用传统的 pages 目录而非 app 目录：

```bash
$ mkdir sample-next
$ cd ./sample-next
$ npx create-next-app@latest --typescript --use-npm
--snip--
What is your project named? ... my-app
--snip--
Creating a new Next.js app in /Users/.../my-app.

Installing dependencies:
- react
- react-dom
- next

Installing devDependencies:
- eslint
- eslint-config-next
- typescript
- @types/react
- @types/node
- @types/react-dom
```

我们新建一个文件夹，切换到该文件夹，然后初始化一个新的 Next.js 项目。我们使用 npx 而不是 npm，因为正如第 1 章所学，npx 不要求我们把任何东西安装为依赖或开发依赖。前面提到过，npx 的一个典型用例就是搭建脚手架（scaffolding），而这正是我们在这里要做的事。

create-react-app 命令有几个选项，其中只有两个与我们相关：--typescript 选项会创建一个支持 TypeScript 的 Next.js 项目，--use-npm 标志则选择 npm 作为包管理器。

我们接受默认的项目名 my-app 以及其他所有默认设置。脚本会根据项目名创建一个文件夹，其中包含 package.json 文件和一个带有全部必要文件与文件夹的完整示例项目。最后，它通过 npm 安装依赖和开发依赖。

注意：你也可以不搭建新项目，而是使用 https://codesandbox.io/s/ 或 https://stackblitz.com 上的在线运行环境来运行本章的 Next.js 代码示例。同样，在那里也请选择 pages 目录而非 app 目录的配置。

### 项目结构

我们来探究一下样板 Next.js 应用的项目结构。输入以下命令运行它：

```bash
$ cd my-app
$ npm run dev

> my-app@0.1.0 dev
> next dev

ready - started server on 0.0.0.0:3000, url: http://localhost:3000
```

在浏览器中访问给出的 URL。你应该会看到一个默认页面，与图 5-1 中的页面类似（这个欢迎页面可能因你的 Next.js 版本而有所不同）。

现在打开脚手架脚本创建的 my-app 文件夹，四处看看。my-app 文件夹包含很多文件夹，但目前对你重要的只有三个：public、styles 和 pages。

public 文件夹存放所有静态资源，例如自定义字体文件、所有图片以及应用提供下载的文件。我们会在应用的 HTML 和 CSS 文件中链接这些资源。pages 文件夹包含应用的所有路由，其中每个文件都是一个端点，属于页面路由或 API 路由（API route，位于 api 子文件夹中）。

注意：较新版本的 Next.js 还额外包含一个 app 目录，你可以选择用它代替 pages 目录进行路由。由于 app 目录使用了更高级的概念，本章介绍的是更简单的 pages 架构风格。不过，你可以在附录 B 中进一步了解 app 目录，我们将在其中详细介绍它的用法。

在 my-app 文件夹中，我们还能找到 _app.tsx 文件，它相当于我们在第 4 章使用的 App.tsx 文件。它是整个应用的入口点，我们将在其中添加全局样式、组件和 context provider。最后，styles 文件夹包含全局 CSS 文件，以及用于局部作用域、组件专属样式的模块文件。

### 开发脚本

我们的应用所使用的技术——包括 TypeScript、React 和 JSX——都不能直接在浏览器中运行，它们都需要一个带有相当复杂的转译器的构建流水线。Next.js 提供了四个命令行脚本来简化开发。

npx next dev 和 npm run dev 命令以开发模式在 http://localhost:3000 启动应用。这样一来，只要我们修改一个文件，Next.js 就会立即重新构建并在浏览器窗口中重新加载已渲染的应用。除了这种热代码重载（hot-code reloading）之外，开发服务器还会显示错误和警告信息，帮助应用开发。安装向导已把这个服务器加入 package.json 的 script 部分，因此我们也可以用 npm run dev 来启动它。

npx next build 和 npm run build 命令创建应用的优化构建版本。它们会移除未使用的代码，并减小脚本、样式和其他所有资源的文件体积，你将在正式部署时使用它们。npx next start 和 npm run start 命令在内置服务器或无服务器（serverless）环境中以生产模式在 http://localhost:3000 运行优化后的应用。这种生产构建依赖于先前创建的构建产物，因此必须先运行 build 命令。

最后，npx next export 和 npm run export 命令创建应用的独立版本，它不依赖内置的 Next.js 服务器，可以运行在任何基础设施上。不过，这个版本的应用无法使用需要在服务器端运行 Next.js 的功能。使用指南请查阅 https://nextjs.org 上的 Next.js 官方文档。

## 为应用设置路由

在构建示例 Express.js 服务器时，我们创建了一个显式的路由文件，把应用的每个端点映射到执行相应行为的不同函数。Next.js 提供了另一种或许更简单的路由系统：它基于 pages 目录中的文件自动创建应用的路由。如果这个文件夹中的某个文件导出的是一个 React 组件（对应网页）或一个异步函数（对应 API），它就会成为一个有效的端点——要么是一个 HTML 页面，要么是一个 API。

在本节中，我们将重新审视在 Express.js 服务器中创建的那些路由，并用 Next.js 的路由技术重新实现它们。

### 简单页面路由

在 Express.js 服务器中，我们在 index.ts 文件里手动创建了 /hello 路由，访问它会返回 Hello World! 现在我们把这个路由转换为 Next.js 中基于页面的路由。最简单的页面路由就是把一个文件直接放进 pages 目录。例如，默认创建的 pages/index.tsx 文件映射到 http://localhost:3000。要创建一个简单的 /hello 路由，在该目录下新建一个 hello.tsx 文件，然后向其中加入代码清单 5-1 中的代码。

```jsx
import type {NextPage} from "next";

const Hello: NextPage = () => {
    return (<>Hello World!</>);
}

export default Hello;
```

*代码清单 5-1：pages/hello.tsx 文件*

我们的 Express.js 服务器使用 routeHello 函数返回 Hello World! 字符串。在这里，我们需要多写一点代码来导出一个 React 组件。首先，我们从 Next.js 模块导入自定义类型 NextPage，并用它创建一个常量 Hello。我们为这个常量赋一个胖箭头函数（fat arrow function），它返回一个 NextPage——后者不过是 React 组件的一层自定义封装。在这个例子里，我们返回渲染 Hello World! 字符串的 JSX。最后，我们把这个 NextPage 导出为文件的默认导出。

运行服务器并访问 http://localhost:3000/hello。你看到的页面应该以 Hello World! 作为内容。

这个页面看起来与示例 Express.js 服务器返回的页面不同。这是因为 Next.js 会自动把 _app.tsx 文件中定义的所有全局样式应用到每个页面。因此，即使我们没有在 hello.tsx 文件中显式定义任何样式，字体看起来也不一样。

### 嵌套页面路由

嵌套路由（nested route），例如示例 Express.js 服务器中的 /components/weather，是其他路由的逻辑子路由。换句话说，weather 嵌套在 components 入口点之内。你可能已经猜到如何用 Next.js 的页面路由模式创建嵌套路由了。没错，我们只需创建一个子文件夹，Next.js 就会把文件夹结构映射到 URL 模式。

在 pages 文件夹中新建一个名为 components 的文件夹，并在其中添加一个新文件 weather.tsx。图 5-2 展示了 URL components/weather 与文件结构 pages/components/weather.tsx 之间的关系。

我们的 pages 文件夹是 URL 的根，每个嵌套的文件夹都成为 URL 的一段。导出 NextPage 的文件是叶子段（leaf segment），即 URL 的最后一段。对于这个文件，我们复用第 4 章编写的天气组件代码，如代码清单 5-2 所示。

```jsx
import type {NextPage} from "next";
import React, {useState, useEffect} from "react";

const PageComponentWeather: NextPage = () => {

    interface WeatherProps {
        weather: string;
    }

    const WeatherComponent = (props: WeatherProps) => {

        const [count, setCount] = useState(0);
        useEffect(() => {
            setCount(1);
        }, []);

        return (
            <h1 onClick={() => setCount(count + 1)}>
                The weather is {props.weather},
                and the counter shows {count}
            </h1>
        );
    };

    return (<WeatherComponent weather="sunny" />);
};

export default PageComponentWeather;
```

*代码清单 5-2：pages/components/weather.tsx 文件*

与第 4 章创建的函数组件相比，唯一的区别是我们把代码包裹在一个返回 NextPage 的函数里，然后将其作为默认导出导出。这与我们在代码清单 5-1 中创建的页面一致，符合 Next.js 的模式要求。

在浏览器中访问新页面 http://localhost:3000/components/weather。它应该与图 5-3 类似。

你应该能认出第 4 章中出现过的点击处理器功能。

### API 路由

除了用户友好的界面之外，全栈应用可能还需要机器可读的接口。例如，你将在第二部分创建的 Food Finder 应用会向外部服务提供 API，让移动应用或第三方小部件能够展示我们的愿望清单。作为以 JavaScript 为驱动的全栈开发者，我们最常用的 API 格式是 GraphQL 和 REST，第 6 章将深入讨论它们。这里我们创建的是 REST API，我们喜欢它是因为它简单。

有了 Next.js，我们可以用与页面相同的模式来设计和创建 API。pages/api/ 文件夹中的每个文件都是一个单独的 API 端点，而且我们可以按照定义嵌套页面路由的相同方式定义嵌套 API 路由。不过，与页面路由不同，API 路由不是 React 组件，而是接收 NextApiRequest 和 NextApiResponse 两个参数、返回 NextApiResponse 和 JSON 数据的异步函数。

关于 API 路由，有两点需要注意。第一，它们默认不指定跨源资源共享（Cross-Origin Resource Sharing，CORS）头。这组 HTTP 头（其中最著名的是 Access-Control-Allow-Origin 头）允许服务器定义客户端脚本可以从哪些源请求资源。如果你希望运行在第三方域名网站中的客户端脚本访问你的 API 端点，就需要添加额外的中间件，直接在 Next.js 服务器中启用 CORS。否则，外部请求会触发 CORS 错误。这并非 Next.js 独有；Express.js 和大多数其他服务器框架也要求你做同样的事。

第二点需要注意的是，通过运行 next export 完成的静态导出不支持 API 路由。API 路由依赖内置的 Next.js 服务器，无法作为静态文件运行。

我们在 Express.js 服务器中使用过一个 API 路由 api/names。现在来重构这段代码，把它转换为 Next.js 的 API 路由。和之前一样，新建一个 names.ts 文件并放入 api 文件夹。由于 API 路由返回的是异步函数而不是 JSX，我们使用 .ts 扩展名，而不是 JSX 代码使用的 .tsx 扩展名。把代码清单 5-3 中的代码粘贴到该文件中。

```typescript
import type {NextApiRequest, NextApiResponse} from "next";

type responseItemType = {
    id: string;
    name: string;
};

export default async function handler(
    req: NextApiRequest,
    res: NextApiResponse
): Promise<NextApiResponse<responseItemType[]> | void> {
    const url = "https://www.usemodernfullstack.dev/api/v1/users";
    let data;
    try {
        const response = await fetch(url);
        data = (await response.json()) as responseItemType[];
    } catch (err) {
        return res.status(500);
    }
    const names = data.map((item) => {
        return {id: item.id, name: item.name};
    });
    return res.status(200).json(names);
}
```

*代码清单 5-3：pages/api/names.ts 文件*

首先，我们从 Next.js 包导入 API 请求和响应的自定义类型。然后，我们为 API 响应定义自定义类型。在第 3 章中，我们曾为 routes.ts 文件里的 await 调用创建过同样的类型。这里我们使用了相同的代码和 await 调用，因此也复用了这个类型。接着，我们创建前面提到的 API 处理器函数并直接导出。你在第 2 章学过，异步函数需要以 promise 作为返回类型，因此我们把这个 API 响应包裹在 promise 中。

函数体中的代码与第 4 章 routeAPINames 函数中的代码类似：它发起 API 请求获取用户数据，把收到的数据转换成期望的返回格式，最后返回数据。不过我们需要做几处修改。第一处，我们不再返回错误字符串，而是返回一个没有内容、状态码为通用值 500（表示服务器内部错误，Internal Server Error）的 API 响应。

第二处调整涉及数据映射。之前我们返回的是一个在浏览器中渲染的字符串，现在改为返回一个 JSON 对象。因此，我们修改 array.map 函数来创建一个对象数组。最后，我们修改 return 语句，返回以 JSON 形式携带 names 对象、状态码为 200（OK）的 API 响应。

现在在浏览器中打开新的 API 路由 http://localhost:3000/api/names。你应该会看到如图 5-4 所示的 API 响应。

### 动态 URL

现在你已经知道如何创建页面路由和 API 路由了，它们是任何全栈应用的基础。不过，你可能想知道如何创建随输入变化的动态 URL。我们经常在个人资料页使用动态 URL，让用户名成为 URL 的一部分。实际上，我们在 Express.js 服务器的天气 API 中就实现过一个动态 URL：在 index.ts 文件中定义了 /api/weather/:zipcode 路由。其中 zipcode 是一个动态参数（也称动态叶子段），通过 req.params.zipcode 读取它的值。

Next.js 为动态 URL 使用了一种略有不同的模式。由于它基于文件夹和文件来创建路由，我们需要通过文件名来定义动态段，把可变的部分用方括号（[]）包裹起来。因此，Express.js 服务器中的动态路由 /api/weather/:zipcode 就对应文件 /api/weather/[zipcode].ts。

我们在示例 Next.js 应用中创建一个模仿 Express.js 服务器 /api/weather/:zipcode 路由的动态路由。在 api 文件夹中新建一个名为 weather 的文件夹，并在其中放一个名为 [zipcode].ts 的文件。然后把代码清单 5-4 中的代码粘贴进去。

```typescript
import type {NextApiRequest, NextApiResponse} from "next";

type WeatherDetailType = {
    zipcode: string;
    weather: string;
    temp?: number;
};

export default async function handler(
    req: NextApiRequest,
    res: NextApiResponse
): Promise<NextApiResponse<WeatherDetailType> | void> {

    return res.status(200).json({
        zipcode: req.query.zipcode,
        weather: "sunny",
        temp: 35
    });

}
```

*代码清单 5-4：api/weather/[zipcode].ts 文件*

这段代码你应该已经熟悉了，它遵循 Next.js 中 API 路由的基本骨架。我们导入所需的类型，然后定义自定义类型 WeatherDetailType，并将其用作函数返回数据的类型（顺便一提，这与我们在第 3 章创建的类型定义相同）。在函数体中，我们返回状态码为 200（OK）的响应和一个 JSON 对象。zipcode 属性填入的是动态 URL 参数中的邮政编码，通过 req.query.zipcode 获取。

运行服务器后，浏览器应该会显示 JSON 响应，其中包含动态 URL 参数对应的值。如果你访问 http://localhost:3000/api/weather/12345，就能看到这个 API 响应。如果修改 URL 中的“12345”部分并再次请求数据，响应数据也会相应变化。

注意，动态路由 /api/weather/[zipcode].ts 能匹配 /api/weather/12345 和 /api/weather/54321，但不能匹配这些路由的子路径，例如 /api/weather/location/12345 或 /api/weather/location/54321。要匹配子路径，你需要使用捕获所有路径的 API 路由（catch all API route），它涵盖当前路径之下的所有路径。创建 catch all 路由的方法是在文件名前加上三个点（...）。例如，catch all 路由 /api/weather/[...zipcode].ts 就能处理本段提到的全部四个 API 端点。

## 为应用添加样式

要为 Next.js 应用添加样式，我们创建普通的 CSS 文件，编写时无需使用其他框架所要求的厂商前缀（vendor prefix）。稍后，Next.js 的后处理器会添加必要的属性来生成向后兼容的样式。虽然 CSS 语法超出了本书的范围，但本节将介绍如何使用 Next.js 的两类 CSS 样式：全局样式，以及定义在 CSS 模块中的局部作用域组件样式。

### 全局样式

全局样式会影响应用的所有页面。在渲染 hello.tsx 文件时我们就已经碰到过这种行为：即使我们自己没有添加任何样式信息，页面也使用了 CSS。

实际上，全局样式就是普通的 CSS 文件。它们在构建过程中不会被修改，类名保证保持不变，因此我们可以在整个应用中把它们当作普通 CSS 类来使用。我们在应用的入口点 pages/_app.tsx 文件中导入这些 CSS 文件。看看样板项目里的这个文件，你应该会看到一行类似代码清单 5-5 的代码。

```javascript
import "@/styles/globals.css";
```

*代码清单 5-5：在 _app.tsx 文件中导入全局样式*

当然，你可以调整所导入样式的文件名和位置，也可以导入多个文件。试着在 global.css 文件中添加一些样式，并为 hello.tsx 文件中的 HTML 元素加上一些普通 CSS 类，然后访问 http://localhost:3000/hello 查看页面的变化。

### 组件样式

在第 4 章中你已经看到，React 让我们能够用独立、可复用的组件构建用户界面。对于为独立组件设置样式来说，全局样式并不是最好的方式：它要求我们记住各个组件中已经用过的类名，而如果我们从以前的项目导入组件，CSS 类就有相互冲突的风险。

为了高效地使用模块化组件，我们需要把 CSS 类的作用域限定在单个模块内。实现这一点有多种架构模式。例如，使用 BEM（Block Element Modifier，块元素修饰符）方法论，你可以手动把样式限定在某个组件或用户界面块内。

幸运的是，我们不必费心使用这种笨拙的方案。Next.js 允许我们使用在构建过程中划定作用域的 CSS 模块（CSS modules）。这类 CSS 模块遵循 <组件名>.module.css 的命名约定。编译器会自动为模块内的每个 CSS 类名加上组件名和唯一标识符作为前缀，这样你就可以在多个组件中使用相同的样式名而不会出问题。

你实际编写的 CSS 并不包含这些前缀。例如，看看 styles 文件夹中的 Home.module.css 文件，如代码清单 5-6 所示。

```css
.container {
    padding: 0 2rem;
}
```

*代码清单 5-6：styles/Home.module.css 中的普通 CSS 代码*

一个问题在于，由于构建过程会修改类名并为它们加上前缀，我们无法在其他文件中直接使用这些样式。我们必须导入样式并把它当作 JavaScript 对象来对待，然后通过 styles 对象的属性来引用它们。例如，代码清单 5-7 中的 pages/index.tsx 文件使用了代码清单 5-6 中的 container 类，展示了如何使用局部作用域样式。

```jsx
import styles from "../styles/Home.module.css"
--snip--
const Home: NextPage = () => {
    return (
        <div className={styles.container}>
        --snip--
        </div>
    );
};
```

*代码清单 5-7：在 index.tsx 文件中使用 CSS 模块 styles/Home.module.css 中的样式*

这段代码把 CSS 文件导入到一个名为 styles 的常量中。现在，所有 CSS 类名都可以作为 styles 对象的属性来访问。在 JSX 中，我们使用花括号（{}）包裹变量，因此我们以 {styles.container} 的形式引用 container 类。

现在你可以用 React 组件构建 API 和带有自定义样式的页面了。下一节将介绍 Next.js 提供的一些实用的自定义组件，用来增强你的全栈应用。

## Next.js 内置组件

Next.js 提供了一组自定义组件，每个组件都针对一个特定的用例：例如，访问页面标题或 SEO 元数据等页面内部属性（next/head）、提升应用的整体渲染性能和用户体验（next/image），或者实现应用的路由（next/link）。本章介绍的这些 Next.js 组件将在第二部分的全栈应用中使用，你可以在那里看到它们的实际应用。至于其他属性和更小众的用例，请查阅 Next.js 文档。

### next/head 组件

next/head 组件导出一个 Next.js 特有的自定义 Head 组件。我们用它来设置页面的 HTML title 和 meta 元素，这些元素位于 HTML 的 head 部分中。为了提升 SEO 排名并增强可用性，每个页面都应该拥有自己的元数据。代码清单 5-8 展示了代码清单 5-1 中的 hello.tsx 页面自定义 title 和 meta 元素之后的示例。

需要记住的一点是，Head 元素不会跨页面合并。Next.js 的客户端路由在页面切换时会移除 Head 元素的内容。

```jsx
import type {NextPage} from "next";
import Head from "next/head";

const Hello: NextPage = () => {
    return (
        <div>
            <Head>
                <title>Hello World Page Title</title>
                <meta property="og:title" content="Hello World" key="title" />
            </Head>
            <div>Hello World!</div>
        </div>
    );
};

export default Hello;
```

*代码清单 5-8：自定义了 title 和 meta 元素的 pages/hello.tsx 文件*

我们从 next/head 组件导入 Head 元素，把它加到返回的 JSX 元素中，放在现有内容的上方，并把两者一起包裹在另一个 div 元素里，因为我们需要返回一个元素而不是两个。

### next/link 组件

next/link 组件导出一个 Link 组件。这个组件构建在 React 的 Link 元素之上。当我们想链接到应用内的另一个页面时，就用它代替 HTML 锚点标签（anchor tag），从而实现页面之间的客户端切换。被点击时，Link 组件会用新的 DOM 更新浏览器 DOM、滚动到新页面顶部，并调整浏览器历史记录。此外，它还提供内置的性能优化：只要 Link 组件进入视口（viewport，即网站当前可见的部分），它就会预取（prefetch）所链接的页面及其数据。这种后台预取让页面切换平滑流畅。代码清单 5-9 在上一个代码清单的页面中加入了 Next.js 的 Link 元素。

```jsx
import type {NextPage} from "next";
import Head from "next/head";
import Link from "next/link";

const Hello: NextPage = () => {
    return (
        <div>
            <Head>
                <title>Hello World Page Title</title>
                <meta property="og:title" content="Hello World" key="title" />
            </Head>
            <div>Hello World!</div>
            <div>
                Use the HTML anchor for an
                <a href="https://nostarch.com" > external link</a>
                and the Link component for an
                <Link href="/components/weather"> internal page
                </Link>
                .
            </div>
        </div>
    );
};

export default Hello;
```

*代码清单 5-9：包含外部链接和内部 next/link 元素的 pages/hello.tsx 文件*

我们导入该组件，然后把它加到返回的 JSX 元素中。为了便于对比，我们用一个普通的 HTML 锚点链接到 No Starch Press 主页，并用自定义的 Link 连接到 Next.js 应用中的天气组件页面。在应用里试着点击这两个链接，看看二者的区别。

### next/image 组件

next/image 组件导出一个用于显示图片的 Image 组件。这个组件构建在原生 HTML 的 <img> 元素之上，能够处理常见的布局需求，例如填满所有可用空间和缩放图片。它可以加载 AVIF 和 WebP 等现代图片格式，并针对客户端屏幕以正确的尺寸提供图片。此外，你还可以选择使用模糊的占位图片，并在真实图片进入视口时才懒加载（lazy-load）它；通过防止累积布局偏移（cumulative layout shift，即图片在页面之后才渲染完成，导致页面内容向下移动的现象），这保证了网站的视觉稳定性。累积布局偏移被视为糟糕的用户体验，会让用户分心。代码清单 5-10 给出了 next/image 组件的基本示例。

```jsx
import type {NextPage} from "next";
import Head from "next/head";
import Link from "next/link";
import Image from "next/image";

const Hello: NextPage = () => {
    return (
        <div>
            <Head>
                <title>Hello World Page Title</title>
                <meta property="og:title" content="Hello World" key="title" />
            </Head>
            <div>Hello World!</div>
            <div>
                Use the HTML anchor for an <a href="https://nostarch.com">
                external link</a> and the Link component for an
                <Link href="/components/weather"> internal page</Link>.
                <Image
                    src="/vercel.svg"
                    alt="Vercel Logo"
                    width={72}
                    height={16}
                />
            </div>
        </div>
    );
};
export default Hello;
```

*代码清单 5-10：使用 next/image 元素的 pages/hello.tsx 文件*

这里我们显示应用 public 文件夹中的 Vercel 徽标。首先，我们从 next/image 包导入该组件，然后把它加到页面内容中。这个最简示例的语法和属性都与 HTML 的 img 元素类似。关于该组件高级属性的更多信息，可阅读 https://nextjs.org/docs/api-reference/next/image 上的官方文档。

## 预渲染与发布

尽管凭借目前学到的知识你已经可以着手构建全栈 Next.js 应用，但还有一个进阶话题值得了解：渲染和发布应用的不同方式，以及它们对应用性能的影响。

Next.js 提供三种使用内置服务器预渲染（pre-rendering）应用的方式。第一种是静态网站生成（static site generation，SSG），它在构建时生成 HTML，因此每次请求总是返回相同的 HTML，这些 HTML 保持静态、不会重新创建。第二种是服务端渲染（server-side rendering，SSR），它在每次请求时生成新的 HTML 文件。第三种是增量静态再生（incremental static regeneration，ISR），它结合了前两种方式。

Next.js 允许我们按页面选择预渲染方式，也就是说，全栈应用中可以同时包含使用 SSG、SSR 和 ISR 的页面，以及为某些 React 组件使用客户端渲染（client-side rendering）的页面。你还可以通过运行 next export 创建站点的完整静态导出版本。导出的应用可以独立运行在所有基础设施上，因为它不需要内置的 Next.js 服务器。

为了体验这些渲染方式，我们将为每种渲染选项分别创建一个新页面，用来显示 names API 返回的数据。在 pages 文件夹旁边新建一个 utils 文件夹，并在其中添加一个空文件 fetch-names.ts，然后加入代码清单 5-11 中的代码。这个工具函数调用远程 API 并返回数据集。

```typescript
type responseItemType = {
    id: string;
    name: string;
};

export const fetchNames = async () => {
    const url = "https://www.usemodernfullstack.dev/api/v1/users";
    let data: responseItemType[] | [] = [];
    let names: responseItemType[] | [];
    try {
        const response = await fetch(url);
        data = (await response.json()) as responseItemType[];
    } catch (err) {
        names = [];
    }
    names = data.map((item) => {return {id: item.id, name: item.name}});

    return names;
};
```

*代码清单 5-11：utils/fetch-names.ts 中的异步工具函数*

定义自定义类型之后，我们创建一个函数并直接导出。这个函数包含之前创建的 names.ts 文件中的代码，但做了两处调整：其一，我们需要把 data 数组定义为可能为空；其二，如果 API 调用失败，我们返回空数组而不是错误字符串。这一改动意味着在生成 JSX 字符串、遍历数组之前，我们无需再验证类型。

### 服务端渲染（SSR）

使用 SSR 时，Next.js 内置的 Node.js 服务器会针对每个请求生成应用的 HTML。如果你的页面依赖来自外部 API 的新鲜数据，就应该使用这种技术。遗憾的是，SSR 在生产环境中速度较慢，因为页面不容易缓存。

要让某个页面使用 SSR，需要从该页面额外导出一个异步函数 getServerSideProps。Next.js 会在每次请求时调用这个函数，并把取到的数据传给页面的 props 参数，在发送给客户端之前完成预渲染。

我们来试一试：在 pages 文件夹中新建一个 names-ssr.tsx 文件，然后把代码清单 5-12 中的代码粘贴进去。

```jsx
import type {
    GetServerSideProps,
    GetServerSidePropsContext,
    InferGetServerSidePropsType,
    NextPage,
    PreviewData
} from "next";
import {ParsedUrlQuery} from "querystring";
import {fetchNames} from "../utils/fetch-names";

type responseItemType = {
    id: string;
    name: string;
};

const NamesSSR: NextPage = (props: InferGetServerSidePropsType<typeof getServerSideProps>) => {

    const output = props.names.map((item: responseItemType, idx: number) => {
        return (
            <li key={`name-${idx}`}>
                {item.id} : {item.name}
            </li>
        );
    });

    return (
        <ul>
            {output}
        </ul>
    );
};

export const getServerSideProps: GetServerSideProps = async (
    context: GetServerSidePropsContext<ParsedUrlQuery, PreviewData>
) => {

    let names: responseItemType[] | [] = [];
    try {
        names = await fetchNames();
    } catch(err) {}
    return {
        props: {
          names
        }
    };
};

export default NamesSSR;
```

*代码清单 5-12：使用 SSR 显示数据的基本页面，page/names-ssr.tsx*

为了使用 Next.js 的 SSR，我们额外导出了异步函数 getServerSideProps。我们还从 next 和 querystring 包导入必要的功能，并导入之前创建的 fetchNames 函数。然后，我们为 API 请求的响应定义自定义类型，它与我们在第 3 章使用的自定义类型相同。

接着，我们创建页面并将其导出为默认导出。这个页面返回一个 NextPage，并接受这种页面类型的默认属性。我们遍历 props 参数中的 names 数组，创建一个 JSX 字符串，渲染后返回给浏览器。然后，我们定义从 API 获取数据的 getServerSideProps 函数。我们从这个异步函数中返回所创建的数据集，并通过页面属性把它传给 NextPage。

访问新页面 http://localhost:3000/names-ssr。你应该会看到用户名列表。

### 静态网站生成（SSG）

SSG 只创建一次 HTML 文件，然后在每次请求时复用它们。这是推荐的方式，因为预渲染的页面易于缓存、交付迅速。例如，内容分发网络可以轻松接手你的静态文件。

通常，SSG 应用的首次绘制时间（time to first paint）更短，也就是从用户请求页面（例如点击链接）到内容出现在浏览器中所需的时间。SSG 还能减少阻塞时间（blocking time），即用户真正能够与页面内容交互之前所需的时间。在这些指标上取得好成绩意味着网站响应迅速，而它们也是 Google 评分算法的一部分，因此这些页面会获得更高的 SEO 排名。

如果你的页面依赖外部数据，仍然可以使用 SSG：从页面文件中额外导出一个异步函数 getStaticProps。Next.js 会在构建时调用这个函数，把取到的数据传给页面的 props 参数，并以 SSG 预渲染该页面。当然，这只在外部数据不是动态数据时才可行。

我们试着用 SSG 创建与 SSR 示例相同的页面。在 pages 文件夹中添加新文件 names-ssg.tsx，然后粘贴代码清单 5-13 所示的代码。

```jsx
import type {
    GetStaticProps,
    GetStaticPropsContext,
    InferGetStaticPropsType,
    NextPage,
    PreviewData,
} from "next";
import {ParsedUrlQuery} from "querystring";
import {fetchNames} from "../utils/fetch-names";

type responseItemType = {
    id: string,
    name: string,
};

const NamesSSG: NextPage = (props: InferGetStaticPropsType<typeof getStaticProps>) => {

    const output = props.names.map((item: responseItemType, idx: number) => {
        return (
            <li key={`name-${idx}`}>
                {item.id} : {item.name}
            </li>
        );
    });

    return (
        <ul>
            {output}
        </ul>
    );
};

export const getStaticProps: GetStaticProps = async (
    context: GetStaticPropsContext<ParsedUrlQuery, PreviewData>
) => {

    let names: responseItemType[] | [] = [];
    try {
        names = await fetchNames();
    } catch (err) {}

    return {
        props: {
            names
        }
    };
};

export default NamesSSG;
```

*代码清单 5-13：使用 SSG 显示数据的页面，page/names-ssg.tsx*

这段代码与代码清单 5-9 大体相同。我们只需把 SSR 特有的代码改为使用 SSG：因此我们导出 getStaticProps 而不是 getServerSideProps，并相应调整类型。

访问该页面时，它看起来应该与 SSR 页面类似。但与每次访问 http://localhost:3000/names-ssg 都请求新数据不同，这里的数据只在页面构建时请求一次。

### 增量静态再生（ISR）

ISR 是 SSG 和 SSR 的混合体，完全在服务器端运行。它在初始构建期间在服务器上生成 HTML，并在页面第一次被请求时发送这份预生成的 HTML。经过指定的时间后，Next.js 会在后台获取数据并在服务器上重新生成页面。在此过程中，它会使服务器内部缓存失效，并用新页面更新缓存。之后的每个请求都会收到最新的页面。与 SSG 一样，ISR 的成本低于 SSR，并能提升页面的 SEO 排名。

要在 SSG 页面中启用 ISR，我们需要在 getStaticProps 的返回对象中添加一个 revalidate 属性。我们以秒为单位定义数据的有效期，如代码清单 5-14 所示。

```typescript
return {
        props: {
        names,
        revalidate: 30
    }
};
```

*代码清单 5-14：修改 getServerSideProps 以启用 ISR*

我们添加值为 30 的 revalidate 属性。这样，定制的 Next.js 服务器将在第一次页面请求 30 秒后使当前 HTML 失效。

### 客户端渲染（CSR）

客户端渲染是一种完全不同的方式：先用 SSR 或 SSG 生成 HTML 并发送给客户端，客户端随后在运行时获取额外的数据并渲染到浏览器 DOM 中。在处理高度灵活、不断变化的数据集（例如实时股票行情或货币价格）时，客户端渲染是不错的选择。另一些网站用它先把页面的骨架版本发送给客户端，之后再补充更多内容来增强页面。不过，客户端渲染会降低 SEO 表现，因为它的数据无法被索引。

代码清单 5-15 展示了我们之前创建的页面配置为客户端渲染后的样子。在 pages 文件夹中新建一个 names-csr.tsx 文件，然后把代码加入其中。

```jsx
import type {
    NextPage
} from "next";
import {useEffect, useState} from "react";
import {fetchNames} from "../utils/fetch-names";

type responseItemType = {
    id: string,
    name: string,
};

const NamesCSR: NextPage = () => {
    const [data, setData] = useState<responseItemType[] | []>();
    useEffect(() => {
        const fetchData = async () => {
            let names;
            try {
                names = await fetchNames();
            } catch (err) {
                console.log("ERR", err);
            }
            setData(names);
        };
        fetchData();
    });

    const output = data?.map((item: responseItemType, idx: number) => {
        return (
            <li key={`name-${idx}`}>
                {item.id} : {item.name}
            </li>
        );
    });

    return (
        <ul>
            {output}
        </ul>
    );

};

export default NamesCSR;
```

*代码清单 5-15：客户端渲染页面，page/names-csr.tsx*

这段代码与前面的示例有很大不同。这里我们导入了 useState 和 useEffect 钩子。后者会在页面已经可用之后获取数据。一旦 fetchNames 函数返回数据，我们就借助 useState 钩子和响应式的 data 状态变量来更新浏览器 DOM。

我们不能把 useEffect 钩子声明为异步函数，因为它必须返回 undefined 或一个函数，而异步函数返回的是 promise，TSC 会因此抛出错误。为了避免这个问题，我们需要把 await 调用包裹在一个名为 fetchData 的异步函数中，然后再调用这个函数。

配置为客户端渲染的页面看起来应该与其他版本类似。但访问 http://localhost:3000/names-csr 时，你可能会看到一阵白屏闪烁。这是页面在等待异步 API 请求。

为了更好地体会不同渲染类型的差异，请把本节每个示例的代码改为使用 API https://www.usemodernfullstack.dev/api/v1/now，它返回一个包含请求时间戳的对象。

### 静态 HTML 导出

next export 命令会生成 Web 应用的静态 HTML 版本。这个版本不依赖基于 Node.js 的 Next.js 内置 Web 服务器，可以运行在任何基础设施上，例如 Apache、NGINX 或 IIS 服务器。

要使用这个命令，页面必须像 SSG 那样实现 getStaticProps。该命令不支持 getServerSideProps 函数、ISR 和 API 路由。

## 练习 5：将 Express.js 和 React 重构为 Next.js

让我们把前几章的 React 和 Express.js 应用重构为一个 Next.js 应用，我们将在接下来的章节中不断扩展它。第一步，先总结需要构建的功能。我们的应用有一个返回用户名的 API 路由 api/names，以及另一个返回静态 JSON 对象和 URL 参数的 API 路由 api/weather/:zipcode，我们曾用它来理解动态 URL。此外，我们还创建了 /hello 和 component/weather 两个页面。

在本章中，我们已经把这些不同的元素逐一重构为符合 Next.js 路由风格的形式。在这个练习里，我们将把它们整合到一起。按照“搭建 Next.js”一节（第 70 页）中的步骤初始化 Next.js 应用。在 sample-next 文件夹内，把你的应用命名为 refactored-app。

### 存储自定义接口和类型

我们在项目根目录新建一个 custom.d.ts 文件，用来存放自定义接口和类型定义（代码清单 5-16）。它与我们第 3 章和第 4 章使用的文件类似，主要区别在于我们为 Next.js 应用添加了自定义类型。

```typescript
interface WeatherProps {
    weather: string;
}

type WeatherDetailType = {
    zipcode: string;
    weather: string;
    temp?: number;
};

type responseItemType = {
    id: string;
    name: string;
};
```

*代码清单 5-16：custom.d.ts 文件*

我们将把自定义接口 WeatherProps 用于显示天气组件的页面 components/weather 的 props 参数；WeatherDetailType 用于使用动态获取的邮政编码的 API 路由 api/weather/:zipcode；最后，我们在 API 路由 api/names 中使用 responseItemType 来标注 fetch 响应的类型。

### 创建 API 路由

接下来，我们重新创建 Express.js 服务器中的两个 API 路由。本章前面的章节已经展示过这些重构后的代码。对于 api/names 路由，在 api 文件夹中新建一个 names.ts 文件，然后加入代码清单 5-3 中的代码。代码的详细说明请参阅相应章节。

把 Express.js 服务器中的动态路由 api/weather/:zipcode 迁移到 Next.js 应用：在 api 文件夹中创建一个 [zipcode].js 文件，并加入“动态 URL”一节（第 77 页）中代码清单 5-4 的代码。更多细节可以参阅该节。

### 创建页面路由

现在来处理页面。首先，对于简单页面 hello.tsx，我们在 pages 文件夹中新建一个文件，并加入代码清单 5-10 中的代码。这段代码渲染 Hello World! 示例，并使用了 Next.js 的自定义组件 Head、Link 和 Image，它们都在“Next.js 内置组件”一节（第 80 页）中有详细说明。

第二个页面是嵌套路由 pages/components/weather.tsx。和之前一样，我们在 pages 文件夹中名为 components 的文件夹里新建一个 weather.tsx 文件，加入代码清单 5-2 中的代码。这个代码清单使用 useState 和 useEffect 钩子创建了响应式用户界面。我们可以从这个文件中删掉 WeatherProps 的自定义接口定义，因为 custom.d.ts 文件已经把它们提供给了 TSC。

### 运行应用

使用 npm run dev 命令启动应用。现在你可以访问我们为 Express.js 服务器创建的那些相同的路由，并看到它们在功能上完全一致。恭喜！你已经创建了第一个基于 Next.js 的全栈应用。试着摆弄一下代码，用全局 CSS 和组件 CSS 为页面设置样式。

## 小结

Next.js 补齐了用 React 创建全栈应用所缺失的功能。在搭建示例项目、探究默认文件结构之后，你学习了如何在这个框架中创建页面路由和 API 路由，还学习了全局和组件级 CSS、Next.js 的四个内置命令行脚本，以及它最有用的一些自定义组件。

我们还讨论了用 Next.js 渲染内容和页面的不同方式，以及何时选择哪种方式。最后，你利用本章的代码，把前几章构建的 Express.js 应用快速迁移到了 Next.js。要继续探索这个实用的框架，我推荐 https://nextjs.org 上的官方教程。

下一章我们将探讨两类 Web API：标准的 RESTful API 和现代的 GraphQL。
