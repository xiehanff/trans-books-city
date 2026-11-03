# 第 4 章 React

*Chapter 4: React*

开发者可以使用 React 库来创建全栈应用的用户界面。React 构建在 Node.js 生态系统之上，作为最常用的 Web 框架之一，目前访问量最高的网站中有超过 40% 的网站以它为基础构建。

要高效地使用 React，你必须理解用于定义用户界面元素外观的语法，然后将这些元素组合成能够动态更新的 React 组件。本章涵盖使用这个库开始开发全栈应用所需了解的全部内容。

## React 的角色

现代前端架构将应用的用户界面拆分成小型、自包含且可复用的单元。其中一些元素（例如页眉、导航和徽标）可能每页只出现一次，另一些则是构成页面内容的重复元素，例如标题、按钮和导语。图 4-1 展示了其中一些元素。React 的语法顺应了这一模式；这个库专注于构建这些独立的组件，从而帮助我们更高效地开发应用。

React 采用声明式编程范式：你通过描述期望的结果来创建用户界面，而不是像命令式编程那样明确列出创建它所需的全部步骤。声明式范式的一个经典示例是 HTML。使用 HTML，你描述网页的元素，然后由浏览器渲染页面。相比之下，你也可以用 JavaScript 编写一个创建每个 HTML 元素的命令式程序，这样就等于明确列出了构建网站所需的每一步。

此外，这些用户界面组件是响应式的。这意味着两点：其一，它们各自管理自己隔离的状态；其二，一旦某个组件的状态发生变化，它就会立即更新页面的 HTML。对 React 代码的修改会即时影响到浏览器的文档对象模型（document object model，DOM），DOM 将网站表示为一棵树，其中每个 HTML 元素都是一个节点。DOM 还为每个节点以及整个网站提供了 API，使脚本能修改网站或某个特定节点。

DOM 操作（例如重新渲染一个组件）的代价高昂。为了更新 DOM，React 使用虚拟 DOM，它是真实浏览器 DOM 在内存中的克隆，之后会与真实 DOM 同步。虚拟 DOM 支持增量更新，从而减少浏览器上代价高昂的操作次数。虚拟 DOM 是 React 的一个关键原则。每次调用渲染函数时，React 都会计算虚拟 DOM 与真实 DOM 之间的差异，然后决定要更新什么。通常，React 会执行批量更新以进一步降低性能影响。这一调和过程让 React 能够交付快速且响应灵敏的用户界面。

尽管 React 主要是一个用户界面库，开发者也可以用它构建不需要中间件或后端的单页应用。这类应用只不过是在浏览器中渲染的一个视图层。在某种程度上，它们可以是动态的：例如，我们可以切换页面的语言、打开图片画廊，或者切换元素的可见性。不过，这一切都发生在浏览器中，依靠额外的 React 模块完成，而不是在服务器上。

借助 React 的 Router 模块，我们还能完全在浏览器中实现更高级的功能，例如更新浏览器的地址以模拟存在多个不同页面。这个模块让我们可以在前端定义路由，类似于我们在 Express.js 服务器中定义的路由。只要用户点击一个内部链接，路由组件就会更新视图并改变浏览器的地址，让用户感觉自己加载了另一个 HTML 页面。实际上，我们只是改变了当前页面的内容。这样做避免了对服务器的又一轮请求，因此这个“模拟的页面”加载速度快得多。而且，由于页面之间的切换由我们的 JavaScript 代码控制，我们还可以为这些切换添加特效和动画。

## 搭建 React

你在第 13 页的练习 1 中创建的基础 Express.js 服务器使用标准 JavaScript，可以直接在 Node.js 上运行；与此不同，React 依赖一个带有完整构建工具链的高级环境。例如，它使用一种自定义的 JavaScript 语法扩展 JSX 来描述 HTML 元素，并使用 TypeScript 进行静态类型标注，这两者都需要转译器将代码转换为 JavaScript。因此，手动搭建 React 的过程相当复杂。

因此，我们通常借助其他工具。对于单页应用，我们使用代码生成器（例如 create-react-app）来搭建脚手架。在脚手架生成过程中，create-react-app 会为新 React 应用生成样板代码，以及项目的构建链和文件夹结构。它还提供一致的项目布局，帮助我们轻松理解其他 React 项目。

要运行本章的示例，一种选择是按照 https://create-react-app.dev/docs/getting-started/ 上的步骤，用 create-react-app 搭建一个简单的 TypeScript React 应用。如果你不想创建专门的项目，也可以在在线运行环境（例如 https://codesandbox.io 或 https://stackblitz.com）中使用带 TypeScript 模板的 React 来运行代码。这些在线运行环境与 create-react-app 遵循相同的文件结构。无论哪种方式，你都应将代码保存到默认的 App.tsx 文件中。

对于更复杂的应用，我们会使用完整的 Web 应用框架（例如 Next.js），它开箱即用地提供了所需的环境。Next.js 将在第 5 章介绍，它是使用 React 的全栈 Web 应用最流行的框架。在内部，Next.js 使用 create-react-app 的一个变体来搭建脚手架。在后续章节中，我们将依靠它来使用 React。

## JavaScript 语法扩展

React 使用 JSX 来定义用户界面组件的外观。JSX 是 JavaScript 的一种扩展，必须经过转译器转换后，浏览器才能将其渲染到 DOM。虽然它具有类似 HTML 的语法，但它不只是一个简单的模板语言，而是允许我们使用任何 JavaScript 特性来描述 React 元素。例如，我们可以在条件语句中使用 JSX 语法、把它赋值给变量，以及从函数中返回它。编译器会把包裹在花括号（{}）中的任何变量或有效的 JavaScript 表达式嵌入到 HTML 中。

借助这一机制，我们可以直接在页面的模板中使用 array.map 遍历数组、对每一项检查某个条件、把该项传给另一个函数，并根据该函数的返回值创建一组 JSX 元素。这听起来可能有些抽象，但在第二部分构建 Food Finder 应用、创建 React 组件时，我们会大量使用这种模式。

### 一个 JSX 表达式示例

如代码清单 4-1 所示的 JSX 表达式是 React 用户界面最核心的部分。这段 JavaScript 代码定义了一个 JSX 函数表达式 getElement，它接受一个字符串作为参数并返回一个 JSX.Element。

```jsx
import React from "react";

export default function App() {
    const getElement = (weather: string): JSX.Element => {
        const element = <h1>The weather is {weather}</h1>;
        return element;
    };
    return getElement("sunny");
}
```

*代码清单 4-1：一个最小的 JSX 表达式示例*

每个 React 应用的入口都是 App 函数。就像 Express.js 服务器的 index.js 文件一样，这个函数在应用启动时被执行。在这里，我们通常会设置全局元素，例如样式表和整体页面布局。

React 会把这个函数的返回值渲染到浏览器。在代码清单 4-1 中，我们直接返回了一个元素。元素是 React 用户界面最小的构建单元，它描述你在屏幕上看到的内容，就像 HTML 元素那样。元素的例子包括自定义按钮、标题和图片。

导入 React 包之后，我们创建这个 JSX 元素并将其存储在一个名为 element 的常量中。乍一看，你可能好奇它为什么没有被引号包裹——它包含的内容看起来就像一个普通的 HTML h1 元素，像是一个字符串。答案是：它不是字符串，而是一个 JSX 元素，React 库会以编程方式由它创建 HTML 元素。最终，这段代码会在页面上显示一条关于天气的消息。

一旦调用这个 JSX 表达式，React 库就会把它转译成普通的 JavaScript 函数调用，并从这个 JSX 元素创建一个将在浏览器中显示的 HTML 字符串。在第 3 章中你学过，所有合法的 JavaScript 也是合法的 TypeScript，因此我们同样可以在 TypeScript 中使用 JSX。JSX 文件使用 .jsx（JavaScript）或 .tsx（TypeScript）扩展名。把这段代码粘贴到你创建的项目中的 App.tsx 文件里，无论是在线运行环境的预览窗格还是你的浏览器，都应渲染出一个内容为 The weather is sunny 的 h1 HTML 元素。

### ReactDOM 包

处理元素的一种简便方式是使用 ReactDOM 包，它包含操作 DOM 的 API。注意，你创建的元素并不是浏览器 DOM 元素，而是普通的 JavaScript 对象；这些对象会通过 React 的 render 函数渲染到虚拟 DOM 的根元素，然后再挂载到浏览器 DOM 上。

React 元素是不可变的：一旦创建，就无法更改。如果你确实修改了元素的任何部分，React 会创建一个新元素并重新渲染虚拟 DOM，然后将虚拟 DOM 与浏览器 DOM 进行比较，以决定浏览器 DOM 是否需要更新。我们会使用 JSX 抽象来完成这些任务；尽管如此，理解 React 底层的工作原理仍然很有好处。如果想深入了解，可以查阅官方文档 https://react.dev/learn。

## 将代码组织成组件

前面提到，组件是由 React 元素构建的独立、可复用的代码片段。元素是可以包含其他元素的对象，一旦渲染到虚拟 DOM 或浏览器 DOM，它们就会创建 DOM 节点或整棵 DOM 子树。而 React 组件是输出元素并将其渲染到虚拟 DOM 的类或函数。我们将使用 React 组件来构建用户界面。关于这一区别的更多信息，可以阅读 React 官方博客的深入解析：https://reactjs.org/blog/2015/12/18/react-components-elements-and-instances.html。

其他框架可能按技术来拆分用户界面的代码，把它分成 HTML、CSS 和 JavaScript 文件；React 则相反，按这些逻辑构建单元来拆分代码。因此，无论底层技术是什么，一个物理文件就包含了组件所需的全部信息。

更具体地说，React 组件是一个按照约定以大写字母开头的 JavaScript 函数。此外，它接受一个单独的对象参数——属性（props），并返回一个 React 元素。这个 props 参数在组件内部绝不应被修改，在 React 代码中被视为不可变的。

代码清单 4-2 展示了一个基本的 React 组件，它显示与前几个代码清单相同的天气字符串。此外，我们还添加了一个自定义接口和一个点击处理器。自定义接口使我们能够在 JSX 组件上设置一个属性，并在 TypeScript 代码中读取它的值。这是在不使用全局状态管理库的情况下向函数组件传值的常用方式。

在这里，我们只是把与前几个代码清单相同的字符串传给组件并把它渲染到 DOM；但在真实应用中，天气字符串可能是 API 响应的一部分。为了获取天气数据，父组件可以查询 API，然后通过组件的属性把这些数据传给组件的代码；否则，应用中的每个组件都得自行查询 API 才能访问这些数据，从而影响应用的整体性能。

点击处理器让我们能够对用户交互做出响应。在 JSX 中，点击处理器与 HTML 中的名称相同，我们添加它们的方式也类似于添加内联 DOM 事件。例如，要响应用户点击某个元素，我们添加一个带有回调函数的 onClick 属性。

```jsx
import React from "react";

export default function App() {

    interface WeatherProps {
        weather: string;
    }

    const clickHandler = (text: string): void => {
        alert(text);
    };

    const WeatherComponent = (props: WeatherProps): JSX.Element => {
        const text = `The weather is ${props.weather}`;
        return (<h1 onClick={() => clickHandler(text)}>{text}</h1>);
    };

    return (<WeatherComponent weather="sunny" />);
}
```

*代码清单 4-2：一个基本的 React 组件*

首先，我们为新组件的属性创建一个自定义接口。稍后我们会把这个接口用于组件的 props 参数。由于我们在组件上设置了 weather 属性，并在接口上定义了匹配的 weather 属性，因此在 TypeScript 代码中可以通过 props.weather 访问 weather 属性的值。

然后，我们把事件处理器创建为一个接受一个字符串参数的箭头函数。我们使用与内联 DOM 事件类似的 onClick 事件属性，并为其指定回调函数 clickHandler。用户一旦点击页面的标题，我们就会显示一个简单的提示框。

接着，我们定义组件。可以看到，它是一个实现了 WeatherProps 接口并返回 JSX 元素的 JSX 表达式。在组件内部，我们使用不带标签的模板字面量创建文本，并通过 props.weather 把来自 weather 属性的值作为动态天气信息加进去。然后我们返回这个 JSX 元素；最后，返回并渲染天气组件，把 sunny 设置为该属性的值。

把这段代码粘贴到 App.tsx 文件中。浏览器应在预览窗格中渲染出一个内容为 The weather is sunny 的 h1 HTML 元素。点击这段文字时，会再次弹出提示框显示它。你可以修改 weather 属性的值来显示不同的天气字符串。

### 编写类组件

React 中有两类组件：类组件和函数组件。代码清单 4-2 中的组件是函数组件，它在很大程度上借鉴了函数式编程。具体来说，这类组件遵循纯函数的模式：它们基于某些输入（props 参数和 JSX 组件的属性）创建某些输出（JSX 元素）。虽然本章重点介绍这类组件，你也应当了解类组件的基础知识。

类组件遵循面向对象编程的典型模式：它被定义为一个类，并从其父类 React.Component 继承方法。与所有组件一样，它有一个名为 props 的参数并返回一个 JSX 元素。类组件还有 constructor 和 super 函数，并且可以使用 this 关键字来引用当前组件的实例。

特别有价值的是，内部属性 this.state 为你提供了一个接口，用于存储和访问组件内部状态的相关信息，例如被打开的元素、图片画廊中当前显示的图片，或者如下一个示例中的简单点击计数器。同样重要的是类的生命周期方法，它们在特定的生命周期阶段运行：例如，每当组件挂载、渲染、更新或卸载时。在代码清单 4-3 中，我们使用了 componentDidMount 生命周期方法。React 会在组件成为 DOM 一部分之后立即运行这个方法，它类似于你可能已经熟悉的浏览器 DOMReady 事件。

代码清单 4-3 把前面创建的天气组件定义成了类组件。为了练习访问组件的状态，我们添加了一个计数器，用来统计对标题元素的点击次数。由于它记录的是组件的内部状态，页面重新加载时计数器会归零。把这段代码粘贴到 App.tsx 文件中，然后点击标题让数字增长。

```jsx
import React from "react";

export default function App() {
    interface WeatherProps {
        weather: string;
    }

    type WeatherState = {
        count: number;
    };

    class WeatherComponent extends React.Component<WeatherProps, WeatherState> {
        constructor(props: WeatherProps) {
            super(props);
            this.state = {
                count: 0
            };
        }

        componentDidMount() {
            this.setState({count: 1});
        }

        clickHandler(): void {
            this.setState({count: this.state.count + 1});
        }

        render() {
            return (
                <h1 onClick={() => this.clickHandler()}>
                    The weather is {this.props.weather}, and the counter shows{" "}
                    {this.state.count}
                </h1>
            );
        }
    }

    return (<WeatherComponent weather="sunny" />);
}
```

*代码清单 4-3：一个基本的 React 类组件*

首先，我们定义用于组件属性的自定义接口，同时定义一个类型，供稍后创建的计数器使用。

接着，我们通过继承基类 React.Component 定义类组件。遵循面向对象编程模式，constructor 调用 super 函数并初始化组件的状态，我们把计数器设为 0。浏览器一挂载这个组件，就会调用生命周期方法 componentDidMount，把组件的 count 变量改为 1。我们把点击处理器改为统计点击次数，而不是显示提示框，并调用 render 函数。在这里，我们返回以 HTML 形式显示 weather 属性和当前状态的 JSX 元素。

最后，我们返回 WeatherComponent，React 会对它进行初始化。预览窗格显示字符串 The weather is sunny，计数器显示 1。从数字 1 可以看出，生命周期方法确实被调用了。由于组件状态的响应式特性，每点击一次标题，数字都会立即增加。状态一发生变化，React 就会重新渲染组件，并用状态的当前值更新视图。

### 用钩子提供可复用行为

函数组件可以使用钩子来提供可复用的行为，例如访问组件的状态。钩子是一些函数，它们为状态和生命周期特性提供简单且可复用的接口。代码清单 4-4 展示了我们在代码清单 4-3 中创建的同一个天气组件，这次改写成了函数组件。它使用钩子而非生命周期方法来更新组件的计数器。

```jsx
import React, {useState,useEffect} from "react";

export default function App() {

    interface WeatherProps {
        weather: string;
    }

    const WeatherComponent = (props: WeatherProps): JSX.Element => {

        const [count, setCount] = useState(0);
        useEffect(() => {setCount(1)},[]);

        return (
            <h1 onClick={() => setCount(count + 1)}>
                The weather is {props.weather},
                and the counter shows {count}
            </h1>
        );
    };

    return (<WeatherComponent weather="sunny" />);
}
```

*代码清单 4-4：使用钩子的 React 函数组件*

我们为这个组件添加了两个新特性：一个组件状态的指示器，以及在挂载组件后立即运行代码的方式。为此，我们使用 useState 和 useEffect 这两个钩子：先把它们作为具名导入从 React 模块导入，再把它们加入函数组件。useState 钩子替代了类组件中的 this.state 属性，useEffect 则替代了 componentDidMount 生命周期方法。此外，我们把上一个例子中的 clickHandler 替换成了一个简单的内联函数来更新计数器。

对钩子的每次调用都会产生一个完全隔离的状态，因此我们可以在同一个组件中多次使用同一个钩子，并确信状态会正确更新。这种模式让钩子回调保持小巧而专注。另请注意，运行时不会提升钩子：它们按照我们在代码中定义的顺序被调用。

对比代码清单 4-3 和 4-4，你会立刻看出函数组件可读性更强、更容易理解。正因如此，在本书剩下的部分中，我们将只使用函数组件。

## 使用内置钩子

React 提供了一组内置钩子。你刚刚见过了最常见的 useState 和 useEffect。另一个有用的钩子是 useContext，用于在组件之间共享数据。其他内置钩子覆盖更具体的使用场景，用于提升应用的性能或处理特定的边界情况。你可以按需在 React 文档中查阅它们。

当你需要把一个庞大的组件拆分成更小的、可复用的代码包时，也可以创建自定义钩子。自定义钩子遵循特定的命名约定：以 use 开头，后跟一个以大写字母开头的动作。每个钩子应只定义一个功能，以便于测试。

本节将带你了解三个最常用的钩子以及使用它们的好处。

### 用 useState 管理内部状态

纯函数只使用函数内部可用的数据，但它仍然可以对局部状态的变化做出响应，例如我们创建的天气组件中的计数器。useState 钩子可能是处理局部状态时使用最多的钩子。这种组件内部状态只在组件内部可用，永远不会暴露给外部。

由于组件状态是响应式的，一旦我们更新状态，React 就会重新渲染组件，使这个值在整个组件中发生变化。不过，React 保证状态是稳定的，不会在重新渲染时发生改变。

useState 钩子返回响应式的状态变量和一个用于设置状态的 setter 函数，如代码清单 4-5 所示。

```javascript
const [count, setCount] = useState(0);
```

*代码清单 4-5：单独查看的 useState 钩子*

我们用默认值初始化 useState 钩子。钩子本身返回状态变量 count 以及修改该状态变量值所需的 setter 函数，因为我们不能直接修改这个变量。例如，要把代码清单 4-5 中创建的状态变量 count 设为 1，需要以新值作为参数调用 setCount 函数，即 setCount(1)。按照约定，setter 函数以 set 开头，后跟状态变量的名称。

### 用 useEffect 处理副作用

纯函数应当只依赖传给它们的数据。当函数使用或修改其局部作用域之外的数据时，我们称之为副作用。副作用最简单的例子是修改全局变量，无论在 JavaScript 还是函数式编程中，这都被认为是一种不良实践。

然而，有时我们的组件需要与“外部世界”交互，或者存在外部依赖。在这些情况下，我们可以使用处理副作用的 useEffect 钩子，它为组件的函数式特性提供了一个“逃生舱口”。例如，useEffect 可以管理依赖、调用 API 以及获取组件所需的数据。

这个钩子在 React 把组件挂载进布局且组件的渲染过程完成之后运行。它有一个可选的返回对象，会在组件被卸载之前运行。你可以用它做清理工作，例如移除事件监听器。

使用这个钩子的一种方式是观察依赖并对其变化做出响应。为此，我们可以传入一个可选的依赖数组。其中任何一个依赖发生变化，都会触发钩子重新运行。如果依赖数组为空，钩子就不依赖任何外部值，也永远不会重新运行。我们的天气组件就是这种情况：useEffect 只在组件挂载和卸载后执行。它没有任何外部依赖，因此依赖数组保持为空，钩子只运行一次。

### 用 useContext 与 Context Provider 共享全局数据

理想情况下，React 的函数组件应当是只操作通过 props 参数传入的数据的纯函数。可惜，组件有时需要消费共享的全局状态。在这种情况下，React 实现了 context provider 来与一棵子组件树共享全局数据。

context provider 会包裹子组件，我们可以用 useContext 钩子访问共享数据。当 context 的值发生变化时，React 会自动重新渲染所有子组件。因此，这是一个开销相当大的钩子，不应把它用于频繁变化的数据集。

在第二部分要构建的全栈应用中，你将使用 useContext 与子组件共享会话数据。共享 context 也常用来跟踪配色方案和主题。代码清单 4-6 展示了如何通过 context provider 消费一个主题。

```jsx
import React, {useState, createContext, useContext} from "react";

export default function App() {
    const ThemeContext = createContext("");

    const ContextComponent = (): JSX.Element => {

        const [theme, setTheme] = useState("dark");

        return (
            <div>
                <ThemeContext.Provider value={theme}>
                    <button onClick={() => setTheme(theme == "dark" ? "light" : "dark")}>
                        Toggle theme
                    </button>
                    <Headline />
                </ThemeContext.Provider>
            </div>
        );
    };

    const Headline = (): JSX.Element => {
        const theme = useContext(ThemeContext);
        return (<h1 className={theme}>Current theme: {theme}</h1>);
    };

    return (<ContextComponent />);
}
```

*代码清单 4-6：一个完整的 context provider 示例*

首先，我们从 React 包导入所需的函数，并用 createContext 函数初始化 ThemeContext。接着，我们创建父组件并将其命名为 ContextComponent。它是承载 context provider 和所有子组件的包装组件。

在 ContextComponent 中，我们用 useState 创建局部变量 theme，并把这一状态变量设置为 context 提供的内容。这使我们能够在子组件内部更改 context 中的变量。由于我们使用响应式的状态变量作为值，所有子组件中的 theme 变量实例都会立即更新。

我们添加了一个按钮元素，每当用户点击按钮时，就在 light 和 dark 之间切换状态变量的值。最后，我们创建 Headline 组件，它调用 useContext 钩子来获取 ThemeContext 提供给所有子组件的 theme 值。Headline 组件将 theme 值用作 HTML class，并显示当前主题。

## 练习 4：为 Express.js 服务器创建响应式用户界面

让我们运用新学的知识和天气组件，为 Express.js 服务器创建一个响应式用户界面。新的 React 组件将让我们能够通过点击来更新网页上的文字。

### 将 React 添加到服务器

首先，我们要把 React 引入项目。出于实验目的，你可以把 React 库和独立版的 Babel.js 转译器直接添加到 HTML 的 head 标签内。但要注意，这种技术并不适合生产环境：在浏览器中转译代码是一个缓慢的过程，而且我们在这里添加的 JavaScript 库也没有经过优化。把 React 与骨架 Express.js 服务器配合使用需要相当多繁琐的设置步骤和大量的维护工作。我们将在第 5 章使用 Next.js 来简化 React 应用的开发。

在 package.json 文件旁边创建一个名为 public 的文件夹，然后在其中创建一个名为 weather.html 的空文件。加入代码清单 4-7 中的代码，它包含带有天气组件的 React 示例。稍后，我们将创建一个新的端点 /components/weather，直接返回这个 HTML 文件。

```html
<!DOCTYPE html>
<html>
    <head>
        <meta charset="UTF-8" />
        <title>Weather Component</title>
        <script src="https://unpkg.com/react@18/umd/react.development.js"></script>
        <script src="https://unpkg.com/react-dom@18/umd/react-dom.development.js"></script>
        <script src="https://unpkg.com/@babel/standalone/babel.min.js"></script>
    </head>
    <body>
        <div id="root"></div>

        <script type="text/babel">
            function App() {

                const WeatherComponent = (props) => {

                    const [count, setCount] = React.useState(0);
                    React.useEffect(() => {
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
          }

            const container = document.getElementById("root");
            const root = ReactDOM.createRoot(container);
            root.render(<App />);
        </script>
    </body>
</html>
```

*代码清单 4-7：静态文件 /public/weather.html 在浏览器中渲染 React*

首先，我们向 weather.html 文件添加三个 React 脚本：react.development、react.dom.development 和独立版的 babel.js，它们都类似于我们之前在 App.tsx 文件中对 React 的导入。接着，我们添加 ReactDOM，让 React 能够与 DOM 交互。这三个文件会向 window 对象添加一个全局属性 React，我们把这个属性当作全局变量来引用 React 的函数。独立版 Babel 脚本添加了 Babel.js 转译器，我们需要它把代码从 JSX 转换为 JavaScript。

接着，我们加入之前开发的天气组件代码。这次不再引用 App.tsx 文件，而是把应用函数直接放在 HTML 文件内，并把 script 块标记为 text/babel。这个类型告诉 Babel，把 script 标签内的代码转译成标准 JavaScript。

我们对天气组件的代码做了几处简单的修改。首先，移除类型标注，因为它们只允许出现在 TypeScript 文件中。然后，由于我们使用的是浏览器环境，我们在钩子前面加上它们的全局属性名 React。最后，我们用 ReactDOM 创建 React 根容器，并在其中渲染 <App /> 组件。

### 为静态 HTML 文件创建端点

我们要编辑的第二个文件是根目录下的 index.ts。我们加入代码清单 4-8 中的高亮代码，以添加新的入口 /components/weather。

```typescript
import {routeHello, routeAPINames, routeWeather} from "./routes.js";
import express, {Request, Response} from "express";

import path from "path";

const server = express();
const port = 3000;

--snip--
server.get("/components/weather", function (req: Request, res: Response): void {
    const filePath = path.join(process.cwd(), "public", "weather.html");
    res.setHeader("Content-Type", "text/html");
    res.sendFile(filePath);
});

server.listen(port, function (): void {
    console.log("Listening on " + port);
});
```

*代码清单 4-8：重构后的 index.ts*

为了加载静态 HTML 文件，我们从 Node.js 默认的 path 模块导入 path。path 模块提供了处理文件和目录的各类工具。我们将使用其中的 join 函数来创建一个符合操作系统格式的有效路径。

我们使用默认的全局函数 process.cwd 获取当前工作目录，然后在此基础上创建指向 HTML 文件的路径。接着，我们添加天气组件的入口点，并把响应的 Content-Type 头设置为 text/html。最后，我们使用 sendFile 函数把之前创建的 weather.html 文件发送给浏览器。

### 运行服务器

我们需要把服务器代码转译成 JavaScript，因此在命令行用 npx 运行 TSC：

```bash
$ npx tsc
```

生成的文件 index.js 和 routes.js 与之前创建的类似。TSC 不会改动静态 HTML；独立版 Babel.js 脚本会在运行时于浏览器中转换 JSX 代码。在命令行启动服务器：

```bash
$ node index.js
Listening on 3000
```

现在在浏览器中访问 http://localhost:3000/components/weather-component。你会看到与在 React 在线运行环境中渲染天气组件时相同的文字，如图 4-2 所示。只要点击这段文字，点击处理器就会增加响应式状态变量的值，计数器显示出新的数值。

你已经成功创建了第一个 React 应用。要获得更多 React 经验，请尝试为点击计数器添加一个自定义的按钮组件，其 style 属性使用一个 JSX 表达式，在计数器值为奇数和偶数时改变背景颜色。

## 小结

现在，你应该已经具备了创建 React 应用的坚实基础。JSX 元素是 React 组件的构建单元，组件返回 JSX，经由 React 的虚拟 DOM 在 DOM 中渲染成 HTML。你还探究了类组件与现代函数组件的区别，深入学习了 React 钩子，并用这些钩子构建了一个函数组件。

如果你想探索 React 的全部潜力，可以看看 W3Schools 的 React 教程（https://www.w3schools.com/REACT/DEFAULT.ASP），以及 React 团队创建的教程（https://react.dev/learn/tutorial-tic-tac-toe）。

下一章我们将使用 Next.js。Next.js 构建在 React 之上，是一个可用于生产环境的全栈 Web 开发框架，面向单页应用。
