# 第 2 章 现代 JavaScript

*Chapter 2: Modern JavaScript*

在第 1 章中，你使用基础的 JavaScript 搭配 Node.js 创建了一个 Web 服务器。现在，我们将更深入地研究这门语言的更多高级特性，以及如何有效地运用它们来创建全栈 Web 应用。

你有时会听到用术语 ES.Next 来指代 JavaScript 的新版本。在本书中，我们把 ES.Next 用作一个宽泛的标签，涵盖现代 JavaScript 及其概念。大多数运行时环境已经实现了这里介绍的特性；即便没有实现，你也可以用 Babel.js 对它们进行转译，生成向后兼容的 JavaScript，在较旧的运行时上模拟这些新特性。

## ES.Next 模块

ES.Next 模块允许你把代码拆分到不同的文件中，以提高可维护性和可测试性。它们把一段逻辑封装成易于复用的代码；由于变量和函数被限制在模块的作用域（scope）内，你可以在不同模块中使用相同的变量名而不会发生冲突。（我们将在第 17 页的“声明变量”一节中讨论作用域的概念。）

官方的 ES.Next 模块取代了各种非官方的模块格式，比如 UMD 和 AMD——那些格式需要通过 `require` 语句加载。例如，在第 1 章的 Node.js 服务器代码中，你就是用 `require` 引入 Express.js 包的。ES.Next 模块则改用 `export` 和 `import` 语句：把函数从一个模块的作用域导出，再到别处导入使用。换句话说，模块让你能够创建函数和变量，并把它们暴露给新的作用域。

### 使用具名导出和默认导出

ES.Next 的导出有两种：具名导出（named export）和默认导出（default export）。这两种导出在后续导入时使用的语法略有不同。默认导出要求你在导入时定义新的函数名；具名导出的重命名则是可选的，通过 `as` 语句完成。

人们普遍认为具名导出优于默认导出，因为具名导出为模块的功能定义了清晰且唯一的接口。使用默认导出时，用户可能以不同的名称导入同一个函数。我们将在第 3 章介绍的 TypeScript 建议：如果模块只有一个明确的用途、只导出一项内容，就使用默认导出；反之，只要模块导出的内容不止一项，就应使用具名导出。

你应当了解默认导出的语法，这样才能使用那些采用了默认导出的第三方模块。与具名导出不同，每个文件只能有一个默认导出，用 `default` 关键字标记（代码清单 2-1）。

```javascript
const getFoo = function () {
    return 'foo';
};

export default getFoo;
```

*代码清单 2-1：默认导出*

在这个清单中，我们定义了一个匿名函数，把它存储在常量 `getFoo` 中，然后用 `default` 关键字导出这个常量，使其成为模块的默认导出。

具名导出既可以内联导出，也可以在文件末尾用花括号（{}）导出。代码清单 2-2 展示了几个具名导出。

```javascript
export const getFooBar = function () {
    return 'foo bar';
};

const getBar = function () {
    return 'bar';
};

const getBaz = function () {
    return 'baz';
};

export {getBar, getBaz};
```

*代码清单 2-2：具名导出*

这里我们定义了一个匿名函数，把它存储在常量 `getFooBar` 中，并立即以 `getFooBar` 之名导出。接着我们又定义了两个匿名函数，把它们放在花括号中作为具名导出。

### 导入模块

导入 ES.Next 模块的语法取决于你创建的导出类型。具名导出必须用花括号导入，默认导出则不需要。在代码清单 2-3 中，我们使用 `import` 语句导入默认导出 `getFoo`，`import` 后面跟着我们为它指定的本地名称。最后，我们以包含代码的文件路径结束这条导入语句。

```javascript
import getFoo from "default.js";
```

*代码清单 2-3：导入默认导出*

代码清单 2-4 中的具名导出遵循类似的模式，区别在于我们需要在花括号内引用原始的函数名。如果想在本地重命名这些函数，必须通过 `as` 语句显式地进行，而通常没有理由这么做。

```javascript
import {getFooBar, getBar, getBaz} from "named.js";
```

*代码清单 2-4：导入具名导出*

现在你就可以在代码中使用这些导入的函数了，因为它们在你导入它们的作用域中可用。

## 声明变量

JavaScript 提供了三种声明变量的方式：`var`、`let` 和 `const`。本节讨论它们各自的使用场景。你常常会得到这样的建议：避免使用 `var`，因为它“过时了”。你可以放心，`var` 并没有过时，而且你必须理解这几种变量声明方式，才能为工作选择合适的工具。

这些变量的差异在于各自的作用域，作用域定义了我们可以访问和使用它们的代码区域。JavaScript 有多个层级的作用域：全局作用域、模块作用域、函数作用域和块级作用域。块级作用域适用于任何由花括号括起来的代码块，是作用域的最小单位。每次使用花括号，都会创建一个新的块级作用域。相比之下，定义函数时就会产生函数作用域，该作用域局限于特定函数内部的代码区域。模块作用域只作用于特定模块，而全局作用域则作用于整个程序。在全局作用域中定义的变量，在代码的任何部分都可用。

正如你将在接下来的代码清单中看到的，变量在自己的作用域及其所有子作用域中始终可用。因此请记住，举例来说，一个函数作用域可以包含多个块级作用域。只要每个变量位于不同的作用域中，同一个变量名就可以在同一个程序中定义两次。

### 被提升的变量

传统 JavaScript 使用 `var` 关键字声明变量。这些变量的作用域是当前执行上下文（通常是外层函数）。如果在任何函数之外声明，变量的作用域就是全局的，并且该变量会在全局对象上创建一个属性。

与所有其他变量不同，运行时环境在执行时会把 `var` 的声明移动（即提升，hoisting）到其作用域的顶部。因此，你可以在代码中先使用这些变量、后定义它们。代码清单 2-5 展示了一个简短的提升示例。

```javascript
function scope() {
    foo = 1;
    var foo;
}
```

*代码清单 2-5：在定义之前使用被提升的变量*

在这个清单中，我们在声明变量的前一行就为它赋了值。在 Java 和 C 这类语言中，变量必须先声明后使用，任何提前使用都会抛出错误。然而，由于 JavaScript 的提升机制，解析器会把所有用 `var` 关键字定义的变量声明移到作用域顶部。因此，这段代码等价于代码清单 2-6。

```javascript
function scope() {
    var foo;
    foo = 1;
}
```

*代码清单 2-6：先定义变量再使用*

由于提升的存在，块级作用域并不适用于用 `var` 关键字声明的变量——它们总是会被提升。为了说明这一点，请看代码清单 2-7：我们用 `var` 关键字声明了一个全局变量 `globalVar`、函数作用域内的变量 `foo`，以及块级作用域内的变量 `bar`。

```javascript
var globalVar = "global";
function scope() {
    var foo = "1";
    if (true) {
        var bar = "2";
    }
    console.log(globalVar);
    console.log(window.globalVar);
    console.log(foo);
    console.log(bar);
}
scope();
```

*代码清单 2-7：`var` 的作用域*

我们运行 `scope` 函数，可以看到 `globalVar` 和 `window.globalVar` 是相同的；解析器把 `foo` 和 `bar` 两个变量都提升到了函数作用域的顶部。因此，变量 `bar` 在块级作用域之外也可用，函数作用域把两个变量的值 1 和 2 都写入了控制台。

### 遵守作用域规则的变量

现代 JavaScript 引入了 `let` 关键字来补充 `var`。使用 `let`，我们可以声明块级作用域的变量，它们只有在声明之后才能被访问。因此，它们被视为非提升变量。块级作用域变量被限制在其定义所在的块语句（block statement）作用域内。与用 `var` 定义的全局变量不同，用 `let` 定义的全局变量不会被添加到 `window` 对象上。

让我们看看用 `let` 声明的变量的作用域。在代码清单 2-8 中，我们在函数作用域内声明变量 `foo`，在块级作用域内声明变量 `bar`，并声明一个全局变量 `globalVar`。

```javascript
let globalVar = "global";
function scope() {
    let foo = "1";
    if (true) {
        let bar = "2";
    }
    console.log(globalVar);
    console.log(window.globalVar);
    console.log(foo);
    console.log(bar);
}
scope();
```

*代码清单 2-8：`let` 的作用域*

每个变量只在其各自的作用域中可用。解析器不会提升它们，因此变量 `bar` 在块语句之外不可用。如果你试图在其他地方引用它，解析器会抛出错误，提示 `bar` 未定义。

我们执行这个函数，与 `var` 版本的代码不同，它只把 `foo` 的值写入了控制台。当我们试图访问 `bar` 时，会收到错误 `Uncaught ReferenceError: bar is not defined`。对于 `globalVar`，我们在控制台上看到输出值 `global`，而 `window.globalVar` 则是 `undefined`。

### 类常量数据

现代 JavaScript 引入了另一个新关键字 `const`，用于声明常量一类的数据类型。与 `let` 一样，`const` 在全局声明时也不会在全局对象上创建属性。它们同样被视为非提升的，因为在声明之前无法访问。

JavaScript 中的常量与许多其他语言中的常量不同，后者表现为不可变的数据类型。在 JavaScript 中，常量只是看起来不可变。实际上，它们是指向其值的只读引用。因此，对于基本数据类型（primitive data type），你不能直接给变量标识符重新赋另一个值。但对象或数组是非基本数据类型，所以即使使用 `const`，你仍然可以通过方法或直接的属性访问来修改它们的值。

在代码清单 2-9 中，我们用 `const` 关键字声明了一个基本数据类型和一个非基本数据类型，并尝试改变它们的内容。

```javascript
const primitiveDataType = 1;
try {
    primitiveDataType = 2;
} catch (err) {
    console.log(err);
}

const nonPrimitiveDataType = [];
nonPrimitiveDataType.push(1);

console.log(nonPrimitiveDataType);
```

*代码清单 2-9：使用 `const` 声明基本类型和非基本类型*

我们声明了两个类常量数据结构并为其赋值。当我们试图给基本数据结构重新赋值时，运行时会抛出错误 `Attempted to assign to readonly property`。因为使用了 `const`，我们无法重新赋值。相比之下，我们可以修改 `nonPrimitiveDataType` 数组（这里用 `push` 方法完成），在不报错的情况下追加一个值。此时数组应包含一个值为 1 的元素，所以我们会在控制台看到 `[1]`。

## 箭头函数

现代 JavaScript 引入了箭头函数（arrow function）作为常规函数的替代。关于箭头函数，你需要了解两个概念。第一，它们使用与常规函数不同的语法：定义箭头函数要快得多，只需要几个字符和一行代码。第二个重要但不那么明显的变化是，它们使用一种称为词法作用域（lexical scope）的机制，这使得它们更直观、更不容易出错。

### 编写箭头函数

声明箭头函数时不使用 `function` 关键字，而是用等号和大于号组成一个箭头（=>）。这种语法也称为胖箭头（fat arrow），减少了噪音，使代码更加紧凑。因此，现代 JavaScript 在把函数作为参数传递时更倾向于使用这种语法。

此外，如果箭头函数只有一个参数和一条语句，我们可以省略花括号和 `return` 关键字。在这种紧凑形式下，我们称该函数为简洁体函数（concise body function）。代码清单 2-10 展示了一个传统函数的定义，后面跟着一个箭头函数。

```javascript
const traditional = function (x) {
    return x * x;
}

const conciseBody = x => x * x;
```

*代码清单 2-10：传统函数与使用简洁体语法的箭头函数*

我们先用 `function` 关键字和熟悉的 `return` 语句定义了一个标准函数，然后用简洁体语法的箭头函数实现了同样的功能。这里我们省略了花括号，使用隐式返回语句，不需要 `return` 关键字。

### 理解词法作用域

与常规函数不同，箭头函数不会把自身的作用域绑定到调用该函数的对象上，而是使用词法作用域——由外围作用域决定 `this` 关键字的值。因此，箭头函数中 `this` 所指的作用域，始终代表定义箭头函数的对象，而不是调用该函数的对象。代码清单 2-11 展示了词法作用域和定义作用域的概念。

```javascript
this.scope = "lexical scope"; // 1

const scopeOf = {
  scope: "defining scope", // 2

    traditional: function () {
      return this.scope; // 3
    },

    arrow: () => {
        return this.scope;
    },
};

console.log(scopeOf.traditional());
console.log(scopeOf.arrow());
```

*代码清单 2-11：箭头函数的作用域*

我们首先在词法作用域上声明 `scope` 属性（标记 1），这就是定义对象。然后我们在定义作用域内创建一个带有同名属性的对象（标记 2）。接着我们定义了两个函数，它们都用 `this` 返回 `this.scope` 的值（标记 3）。

调用它们之后，你可以看到两种引用的区别：箭头函数中的 `this.scope` 指向词法作用域中定义的属性，而传统函数的 `this` 指向我们定义的第二个属性。因此，`scopeOf.traditional` 函数输出 `defining scope`，而 `scopeOf.arrow` 函数输出 `lexical scope`。

### 探索实际用例

由于函数在 JavaScript 中是一等公民，我们可以把它们作为参数传递给其他函数。在第 1 章中，你曾在 Node.js 里用这种模式定义回调（callback）；更早之前，在浏览器中处理事件处理器时你也用过。但是，当使用常规函数作为回调时，即使回调中的实际代码非常简单，代码也会很快被 `function` 语句和花括号搞得杂乱不堪。箭头函数为回调提供了干净简洁的语法。在代码清单 2-12 中，我们在数组的 `filter` 方法上使用回调，并分别将其定义为传统函数和箭头函数。

```javascript
let numbers = [-2, -1, 0, 1, 2];

let traditional = numbers.filter(function(num) {
      return num >= 0;
    }
);

let arrow = numbers.filter(num => num >= 0);

console.log(traditional);
console.log(arrow);
```

*代码清单 2-12：把胖箭头函数作为参数传递*

第一个版本的回调是传统函数，第二个实现则使用了简洁体语法的箭头函数。两者返回相同的数组：`[0, 1, 2]`。可以看到，实际的功能——从数组中移除所有负数——只是简单地检查当前项是否大于等于零。传统函数需要额外的字符，理解起来更费劲。一旦你完全掌握箭头语法，就能提升代码的可读性，进而提高代码质量。

## 创建字符串

现代 JavaScript 引入了非标签模板字符串（untagged template literal）和标签模板字符串（tagged template literal）。模板字符串是向字符串中添加变量和表达式的简单方式。这种字符串插值（string interpolation）可以跨越多行，并且可以包含单引号和双引号而无需转义。我们用反引号（`` ` ``）包裹模板字符串，并用美元符号（$）和花括号来标记模板中的变量或表达式。

非标签模板字符串就是用反引号包裹的字符串。解析器会对其中的变量和表达式进行插值，并返回一个字符串。作为全栈开发者，每当你想向字符串中添加变量或拼接多个字符串时，都会用到这种模式。代码清单 2-13 展示了一个非标签模板字符串的例子。它们可以跨越多行，而不需要任何控制字符。

```javascript
let a = 1;
let b = 2;
let string = `${a} + ${b} = ${a + b}`;
console.log(string);
```

*代码清单 2-13：非标签模板字符串*

解析器会替换占位符并求值模板字符串中的表达式，得到字符串 `1 + 2 = 3`。

一旦模板字符串前面跟了一个表达式，它就变成了标签模板字符串。在这种情况下，函数会同时接收模板字符串和替换值作为参数，先对两者进行处理，然后返回一个值。返回值可以是任何基本或非基本类型。在代码清单 2-14 中，我们用一个自定义函数配合标签模板字符串来做数字的加减运算，并用文字解释这个过程。

```javascript
function tag(literal, ...values) {
    console.log("literal", literal);
    console.log("values", values);

    let result;
    switch (literal[1]) {
        case " plus ":
            result = values[0] + values[1];
            break;
        case " minus ":
            result = values[0] - values[1];
            break;
    }
    return `${values[0]}${literal[1]}${values[1]} is ${result}`;
}

let a = 1;
let b = 2;
let output = tag`What is ${a} plus ${b}?`;

console.log(output);
```

*代码清单 2-14：一个基础的标签模板字符串*

这里，解析器调用 `tag` 表达式，然后把模板字符串和替换值作为参数传给该函数。函数根据这些参数构造出一个字符串并返回。

让我们更深入地研究一下这段代码。在 `tag` 表达式中，第一个参数 `literal` 是一个在变量处被切分的数组，形如 `['What is ', ' minus ', '?']`。参数 `values` 也是一个数组，包含我们传给函数的模板字符串变量的值：`[1, 2]`。我们用一个简单的 `switch`/`case` 语句根据 `literal` 和 `values` 计算结果。最后，我们返回一个带有“问题”答案的新字符串，并在控制台上看到 `1 plus 2 is 3`。

标签模板字符串为复杂的字符串替换提供了简单的接口，是在 JavaScript 中创建领域特定语言（DSL，domain-specific language）的一种优雅方式。DSL 是一种面向特定领域、用于解决特定任务的语言，与通用语言相对——比如 JavaScript，我们可以用它解决各种各样的软件相关问题。DSL 的一个常见例子是 HTML：在 Web 开发领域，我们用它来标记文本，但不能用它进行数学运算或读取文件内容。你将在全栈开发中用 GraphQL 模式（schema）定义自己的 DSL。当你在第 6 章定义第一个 GraphQL 模式时，你会明白它的 DSL 不过是一个标签模板字符串。

## 异步脚本

JavaScript 是单线程的，这意味着它一次只能运行一个任务。因此，长时间运行的任务可能会阻塞应用程序。一个简单的解决方案是异步编程（asynchronous programming）——一种在不阻塞整个应用程序的情况下启动长时间运行任务的模式。当脚本等待结果时，应用程序的其余部分仍然可以响应交互或用户界面事件，并执行其他计算。

### 避免传统回调

传统 JavaScript 用回调函数来实现异步代码，回调函数会在另一个函数返回结果之后执行。当你的代码需要响应某个事件而不是立即运行时，你可能已经用过回调。在全栈 Web 开发中，这种技术的一个常见用例是在 Node.js 中执行 I/O 操作或调用远程 API。代码清单 2-15 给出了一个 I/O 操作的例子。我们导入处理文件系统操作的 Node.js `fs` 模块，并使用回调函数在操作结束后立即显示文件内容。

```javascript
const fs = require("fs");

const callback = (err, data) => {
    if (err) {
        return console.log("error");
    }
    console.log(`File content ${data}`);
};

fs.readFile(" file.txt", callback);
```

*代码清单 2-15：在 Node.js 中用回调函数读取文件*

读取文件是异步脚本的一个常见例子。我们不希望应用程序在等待文件内容就绪时被阻塞；但同时，我们又需要在应用程序的特定部分使用文件的内容。

这里我们创建了回调函数，并把它作为参数传给 `fs.readFile` 函数。该函数从文件系统中读取文件，并在 I/O 操作失败或成功后立即执行回调。回调接收文件数据和一个可选的错误对象，目前我们只是把它们写入控制台。

回调是异步脚本的一种笨拙解决方案。一旦你有了多个相互依赖的回调函数，就会陷入所谓的回调地狱（callback hell）：每个回调函数都要把另一个回调函数作为参数，结果就是一座难以阅读、容易出错的函数金字塔。现代 JavaScript 引入了 promise 和 async/await 作为回调的替代方案。

### 使用 promise

promise 为可链式调用的异步任务提供了简洁得多的语法。与回调类似，它们把后续任务推迟到前一个操作完成或失败之后。从本质上讲，promise 是不会立即返回结果的函数调用；相反，它们承诺在之后的某个时刻返回结果。如果出现错误，promise 会被拒绝（rejected）而不是被解决（resolved）。

`Promise` 对象有两个属性：状态（state）和结果（result）。当状态为 `pending` 时，结果是 `undefined`。promise 一旦解决，状态就变为 `fulfilled`，结果则反映返回值。如果 promise 是被拒绝的，状态同样被设为 `rejected`，结果中包含一个错误对象。

promise 遵循一种独特的语法。要使用它们，你首先要创建一个新的 `Promise`，或者调用一个返回 `Promise` 的函数；然后消费这个 `Promise` 对象；最后进行清理。这通过注册消费函数 `then`、`catch` 和 `finally` 来完成。promise 最初会在状态从 `pending` 变为 `fulfilled` 时立即调用 `then`，并把返回的数据传递给它。后面的每个 `then` 方法都接收前一个的返回值，让你能够创建一条单一的任务链，来处理和操作这些返回值。

只有当错误发生在最初或任务链的后续环节时，promise 链才会调用 `catch` 方法。此外，这个特定 promise 的状态变为 `rejected` 时也会触发它。无论如何，解析器都会在这组 `then` 方法执行完毕或 `catch` 方法被调用之后调用 `finally` 方法。`finally` 方法用于清理任务，比如解锁用户界面或关闭数据库连接，它类似于 `try...catch` 语句中的 `finally` 调用。

你可以在任何函数中使用 promise。在代码清单 2-16 中，我们使用原生的 `fetch` API 来请求 JSON 数据。

```javascript
function fetchData(url) {
    fetch(url)
        .then((response) => response.json())
        .then((json) => console.log(json))
        .catch((error) => {
            console.error(`Error : ${error}`);
        });
}
fetchData("https://www.usemodernfullstack.dev/api/v1/users");
```

*代码清单 2-16：用 promise 获取远程数据*

与文件系统上的 I/O 操作一样，网络请求是会阻塞应用程序的长时间运行任务。因此，我们应该使用异步模式来加载远程数据集。与代码清单 2-15 一样，我们需要等操作完成后才能处理请求到的数据或处理错误。

`fetch` API 默认基于 promise。一旦 promise 解决、状态变为 `fulfilled`，后面的 `then` 函数就会接收到响应对象。接着我们解析数据，把 JSON 对象传递给 promise 链中的下一个函数——promise 链就是用点号（`.then`）连接起来的函数序列。如果出现错误，promise 会被拒绝。在这种情况下，我们捕获错误并把它写入控制台。

### 简化异步脚本

现代 JavaScript 引入了一种更简单的处理异步请求的新模式：async/await 关键字。借助这些关键字，我们可以编写结构上类似于常规同步代码的代码，而不必依赖链式函数。

使用这种模式时，你先用 `async` 显式地把函数标记为异步函数，然后在异步代码中用 `await` 代替基于 promise 的语法。在代码清单 2-17 中，我们将原生的 `fetch` API 与 async/await 结合使用，执行另一个长时间运行的任务，从远程位置获取 JSON 数据。这段代码在功能上与代码清单 2-16 相同，你应该能看出它的语法比一长串 `then` 调用更直观、更整洁。

```javascript
async function fetchData (url) {
    try {
        const response = await fetch(url);
        const json = await response.json();
        console.log(json);
    } catch (error) {
        console.error(`Error : ${error}`);
    }
}

fetchData("https://www.usemodernfullstack.dev/api/v1/users");
```

*代码清单 2-17：用 async/await 获取远程数据*

首先，我们把函数声明为 `async`，以便在函数内部使用 `await` 关键字。然后我们用 `await` 等待 `fetch` 调用的响应。与我们之前使用的 promise 语法不同，`await` 简化了代码：它等待响应对象并返回它。因此，这段代码块看起来与常规的同步代码十分相似。

这种模式要求我们手动处理错误。与 promise 不同，这里没有默认的 reject 函数。因此，我们必须把 `await` 语句包裹在 `try...catch` 块中，才能优雅地处理错误状态。

## 遍历数组

现代 JavaScript 引入了一整套新的数组函数。对全栈 Web 开发来说，最重要的是 `array.map`。它让我们可以对每个数组元素运行一个函数，并返回一个包含修改后元素的新数组，同时保留原始数组。开发者通常在 React 中用它来生成列表，或者用数组中的数据集填充 JSX。在第 4 章介绍 React 之后，你会大量使用这种模式。

在代码清单 2-18 中，我们使用 `array.map` 遍历一个数字数组，并创建一个箭头函数作为回调。

```javascript
const original = [1,2,3,4];
const multiplied = original.map((item) => item * 10);
console.log(`original array: ${original}`);
console.log(`multiplied array: ${multiplied}`);
```

*代码清单 2-18：使用 `array.map` 处理数组的每个元素*

我们遍历数组元素，把它们逐个传给回调函数。这里我们把每个元素乘以 10，然后 `array.map` 返回一个由相乘后元素组成的新数组。

当我们输出初始数组和返回的数组时，会看到原始数组仍然包含未改变的数字 `1,2,3,4`，只有相乘后的数组包含新的、修改过的元素 `10,20,30,40`。

## 展开数组和对象

现代 JavaScript 的展开运算符（spread operator）写作三个点（...）。它把数组的值或对象的属性展开到各自的变量或常量中。

从技术上讲，展开运算符会把内容复制到分配了自己内存的变量中。在代码清单 2-19 中，我们使用展开运算符把一个对象的多个值赋给几个常量。在几乎所有的 React 代码中，你都会用这种模式来访问组件属性。

```javascript
let object = {fruit: "apple", color: "green"};
let {fruit, color} = {...object};

console.log(`fruit: ${fruit}, color: ${color}`);

color = "red";
console.log(`object.color: ${object.color}, color: ${color}`);
```

*代码清单 2-19：用展开运算符把对象展开到常量中*

我们首先创建一个具有 `fruit` 和 `color` 两个属性的对象，然后用展开运算符把对象展开到变量中，并把它们输出到控制台。这些变量的名称与对象属性的名称相同。现在我们可以直接从变量访问这些值，而不必引用对象本身。我们在模板字符串中就是这么做的，控制台输出为 `fruit: apple, color: green`。

此外，由于这些变量分配了自己的内存，它们是完全的克隆。因此，把变量 `color` 改成 `red` 不会改变原始值：当我们把两者都输出到控制台时，`object.color` 仍然返回 `green`。

用展开运算符克隆数组或对象非常有用，因为 JavaScript 把数组当作对其值的引用来处理。当你把一个数组或对象赋给新的变量或常量时，只是复制了对原始值的引用，并没有通过分配内存来克隆数组或对象。因此，修改副本也会改变原始值。用展开运算符代替等号运算符（=）会分配内存，并且不保留对原始值的引用。因此，它是克隆数组或对象的极佳解决方案，如代码清单 2-20 所示。

```javascript
let originalArray = [1,2,3];
let clonedArray = [...originalArray];

clonedArray[0] = "one";
clonedArray[1] = "two";
clonedArray[2] = "three";

console.log (`originalArray: ${originalArray}, clonedArray: ${clonedArray}`);
```

*代码清单 2-20：用展开运算符克隆数组*

这里我们使用展开运算符，在同一个操作中把原始数组的值复制到克隆数组。然后我们修改克隆数组的元素。最后，我们把两个数组输出到控制台，可以看到原始数组与克隆数组并不相同。

## 练习 2：用现代 JavaScript 扩展 Express.js

现代 JavaScript 提供了编写整洁且高效代码所需的工具。在第二部分中，你将在 Food Finder 应用中使用它。现在，让我们运用新学到的知识，优化你在第 1 章创建的简单 Express.js 服务器。

### 编辑 package.json 文件

我们将用针对不同路由的具名模块来替换服务器中的 `require` 调用。为此，我们需要显式声明项目使用原生模块，否则 Node.js 会抛出错误。请修改你的 `package.json` 文件，使其与代码清单 2-21 一致。

```json
{
    "name": "sample-express",
    "version": "1.0.0",
    "description": "sample express server",
    "license": "ISC",
    "type": "module",
    "dependencies": {
        "express":"^4.18.2",
        "node-fetch": "^3.2.6"
    },
    "devDependencies": {}
}
```

*代码清单 2-21：修改后的 package.json 文件*

添加属性 `type`，取值为 `module`。另外，你还需要安装 `node-fetch` 包，以便在其中一条路由中发起异步 API 调用。运行 `npm install node-fetch` 即可完成安装。

### 编写含异步代码的 ES.Next 模块

在 `sample-express` 文件夹中、`index.js` 文件旁边创建 `routes.js` 文件，并添加代码清单 2-22 中的代码。

```javascript
import fetch from "node-fetch";

const routeHello = () => "Hello World!";

const routeAPINames = async () => {
    const url = "https://www.usemodernfullstack.dev/api/v1/users";
    let data;
    try {
        const response = await fetch(url);
        data = await response.json();
    } catch (err) {
        return err;
    }
    const names = data
        .map((item) => `id: ${item.id}, name: ${item.name}`)
        .join("<br>");
    return names;
};

export {routeHello, routeAPINames};
```

*代码清单 2-22：routes.js 文件中的路由模块*

首先，我们导入用于发起异步请求的 `fetch` 模块。然后我们创建第一条路由，对应已有的 `/hello` 端点。它的行为应与之前相同：使用简洁体语法的胖箭头函数，返回字符串 `Hello World!`。

接下来，我们为新的 `/api/names` 端点创建一条路由。这个端点将为我们的 Web 服务器添加一个页面，显示用户名和 ID 的列表。但首先我们要显式定义一个 `async` 函数，以便在 `fetch` 调用中使用 `await` 语法。然后，我们把 API 端点定义在一个常量中，并定义另一个变量来存储异步数据。我们需要在使用它们之前完成定义，因为 `await` 调用发生在 `try...catch` 块内部，而这些变量是块级作用域的。如果把它们定义在块内部，之后就无法使用了。

我们调用 API 并等待响应数据，调用一成功就把它转换为 JSON。此时 `data` 变量持有一个对象数组。我们用 `array.map` 遍历这些数据，生成想要显示的字符串，然后用换行标签（`<br>`）把所有数组元素拼接起来以分行显示，并返回这个字符串。

最后，我们以各自的名称导出这两条路由。

### 将模块添加到服务器

修改 `sample-express` 文件夹中的 `index.js` 文件，使其与代码清单 2-23 一致。我们使用原生模块来导入此前由 `require` 引入的模块，以及代码清单 2-22 中创建的路由。

```javascript
import {routeHello, routeAPINames} from "./routes.js";
import express from "express";

const server = express();
const port = 3000;

server.get("/hello", function (req, res) {
    const response = routeHello(req, res);
    res.send(response);
});

server.get("/api/names", async function (req, res) {
    let response;
    try {
        response = await routeAPINames(req, res);
    } catch (err) {
        console.log(err);
    }
    res.send(response);
});

server.listen(port, function () {
    console.log("Listening on " + port);
});
```

*代码清单 2-23：使用现代 JavaScript 的基础 Express.js 服务器*

首先，我们用具名导入的语法导入路由。然后，我们用 `import` 语句替换了针对 `express` 包的 `require` 调用。我们之前创建的 `/hello` 端点调用导入的路由，服务器把 `Hello World!` 作为响应发送给浏览器。

最后，我们创建一个包含异步代码的新端点 `/api/names`。因此，我们把处理函数标记为 `async`，并在 `try...catch` 块中 `await` 路由的执行。

在命令行中启动服务器：

```bash
$ node index.js
Listening on 3000
```

现在在浏览器中访问 http://localhost:3000/api/names，如图 2-1 所示。

*图 2-1：浏览器从 Node.js Web 服务器接收到的响应*

你应该会看到新的用户 ID 和名称列表。

## 总结

本章教给你足以创建全栈应用的现代 JavaScript 和 ES.Next 知识。我们介绍了如何使用 JavaScript 模块创建可维护的包以及导入和导出代码、声明变量和常量的不同方式、箭头函数，以及标签和非标签模板字符串。我们用 promise 和 async/await 编写了异步代码，还介绍了 `array.map`、展开运算符，以及它们对你的全栈代码的用处。最后，你运用新学到的知识，用现代 JavaScript 概念更新了第 1 章的示例 Node.js 服务器。

现代 JavaScript 的特性远不止本章所涵盖的这些。在众多免费资源中，我推荐 https://www.javascripttutorial.net 上的 JavaScript 教程。

下一章我们将介绍 TypeScript——一个支持类型的 JavaScript 超集。
