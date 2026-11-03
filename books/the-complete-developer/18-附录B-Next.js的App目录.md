# 附录 B Next.js 的 App 目录

*Appendix B: The Next.js App Directory*

在版本 13 中，Next.js 引入了一种新的路由范式，使用 app 目录（即 App Router）代替 pages 目录。本附录将讨论这一新特性，方便你在此基础上自行深入探索。由于官方并无废弃 pages 目录的计划，你仍然可以继续使用第 5 章中学到的路由方式。你甚至可以同时使用这两个目录；只是要小心，不要往两个目录里添加会生成同一路由的文件夹和文件，否则可能引发错误。

app 目录和 pages 目录都用文件夹和文件来创建路由。不过，app 目录会区分服务端组件（server component）和客户端组件（client component）。在 pages 文件夹中，一切都是客户端组件，也就是说所有代码都会被打进 Next.js 发送给客户端的 JavaScript 包里；而在 app 目录中，每个文件默认都是服务端组件，其代码永远不会被发送到客户端。

本附录先介绍这种新方式的基本概念，然后使用新结构初始化一个 Next.js 应用。

## 服务端组件与客户端组件

在这里，客户端（client）和服务端（server）这两个术语指的是 Next.js 运行时渲染组件所处的环境。客户端环境是用户的环境（通常是浏览器），而服务端指的是接收客户端请求的 Next.js 服务器，无论它运行在你的本机上还是远程位置。

随着服务端组件的引入，Next.js 不再纯粹使用客户端路由。在以服务端为中心的路由中，由服务器渲染组件，然后把渲染好的代码发送给客户端。这意味着客户端无需下载路由映射表，从而减小了页面的初始体积。此外，用户也不必等所有资源加载完毕才能与页面交互。Next.js 服务端组件利用 React 的流式渲染（streaming）架构逐步渲染每个组件的内容。在这种模型下，页面在尚未加载完成时就可以进行交互了。

### 服务端组件

Next.js 服务端组件建立在 React 18 就已推出的 React Server Components 之上。由于这些组件由服务器渲染，它们不会给发送到客户端的 JavaScript 增加任何内容，从而减小了页面的总体积，提升了页面的性能评分。而且，JavaScript 包是可以缓存的，因此当我们新增服务端组件时，客户端不会重新下载它；只有通过新增客户端组件引入新的客户端脚本时才会重新下载。

另外，由于这些组件完全在服务器上渲染，它们可以包含敏感的服务器信息，例如访问令牌和 API 密钥。（作为额外一层保护，Next.js 的渲染引擎会把所有没有显式加上 NEXT_PUBLIC 前缀的环境变量替换为空字符串。）最后，我们可以在不让客户端脚本膨胀的情况下使用大型依赖和额外的框架，并直接访问后端资源，从而提升应用的性能。

代码清单 B-1 展示了服务端组件的基本结构。

```tsx
export default async function ServerComponent(props: WeatherProps): Promise<JSX.Element> {

    return (
      <h1>The weather is {props.weather}</h1>
    );
}
```

*代码清单 B-1：一个基本的服务端组件*

在第 4 章中你已了解到，React 组件就是一个返回 React 元素的 JavaScript 函数；Next.js 服务端组件遵循同样的结构，区别在于它们是异步函数，因此我们可以在 fetch 中使用 async/await 模式。所以，它返回的不是 React 元素本身，而是它的 promise。代码清单 B-1 中的代码应当会让你想起前面几章创建的 WeatherComponent，只不过它不包含任何客户端代码。

### 客户端组件

相比之下，客户端组件是由浏览器而非服务器渲染的组件。你已经知道如何编写客户端组件了，因为传统上所有 React 和 Next.js 组件都是客户端组件。

要渲染这些组件，客户端需要接收所有必需的脚本及其依赖。每个组件都会增大包体积，降低应用的性能。因此，Next.js 提供了一些优化应用性能的选项，例如服务器端渲染（server-side rendering，SSR）：先在服务器上预渲染页面，再让客户端为页面添加交互元素。

app 目录中的所有组件默认都是服务端组件。不过，客户端组件可以放在任何位置（例如我们之前用过的 components 目录）。代码清单 B-2 展示了把代码清单 5-4 中创建的 WeatherComponent 重构成可与 app 目录配合使用的客户端组件后的样子。

```tsx
"use client";

import React, {useState, useEffect} from "react";

export default function ClientComponent (props: WeatherProps): JSX.Element {

    const [count, setCount] = useState(0);
    useEffect(() => {setCount(1);}, []);

    return (
        <h1
          onClick={() => setCount(count + 1)} >
          The weather is {props.weather},
          and the counter shows {count}
        </h1>
    );
}
```

*代码清单 B-2：一个基本的客户端组件，类似于代码清单 5-4 中创建的 WeatherComponent*

我们把该组件以默认函数的形式导出，命名为 ClientComponent。由于使用了客户端 hooks useEffect 和 useState 以及 onClick 事件处理器，我们需要在文件顶部用 "use client" 指令把它声明为客户端组件。否则 Next.js 会抛出错误。

## 渲染组件

在第 5 章中，我们用 getServerSideProps 函数执行服务器端渲染，并用 getStaticProps 函数实现静态站点生成（static site generation，SSG）。在 app 目录中，这两个函数都已被淘汰。如果想优化应用，我们可以改用 Next.js 内置的 fetch API，它在组件级别控制数据获取和渲染。

### 获取数据

新的异步 fetch API 扩展了原生的 fetch Web API，并返回一个 promise。由于服务端组件只是返回 JSX 元素的导出函数，我们可以把它们声明为异步函数，然后在 fetch 中使用 async/await 模式。

这种模式很有用，因为它让我们只需为使用数据的那一段（segment）获取数据，而不必为整个页面获取数据。这样我们就可以利用 React 的特性自动显示 loading 加载状态并优雅地捕获错误，正如第 269 页“探索项目结构”一节所讨论的那样。如果我们遵循这种模式，某个加载状态只会阻塞特定服务端组件及其用户界面的渲染；页面的其余部分仍将完全可用并可交互。

**注意**

客户端组件不应是异步函数，因为 JavaScript 处理异步调用的方式很容易导致多次重复渲染，拖慢整个应用。Next.js 开发者曾讨论过添加一个通用的 use hook，通过缓存结果让我们能在客户端组件中使用异步函数，但这个 hook 尚未定稿。如果你确实需要在客户端获取数据，我建议使用专门的库，例如 SWR，可在 https://swr.vercel.app 找到。

你可能会担心，当每个服务端组件都自行加载数据时，请求数量会变得非常庞大。这些数字会如何影响页面的整体性能呢？实际上，Next.js 的 fetch 自带多项优化来为应用提速。例如，它会自动缓存从服务端组件向同一 API 发送的 GET 请求的响应数据，从而减少请求数量。

不过，POST 请求通常不可缓存，因为其携带的数据可能变化，所以 fetch 不会自动缓存它们。这对我们来说是个问题，因为 GraphQL 通常使用 POST 请求。幸运的是，React 暴露了一个 cache 函数，可以记住其所包装函数的结果。代码清单 B-3 展示了在 GraphQL API 中使用 cache 的例子。

```typescript
import {cache} from 'react';

export const getUserFromGraphQL = cache(async (id:string) => {
    return await fetch("/graphql," {method: "POST", body: "query":" "});
});
```

*代码清单 B-3：缓存 POST API 调用的简单示意*

我们把 API 调用包装在从 React 导入的 cache 函数中，并返回 API 的响应对象。注意，被缓存的参数只能使用原始值（primitive value），因为 cache 函数不会对参数做深比较。

我们可以实现的另一种优化，是利用 fetch 的异步特性以并行而非串行的方式为服务端组件请求数据。这里最常见的模式是使用 Promise.all 同时发起所有请求，并阻塞渲染直到所有请求完成。代码清单 B-4 展示了这种模式的相关代码。

```typescript
const userPromiseOne = getUserFromGraphQL ("0001");
const userPromiseTwo = getUserFromGraphQL ("0002");

const [userDataOne, userDataTwo] = await Promise.all([userPromiseOne, userPromiseTwo]);
```

*代码清单 B-4：使用 Promise.all 的两个并行 API 调用*

我们发起两个请求，两者都返回一个包装用户对象的 promise。然后我们 await 这两个 promise 的结果，把前面创建的异步 API 调用组成数组传给 Promise.all。只要两个 promise 都返回数据，Promise.all 函数就会 resolve，随后服务端组件的代码继续执行。

### 静态渲染

静态渲染（static rendering）是服务端组件和客户端组件的默认设置。它类似于第 5 章中配合 getStaticProps 使用的静态站点生成。这种渲染选项会在构建时于服务器环境中预渲染客户端组件和服务端组件。其结果是，请求总是返回相同的 HTML，这些 HTML 保持静态，永远不会重新生成。

每种组件类型的渲染方式略有不同。对于客户端组件，服务器会预渲染 HTML 和 JSON 数据；客户端随后收到预渲染的数据（包括客户端脚本），为 HTML 添加交互性。对于服务端组件，浏览器只接收用于水合（hydrate）组件的渲染负载。它们既没有客户端 JavaScript，也不使用 JavaScript 进行水合；因此不会向客户端发送任何 JavaScript，也就不会使打包的脚本膨胀。

代码清单 B-5 展示了如何静态渲染代码清单 5-8 中的 utils/fetch-names.ts 文件。

```tsx
export default async function ServerComponentUserList(): Promise<JSX.Element> {
    const url = "https://www.usemodernfullstack.dev/api/v1/users";
    let data: responseItemType[] | [] = [];
    let names: responseItemType[] | [];
    try {
        const response = await fetch(url, {cache: "force-cache"});
        data = (await response.json()) as responseItemType[];
    } catch (err) {
        throw new Error("Failed to fetch data");
    }
    names = data.map((item) => {
        return {id: item.id, name: item.name};
    });

    return (
        <ul>
            {names.map((item) => (
                <li key="{item.id}">{item.name}</li>
            ))}
        </ul>
    );
}
```

*代码清单 B-5：一个使用静态渲染的服务端组件*

首先，我们把服务端组件定义为一个异步函数，它直接返回一个包装在 promise 中的 JSX.Element。

在第 5 章中，我们先返回页面数据，然后通过页面 props 把数据传给 NextPage 函数，并在其中生成元素。而在这里，设置好 url 之后，我们使用异步的 fetch 函数从远程 API 获取数据。Next.js 会缓存 API 调用的结果和渲染出的组件，服务器将复用生成的代码，永远不会重新创建。

如果使用 fetch 时没有显式指定缓存设置，它默认会使用 force-cache 执行静态渲染。若想改用增量静态再生（incremental static regeneration，ISR），请把代码清单 B-5 中的 fetch 调用替换为代码清单 B-6 中的调用。

```typescript
    const response = await fetch(url, {next: {revalidate: 20}});
```

*代码清单 B-6：为类 ISR 渲染修改后的 fetch 调用*

我们只是添加了值为 30 的 revalidate 属性。之后，服务器会静态渲染该组件，但在首次页面请求 30 秒后使当前 HTML 失效并重新渲染。

### 动态渲染

动态渲染（dynamic rendering）取代了 Next.js 传统的服务器端渲染（SSR）——在第 5 章中，我们通过从页面路由导出 getServerSideProps 函数来使用它。由于 Next.js 默认使用静态渲染，我们必须通过以下两种方式之一主动选择启用动态渲染：在 fetch 请求中禁用缓存，或者使用动态函数（dynamic function）。在代码清单 B-7 中，我们禁用了缓存。

```tsx
export default async function ServerComponentUserList(): Promise<JSX.Element> {
    const url = "https://www.usemodernfullstack.dev/api/v1/users";
    let data: responseItemType[] | [] = [];
    let names: responseItemType[] | [];
    try {
            const response = await fetch(url, {cache: "no-cache"});
        data = (await response.json()) as responseItemType[];
    } catch (err) {
        throw new Error("Failed to fetch data");
    }
    names = data.map((item) => {
        return {id: item.id, name: item.name};
    });

    return (
        <ul>
            {names.map((item) => (
                <li key="{item.id}">{item.name}</li>
            ))}
        </ul>
    );
}
```

*代码清单 B-7：通过禁用缓存使用动态渲染的服务端组件*

我们显式地把 cache 属性设置为 no-cache。现在，服务器会在每次请求时为该组件重新获取数据。

除了禁用缓存，我们还可以使用动态函数，包括服务端组件中的 header 函数和 cookies 函数，以及客户端组件中的 useSearchParams hook。这些函数使用请求头、cookie、搜索参数等在构建时未知的数据，它们是我们传给函数的请求对象的一部分。服务器必须为每次请求运行这些函数，因为所需的数据取决于请求。

请记住，动态渲染会影响整个路由。如果路由中有一个服务端组件选择动态渲染，Next.js 就会在请求时动态渲染整个路由。

## 探索项目结构

我们来搭建一个新的 Next.js 应用，以探索前面讨论过的特性。首先，使用 npx create-next-app@latest 命令并加上 --typescript --use-npm 标志来创建一个示例应用。在回答安装向导的问题时，选择使用 app 目录而不是 pages 目录。

**注意**

你也可以使用 https://codesandbox.io/s/ 上的在线 playground 来运行本附录中的 Next.js 代码示例。在那里创建新的 code sandbox 时，搜索官方的 Next.js (App router) 模板即可。

现在输入 npm run dev 命令，以开发模式启动应用。你应该会在浏览器中看到 http://localhost:3000 处的 Next.js 欢迎界面。与第 5 章看到的欢迎界面（它鼓励我们编辑 pages/index.tsx 文件）不同，这里的欢迎界面指向的是 app/page.tsx 文件。

看看向导创建的文件和文件夹，并与第 5 章中的进行比较。你会发现 pages 和 styles 目录已不在新结构中，路由器改用 app 目录取代了这两者。在其中，你既看不到 _app.tsx 文件，也看不到 _document.tsx 文件。它改用根 layout 布局文件 layout.tsx 为所有渲染的页面定义 HTML 包装器，并用 page.tsx 文件渲染根段（root segment，即主页）。

pages 目录只用一个文件来创建页面路由的最终内容；相比之下，app 目录使用多个文件来创建页面路由并添加额外行为。

page.tsx 文件生成路由的用户界面和内容，其父文件夹定义了叶子段（leaf segment）。如果没有 page.tsx 文件，该 URL 路径将无法访问。接着，我们可以在页面的文件夹中添加其他特殊文件，Next.js 会自动把它们应用到这个 URL 段及其子段上。这些特殊文件中最重要的有：layout.tsx，用于创建通用的用户界面；loading.tsx，它使用 React 的 suspense 边界（suspense boundary）在页面加载（loading）时自动创建“加载中”用户界面；以及 error.tsx，它使用 React 的错误边界（error boundary）捕获错误，然后向用户展示自定义的错误处理界面。

图 B-1 比较了使用 pages 目录和 app 目录时 components/weather 页面路由的文件和文件夹。

*图 B-1：pages 与 app 目录结构下 components/weather 页面路由的对比*

当 app 目录作为根文件夹时，其子文件夹仍然对应 URL 段，只不过现在包含 page.tsx 文件的文件夹定义了 URL 的最终叶子段。它旁边的可选特殊文件只影响 components/weather 页面的内容。

下面我们用 app 目录重建你在代码清单 5-1 中创建的 components/weather 页面路由。在 app 目录中创建 components 文件夹和 weather 子文件夹，然后把前面代码练习中的 custom.d.ts 文件复制到根文件夹。

### 更新 CSS

首先打开现有的 app/globals.css 文件，将其内容替换为代码清单 B-8 中的代码。为了在组件中使用特殊文件，我们需要做一些修改。

```css
html,
body {
    background-color: rgb(230, 230, 230);
    font-family: -apple-system, BlinkMacSystemFont, Segoe UI, Roboto, Oxygen,
        Ubuntu, Cantarell, Fira Sans, Droid Sans, Helvetica Neue, sans-serif;
    margin: 0;
    padding: 0;
}

a {
    color: inherit;
    text-decoration: none;
}

* {
    box-sizing: border-box;
}

nav {
    align-items: center;
    background-color: #fff;
    box-shadow: 0 0 10px rgba(0, 0, 0, 0.25);
    display: flex;
    height: 3rem;
    justify-content: space-evenly;
    padding: 0 25%;
}

main {
    display: flex;
    justify-content: center;
}

main .content {
    height: 300px;
    padding-top: 1.5rem;
    width: 400px;
}

main .content li {
    height: 1.25rem;
    margin: 0.25rem;
}

main .loading {
    animation: 1s loading linear infinite;
    background: #ddd linear-gradient(110deg, #eeeeee 0%, #f5f5f5 15%, #eeeeee 30%);
    background-size: 200% 100%;
    min-height: 1.25rem;
    width: 90%;
}

@keyframes loading {
    to {
        background-position-x: -200%;
    }
}
main .error {
    background: #ff5656;
    color: #fff;
}

section {
    background: #fff;
    border: 1px dashed #888;
    box-shadow: 0 0 10px rgba(0, 0, 0, 0.25);
    margin: 2rem;
    padding: 0.5rem;
    position: relative;
}

section .flag {
    background: #888;
    box-shadow: 0 0 10px rgba(0, 0, 0, 0.25);
    color: #fff;
    font-family: monospace;
    left: 0;
    padding: 0.25rem;
    position: absolute;
    top: 0;
    white-space: nowrap;
}
```

*代码清单 B-8：为我们的代码示例提供基础样式的 app/globals.css 文件*

我们为导航创建一个 nav 元素，其下方是 main 内容区域。然后为稍后要创建的加载和错误状态添加样式。此外，我们使用 section 元素勾画出各文件的边界，并用 flag 样式为这些区块添加标签。

### 定义布局

布局（layout）是服务端组件，为特定的路由段定义用户界面。当该段处于激活状态时，Next.js 会渲染这个布局。布局在所有页面之间共享，因此可以相互嵌套；当某个路由段被激活时，该路由及其子路由的所有布局都会被渲染。图 B-2 展示了 components/weather 路由的 URL、文件与组件层级之间的关系。

*图 B-2：简化的布局组件层级*

在这个例子中，每个文件夹都包含一个 layout.tsx 文件。Next.js 会以嵌套的方式渲染它们，并把页面内容作为最终渲染出的组件。

虽然我们可以在布局中获取数据，但不能在父布局与其子级之间共享数据。不过，我们可以利用 fetch API 的自动去重（deduplication）特性，在每个子段或组件中复用数据。当我们从一个页面导航到另一个页面时，只有发生变化的布局会被重新渲染。共享布局在其子段变化时不会被重新渲染。

返回包含 html 和 body 元素的页面骨架结构的根布局（root layout）是必需的，而我们创建的其他布局都是可选的。来创建一个根布局吧。首先，在我们从前面练习复制过来的 custom.d.ts 文件末尾添加一个新接口。我们将使用 LayoutProps 接口为布局的属性对象提供类型：

```typescript
interface LayoutProps {
    children: React.ReactNode;
}
```

现在打开 app/layout.tsx 文件，把它的内容替换为代码清单 B-9 中的代码。

```tsx
import "./globals.css";

export const metadata = {
    title: "Appendix C",
    description: "The Example Code",
};

export default function RootLayout(props: LayoutProps): JSX.Element {
    return (
        <html lang="en">
            <body>
                <section>
                    <span className="flag">app/layout(.tsx)</span>
                    {props.children}
                </section>
            </body>
        </html>
    );
}
```

*代码清单 B-9：app/layout.tsx 文件定义了根布局*

我们导入之前创建的 global.css 文件，然后通过 metadata 对象定义默认的 SEO 元数据、页面标题和页面描述。对于 app 目录中的所有页面，这取代了我们在 pages 目录中使用的 next/head 组件。

接着我们定义 RootLayout 组件，它接受一个 LayoutProps 类型的对象并返回一个 JSX.Element。我们同样创建了这个 JSX.Element，显式添加 html 和 body 元素，然后使用 section 和一个带 flag CSS 类的 span 来勾画页面结构。我们加入 LayoutProps 对象中的 children 属性，用我们的根 HTML 结构包裹它们。

现在我们为 app/components 和 app/components/weather 文件夹添加可选布局。在各自目录中创建 layout.tsx 文件，然后把代码清单 B-10 中的代码放进 app/components/layout.tsx 文件。

```tsx
export default function ComponentsLayout(props: LayoutProps): JSX.Element {
    return (
        <section>
            <span className="flag">app/components/layout(.tsx)</span>
            <nav>Navigation Placeholder</nav>
            <main>{props.children}</main>
        </section>
    );
}
```

*代码清单 B-10：app/components/layout.tsx 文件定义了段布局*

这个段布局文件遵循与根布局相同的基本结构。我们定义了一个布局组件，它接收带有 children 属性的 LayoutProps 对象并返回一个 JSX.Element。与根布局不同的是，我们只设置内部结构：带导航占位符的 nav 元素，以及渲染 LayoutProps 对象中子元素的 main 内容区域，这些子元素代表该段的子内容（叶子）。

最后，把代码清单 B-11 中的代码添加到 app/components/weather/layout.tsx 文件，创建叶子的布局。

```tsx
export default function WeatherLayout(props: LayoutProps): JSX.Element {
    return (
        <section>
            <span className="flag">app/components/weather/layout(.tsx)</span>
            {props.children}
        </section>
    );
}
```

*代码清单 B-11：app/components/weather/layout.tsx 文件定义了叶子布局*

叶子的布局与代码清单 B-10 的段布局类似，但返回的 HTML 结构更为简单，因为这里的 children 属性不再包含另一个布局，而是包含页面的内容（位于 page.tsx 中），以及来自 loading.tsx 和 error.tsx 的 suspense 边界和错误边界。

### 添加内容与路由

要暴露页面路由，我们需要创建 page.tsx 文件；否则，如果我们尝试访问 http://localhost:3000/components/weather 处的 components/weather 页面路由，就会看到 Next.js 默认的 404 错误页面。为了重现代码清单 5-1 的页面内容，我们要创建两个文件：一个是包含 WeatherComponent 的 component.tsx，另一个是 page.tsx，它类似于代码清单 5-1 中使用的 NextPage 包装器。当然，页面还可以包含位于其他文件夹中的额外组件。

我们先在 apps/components/weather 文件夹中创建 component.tsx 文件，并把代码清单 B-12 中的代码添加进去。

```tsx
"use client";

import {useState, useEffect} from "react";

export default function WeatherComponent(props: WeatherProps): JSX.Element {

    const [count, setCount] = useState(0);

    useEffect(() => {
        setCount(1);
    }, []);
    return (
        <h1 onClick={() => {setCount(count + 1)}} >
            The weather is {props.weather}, and the counter shows {count}
        </h1>
    );
}
```

*代码清单 B-12：app/components/weather/component.tsx 文件定义了 WeatherComponent*

这段代码与代码清单 5-1 中 WeatherComponent 常量的代码类似，区别在于我们添加了 "use client" 语句把它显式设置为客户端组件，并将其作为默认函数导出，而不是存储在常量中。组件本身的功能与之前相同：我们创建了一个显示天气字符串的大标题，以及一个点击标题就能增加的计数器。

现在我们添加 page.tsx 文件和代码清单 B-13 中的代码，以创建页面路由并把路由暴露给用户。

```tsx
import WeatherComponent from "./component";

export const metadata = {
    title: "Appendix C - The Weather Component (Weather & Count)",
    description: "The Example Code For The Weather Component (Weather & Count)",
};

export default async function WeatherPage() {
    return (
        <section className="content">
            <span className="flag">app/components/weather/page(.tsx)</span>
            <WeatherComponent weather="sunny" />
        </section>
    );
}
```

*代码清单 B-13：app/components/weather/page.tsx 文件定义了页面路由*

我们导入刚创建的 WeatherComponent，然后在页面级别设置 SEO 元数据。接着，我们把页面路由导出为默认的 async 函数。与包含类似页面的代码清单 5-1 相比可以看到，我们不再需要导出 NextPage，而是使用一个普通函数。app 目录简化了代码结构。

现在在浏览器中访问 http://localhost:3000/components/weather 处的 components/weather 页面路由。你应该会看到一个与图 B-3 相似的页面。

这里有两点需要注意。第一，你应该认得出这个来自第 5 章的组件，点击标题时计数器会增加。此外，样式与我们添加到每个 .tsx 文件中的 span 元素相结合，把文件之间的关系可视化了。我们看到，嵌套的布局文件呈现出与图 B-3 所示简化组件层级相似的样子。

*图 B-3：显示嵌套组件的 components/weather 页面*

### 错误处理

只要在文件夹中添加一个 error.tsx 文件，Next.js 就会用 React 错误边界包裹我们页面的内容。图 B-4 展示了添加 error.tsx 文件后 components/weather 路由的简化组件层级。

*图 B-4：简化的布局组件层级，其中包含错误边界*

我们看到，error.tsx 文件会围绕页面内容自动创建一个错误边界。这样一来，Next.js 让我们能够在页面级别捕获错误并优雅地处理，而不是冻结整个用户界面或把用户重定向到一个通用的错误页面。可以把它理解为组件级别的 try...catch 块。现在我们可以显示量身定制的错误信息，并提供一个按钮，让用户在不重新加载整个应用的情况下，把页面内容重新渲染回之前正常的状态。

error.tsx 文件导出一个客户端组件，错误边界把它用作回退（fallback）界面。换句话说，当代码抛出错误并激活错误边界时，这个组件会替换原有内容。错误边界一旦激活就会包含（contain）这个错误，确保边界之上的布局保持激活状态并维持其内部状态。error 组件接收 error 对象和 reset 函数作为参数。

我们来给 components/weather 路由添加一个错误边界。首先，在 customs.d.ts 文件中添加一个新的 ErrorProps 接口，为组件的属性提供类型：

```typescript
interface ErrorProps {
    error: Error;
    reset: () => void;
}
```

接着，在 app/components/weather 目录中 page.tsx 旁边创建 error.tsx 文件，并添加代码清单 B-14 中的代码。

```tsx
"use client";

export default function WeatherError(props: ErrorProps): JSX.Element {
    return (
        <section className="content error">
            <span className="flag">app/components/weather/error(.tsx)</span>
            <h2>Something went wrong!</h2>
            <blockquote>{props.error?.toString()}</blockquote>
            <button onClick={() => props.reset()}>Try again (re-render)</button>
        </section>
    );
}
```

*代码清单 B-14：app/components/weather/error.tsx 文件添加了错误边界和回退 UI*

因为我们知道 error 组件必须是客户端组件，所以先在文件顶部添加 "use client" 指令，然后定义并导出该组件。我们使用刚创建的 ErrorProps 接口为组件的属性提供类型。接着，我们把 error 属性转换为字符串并显示出来，告知用户发生了哪种类型的错误。最后，我们渲染一个按钮，它调用组件通过属性对象接收到的 reset 函数。用户点击该按钮即可把组件重新渲染回之前正常的状态。

现在，错误边界就位之后，我们修改 component.tsx，让计数器达到 4 或更大时抛出错误。打开该文件，把代码清单 B-15 中的代码添加到第一个 useEffect hook 下方。

```typescript
    useEffect(() => {
        if (count && count >= 4) {
            throw new Error("Count >= 4! ");
        }
    }, [count]);
```

*代码清单 B-15：为 app/components/weather/component.tsx 添加的额外 useEffect hook*

我们添加到组件中的这个额外 useEffect hook 很简单：一旦 count 变量变化，就检查错误条件；一旦该变量的值达到 4 或更大，就抛出消息为 Count >= 4! 的错误。错误边界会捕获这个错误并优雅地处理——显示 error.tsx 文件导出的回退用户界面。

要测试这个功能，在浏览器中打开 http://localhost:3000/components/weather，不断点击标题直到触发错误。你应该会看到 error 组件取代了天气组件，如图 B-5 所示。

*图 B-5：处于错误状态的 components/weather 页面*

布局标记告诉我们 error.tsx 已经取代了 page.tsx。我们还能看到传给 error 构造函数的字符串 Error: Count >=4!。一旦点击重新渲染按钮，page.tsx 就应取代 error.tsx，屏幕会恢复到之前图 B-4 所示的样子。

### 显示可选的加载界面

现在我们来创建 loading.tsx 文件。这个功能就位后，Next.js 会自动用一个 React suspense 组件包裹页面内容，形成与图 B-6 相似的组件层级。

*图 B-6：带加载界面的简化布局组件层级*

loading.tsx 文件是一个基本的服务端组件，返回预渲染的加载用户界面。当我们加载页面或在页面之间导航时，Next.js 会在加载新段内容的同时立即显示这个组件。一旦渲染完成，运行时就会用新内容替换加载状态。这样，我们就能轻松地显示有意义的加载状态，例如骨架屏或自定义动画。

我们把代码清单 B-16 中的代码添加到 loading.tsx 文件，为天气组件路由添加一个基本的加载用户界面。

```tsx
export default function WeatherLoading(): JSX.Element {
    return (
        <section className="content">
            <span className="flag">app/components/weather/loading(.tsx)</span>
            <h1 className="loading"></h1>
        </section>
    );
}
```

*代码清单 B-16：app/components/weather/loading.tsx 文件添加了带加载用户界面的 suspense 边界*

我们定义并导出返回 JSX.Element 的 WeatherLoading 组件。在 HTML 中，我们添加了一个与 page.tsx 中类似的大标题元素，区别是这里给标题加上了我们在 global.css 文件中创建的 loading 类，显示一个动画占位符。

在浏览器中打开 http://localhost:3000/components/weather 时，我们应该会看到与图 B-7 相似的加载界面。

*图 B-7：正在加载页面内容的 components/weather 页面*

如果你没有看到动画占位符，说明 Next.js 已经缓存了你这个段的内容。

### 添加使用远程数据的服务端组件

既然你已经了解了 app 目录中的文件夹和文件，我们来添加一个服务端组件，它使用 fetch API 从远程 API https://www.usemodernfullstack.dev/api/v1/users 获取用户列表并渲染到浏览器。我们在第 5 章写过这段代码的一个版本。

创建 app/components/server-component 文件夹，并在其中添加特殊文件 component.tsx、loading.tsx、error.tsx、layout.tsx 和 page.tsx。然后把代码清单 B-17 中的代码添加到 component.tsx 文件，搭建组件的功能。

```tsx
export default async function ServerComponentUserList(): Promise<JSX.Element|Error> {
    const url = "https://www.usemodernfullstack.dev/api/v1/users";
    let data: responseItemType[] | [] = [];
    let names: responseItemType[] | [];
    try {
        const response = await fetch(url, {cache: "force-cache"});
        data = (await response.json()) as responseItemType[];
    } catch (err) {
        throw new Error("Failed to fetch data");
    }
    names = data.map((item) => {
        return {id: item.id, name: item.name};
    });

    return (
        <ul>
            {names.map((item) => (
                <li id="{item.id}" key="{item.id}">
                    {item.name}
                </li>
            ))}
        </ul>
    );
}
```

*代码清单 B-17：app/components/server-component/component.tsx 文件*

这里我们创建了一个默认服务端组件，它使用 fetch API 等待 API 响应。为此，我们把它定义为一个异步函数，返回 JSX.Element 或 Error 的 promise。然后，我们把 API 端点存入一个常量，并定义稍后需要的变量。我们把 API 调用包在 try...catch 语句中，以便在 API 请求失败时激活错误边界（Error Boundary）。接着，我们按照与第 5 章类似的方式转换数据，并返回一个显示用户列表的 JSX.Element。

接下来我们添加加载用户界面，在我们等待 API 响应和组件 JSX 输出期间，Next.js 会自动显示它。把代码清单 B-18 中的代码放入 loading.tsx 文件。

```tsx
export default function ServerComponentLoading(): JSX.Element {
    return (
        <section className="content">
            <span className="flag">
                app/components/server-component/loading(.tsx)
            </span>
            <ul id="load">
                {[...new Array(10)].map((item, i) => (
                    <li className="loading"></li>
                ))}
            </ul>
        </section>
    );
}
```

*代码清单 B-18：app/components/server-component/loading.tsx 文件*

和之前一样，loading 组件是返回 JSX.Element 的服务端组件。这一次，加载骨架是一个包含 10 个列表项的列表，与组件渲染出的 HTML 结构相似。你会看到，这让用户对即将出现的内容有良好的预期，应当能改善用户体验。

接下来，把代码清单 B-19 中的代码添加到 error.tsx 文件，创建错误边界。

```tsx
"use client"; // Error components must be Client components

export default function ServerComponentError(props: ErrorProps): JSX.Element {
    return (
        <section className="content">
            <span className="flag">app/components/server-component/error(.tsx)</span>
            <h2>Something went wrong!</h2>
            <code>{props.error?.toString()}</code>
            <button onClick={() => props.reset()}>Try again (re-render)</button>
        </section>
    );
}
```

*代码清单 B-19：app/components/server-component/error.tsx 文件*

除了勾画文件结构的 flag 之外，这个错误边界与我们在天气组件中使用的类似。

然后我们把代码清单 B-20 中的代码添加到 layout.tsx 文件。

```tsx
export default function ServerComponentLayout(props: LayoutProps): JSX.Element {
    return (
        <section>
            <span className="flag">app/components/server-component/layout(.tsx)</span>
            {props.children}
        </section>
    );
}
```

*代码清单 B-20：app/components/server-component/layout.tsx 文件*

同样，这段代码与我们在天气组件中使用的类似。我们只调整了勾画组件层级的 flag。

最后，所有部分就位之后，我们把代码清单 B-21 中的代码添加到 page.tsx 文件，暴露页面路由。

```tsx
import ServerComponentUserList from "./component";

export const metadata = {
    title: "Appendix C - Server Side Component (User API)",
    description: "The Example Code For A Server Side Component (User API)",
};

export default async function ServerComponentUserListPage(): JSX.Element {
    return (
        <section className="content">
            <span className="flag">app/components/server-component/page(.tsx)</span>
            {/* @ts-expect-error Async Server Component */}
            <ServerComponentUserList />
        </section>
    );
}
```

*代码清单 B-21：app/components/server-component/page.tsx 文件*

### 用导航补全应用

应用中有了两个页面，现在我们可以使用 next/link 组件替换 nav 元素中的导航占位符。这样就能得到一个功能完整的应用原型，让我们在页面之间导航。打开 app/components/layout.tsx 文件，把文件中的代码替换为代码清单 B-22 中的代码。

```tsx
import Link from "next/link";

export default function ComponentsLayout(props: LayoutProps): JSX.Element {
    return (
        <section>
            <span className="flag">app/components/layout(.tsx)</span>
            <nav>
                <Link href="/components/server-component">
                    User API <br />
                    (Server Component)
                </Link>{" "}
                |
                <Link href="/components/weather">
                    Weather Component <br />
                    (Client Component)
                </Link>
            </nav>
            <main>{props.children}</main>
        </section>
    );
}
```

*代码清单 B-22：更新后的 app/components/layout.tsx 文件*

我们导入 next/link 组件，然后在导航中添加两个链接：一个指向刚创建的用户列表服务端组件，另一个指向天气客户端组件。

我们访问应用的天气组件页面 http://localhost:3000/components/weather。你应该会看到一个与图 B-8 截图相似的应用。

只要在页面之间导航，你就应该会看到加载用户界面。借助我们添加到所有文件中的边框标记，可以轻松分辨 Next.js 当前用哪些文件来渲染页面。

*图 B-8：带可用导航的 components/weather 页面*

### 用路由处理器取代 API 路由

如果你查看 Next.js 为你创建的文件夹结构，会发现 app 目录中包含一个 api 子文件夹。你可能已经猜到，我们用这个文件夹来定义 API。但与第 5 章讨论的 API 路由（它们是普通函数）不同，app 目录使用路由处理器（route handler）——它们是要求特定命名约定的函数。

这些路由处理器应放在名为 route.ts 的特殊文件中，这些文件通常位于 app/api 文件夹的子文件夹里。它们是异步函数，接收一个 Request 对象和一个可选的 context 对象作为参数。我们以每个函数应响应的 HTTP 方法来命名函数。例如，代码清单 B-23 中的代码展示了如何为同一个 API 定义处理 GET 和 POST 请求的路由处理器。

```tsx
import {NextRequest, NextResponse} from 'next/server';
export async function GET(request: NextRequest): Promise<NextResponse> {
    return NextResponse.json({});
}

export async function POST(request: NextRequest): Promise<NextResponse {
    return NextResponse.json({});
}
```

*代码清单 B-23：定义路由处理器的 route.ts 文件骨架结构*

要创建路由处理器，我们从 Next.js 的 server 包导入 NextRequest 和 NextResponse 对象。Next.js 为 cookie 处理、重定向和重写提供了额外的便捷方法。更多信息可参阅 https://nextjs.org 上的官方文档。

然后我们定义两个异步函数，它们都接收一个 NextRequest 并返回 NextResponse 的 promise。函数名与它们应响应的 HTTP 方法相对应。Next.js 支持用 GET、POST、PUT、PATCH、DELETE、HEAD 和 OPTIONS 方法作为函数名。

定义页面路由和路由处理器时要记住，这些文件会接管给定段的所有请求。这意味着 route.ts 文件不能与 page.tsx 文件位于同一个文件夹。另外，如果使用 app 目录，路由处理器是定义 API 的唯一方式：你不能同时在 pages 目录和 app 目录中拥有 api 文件夹。

Next.js 的路由处理器有静态求值和动态求值两种。静态求值的路由处理器会被缓存并在每次请求时复用，而动态求值的路由处理器必须在每次请求时重新获取数据。默认情况下，运行时会对所有不使用动态函数或 Response 对象的 GET 路由处理器进行静态求值。一旦我们使用了其他 HTTP 方法、动态的 cookies 或 headers 函数，或者 Response 对象，路由处理器就会变成动态求值。带动态段的 API 也是如此，它们通过 context 对象接收动态参数。

我们把代码清单 5-1 中的 API 路由 api/v1/weather/[zipcode].ts 重构为可在 app 目录中使用的路由处理器。把代码清单 B-24 中的代码添加到 app/api/v1/weather/[zipcode] 文件夹结构下的 route.ts 文件中。

```tsx
import {NextResponse, NextRequest} from "next/server";

interface ReqContext {
    params: {
        zipcode: number;
    }
}

export async function GET(req: NextRequest, context: ReqContext): Promise<NextResponse> {
    return NextResponse.json(
        {
            zipcode: context.params.zipcode,
            weather: "sunny",
            temp: 35,
        },
        {status: 200}
    );
}
```

*代码清单 B-24：app/api/v1/weather/[zipcode]/route.ts 中的路由处理器*

注意，我们在文件夹结构上使用了方括号模式，以便通过函数的第二个参数 context 对象访问动态段。

在文件中，我们从 server 包导入 Next.js 的 Response 和 NextRequest 对象，然后为路由处理器定义接口。在 RequestContext 接口上，我们给 params 添加了 zipcode 属性，代表该 API 的动态段。最后，我们导出异步的 GET 函数——响应此 API 端点所有 GET 请求的 API 路由处理器。它接收请求对象和请求上下文作为参数，并使用 NextResponse 的 json 函数返回响应数据。我们通过 context 对象的 params 对象访问 URL 参数 zipcode，然后把它加入响应数据。我们通过 json 函数的第二个参数设置额外的响应选项，显式地把 HTTP 状态码设为 200。

现在试着用 curl 查询这个 API：

```bash
$ curl -i \
    -X GET \
    -H "Accept: application/json" \
    http://localhost:3000/api/v1/weather/12345
```

你应该会收到如下 JSON 响应：

```json
HTTP/1.1 200 OK
content-type: application/json
--snip--
{"zipcode":"12345","weather":"sunny","temp":35}
```

这与第 5 章中通过浏览器访问 API 时收到的响应相同。
