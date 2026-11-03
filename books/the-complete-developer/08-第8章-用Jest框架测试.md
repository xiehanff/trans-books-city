# 第 8 章 用 Jest 框架测试

*Chapter 8: Testing with the Jest Framework*

每当你修改代码时，都有可能给应用程序的其他部分带来无法预见的副作用。因此，保证代码库的完整性与稳定性可能颇具挑战。为此，开发者主要采用两种策略。

第一种是架构模式：我们把代码拆分成小而自包含的 React 组件。这些组件天然互不干扰，因此改动其中一个不应引发任何副作用。第二种是执行自动化单元测试（unit test），本章将借助 Jest 框架来介绍这一做法。

在接下来的几节中，我们将讨论自动化单元测试的要点及其好处。你将学习如何在 Jest 中编写测试套件（test suite），并利用它的报告来改进代码。你还将学习使用代码替身（code double）来处理依赖。最后，你会了解其他一些可能想对应用程序运行的测试类型。

## 测试驱动开发与单元测试

开发者有时会使用测试驱动开发（test-driven development，TDD）技术：在实现真正待测试的代码之前，先编写自动化测试。他们首先创建一个测试，用于评估尽可能小的代码单元能否按预期工作，这样的测试称为单元测试。接着，他们编写通过测试所需的最少量代码。

这种做法有显著的好处。首先，它通过明确定义代码的功能与边界情况，让你专注于应用程序的需求。这样，你对代码的预期行为心中有数，并能尽早发现不清晰或缺失的规范。如果等到功能完成之后才编写测试，测试反映的可能是你实现出来的行为，而不是你所需的行为。

其次，约束自己只编写必要的代码，可以避免函数变得过于复杂，并把应用程序拆分成易于理解的小块。可测试的代码才是可维护的代码。此外，这种技术还能确保测试覆盖应用代码的很大一部分，这一指标称为代码覆盖率（coverage）；而且在开发期间频繁运行测试，你就能立刻发现新引入的代码行带来的缺陷。

视具体情况而定，单元测试所针对的单元可以是一个模块、一个函数或一行代码。这些测试的目标是验证每个单元都能独立运行。每个测试函数内部的单独语句是测试步骤，而整个测试函数称为测试用例（test case）。测试套件把测试用例聚合为逻辑块。测试要被认为是可复现的，就必须在每次运行时都返回相同的结果。正如本章将要探讨的，这意味着我们必须在一个带有确定数据集的受控环境中运行测试。

Facebook 在开发 React 的同时开发了 Jest 测试框架，但我们也可以把它用于任何 Node.js 项目。它为搭建和编写测试定义了一套语法。它的测试运行器（test runner）会执行这些测试、自动替换代码中的各种依赖，并生成测试覆盖率报告。额外的 npm 模块提供了用于测试 DOM 或 React 组件的自定义代码，当然还有为 TypeScript 添加类型的支持。

## 使用 Jest

要在项目中使用 Jest，我们必须安装所需的包、为所有测试文件创建一个目录，并添加一条用于运行测试的 npm 脚本。在 Next.js 应用程序的根目录下执行以下命令，把该框架以及来自 DefinitelyTyped 的类型定义安装为开发依赖：

```bash
$ npm install --save-dev jest @types/jest
```

然后创建用于保存测试的目录。Jest 默认使用 __tests__ 文件夹，因此请在根目录下新建一个。接着，为了把 npm 脚本 test 添加到项目中，打开 package.json 文件，将 scripts 对象修改成与 代码清单 8-1 中一致的内容。

```javascript
  "scripts": {
      "dev": "next dev",
      "build": "next build",
      "start": "next start",
      "lint": "next lint",
      "test": "jest"
  },
```

*代码清单 8-1：包含新增 test 命令的 package.json 文件*

现在我们就可以用 npm test 命令运行测试了。通常，构建服务器会在构建过程中默认执行这条命令。最后，为了在 Jest 中启用 TypeScript 支持，添加 ts-jest 转译器：

```bash
$ npm install --save-dev ts-jest
```

同时，运行 npx ts-jest config:init 创建一个 jest.config 文件以加入 TypeScript 支持。

## 创建一个用于测试的示例模块

让我们编写一些示例代码，来帮助理解单元测试和 TDD。假设我们想在应用中创建一个新模块 ./helpers/sum.ts，它应当导出一个函数 sum，返回各参数之和。为了遵循 TDD 模式，我们先为这个模块创建测试用例。

首先我们需要创建用于运行测试的文件。在默认的测试目录中创建一个名为 sum.test.ts 的文件，并加入 代码清单 8-2 中的代码。

```typescript
import {sum} from "../helpers/sum";

describe("the sum function", () => {

});
```

*代码清单 8-2：空的测试套件*

我们导入了稍后才编写的 sum 函数，并用 Jest 的 describe 函数创建一个空的测试套件。一旦用 npm test 运行这些（尚不存在的）测试，Jest 就会报错说 helpers 目录下没有名为 sum.ts 的文件。现在请在项目根目录下创建这个文件和文件夹，并在文件中编写如 代码清单 8-3 所示的 sum 函数。

```typescript
const sum = () => {};
export {sum};
```

*代码清单 8-3：sum 函数的最初骨架*

再用 npm test 运行一次测试。由于这段代码只是导出了一个什么都不返回的占位 sum 函数，Jest 测试运行器会再次报错。这一次，它提示我们测试套件至少需要包含一个测试。

下面我们来剖析测试用例的构成，并在此过程中向 sum.test.ts 文件添加几个测试用例。

## 测试用例的构成

单元测试分为两类：基于状态的测试和基于交互的测试。基于交互（interaction-based）的测试用例验证被评估代码是否调用了某个特定函数；基于状态（state-based）的测试用例则检查代码的返回值或最终状态。两类测试都遵循同样的三个步骤：准备（arrange）、执行（act）、断言（assert）。

### 准备（Arrange）

要编写独立且可复现的测试，我们首先需要通过定义测试数据等前提条件来准备环境。如果这些前提条件只服务于某一个特定的测试用例，我们就在该用例的开头定义它们；否则，我们使用 beforeEach 钩子（在每个测试用例之前执行）或 beforeAll 钩子（在所有测试运行之前执行），为测试套件中的所有测试全局设置它们。

例如，如果出于某种原因，每个测试用例都要使用同一个全局数据集，而且我们知道测试步骤会修改这个数据集，那么就需要在每次测试之前重新创建该数据集。beforeEach 钩子正是做这件事的绝佳位置。反之，如果测试用例只是读取这些数据，那么数据集只需定义一次，因此应使用 beforeAll 钩子。

让我们定义两个测试用例，并为每个用例创建输入值。由于输入参数是每个测试用例特有的，我们将直接在测试用例内部声明它们，而不使用 beforeEach 或 beforeAll 钩子。用 代码清单 8-4 中的代码更新 sum.test.ts 文件。

```typescript
import {sum} from "../helpers/sum";

describe("the sum function", () => {
    test("two plus two is four", () => {
        let first = 2;
        let second = 2;
        let expectation = 4;
    });

    test("minus eight plus four is minus four", () => {
        let first = -8;
        let second = 4;
        let expectation = -4;
    });
});
```

*代码清单 8-4：包含准备步骤的测试套件*

describe 函数创建了我们的测试套件，其中包含对 test 函数的两次调用，每次调用都是一个测试用例。两者的第一个参数都是我们在测试运行器报告中看到的描述文字。

我们的每个测试都在评估 sum 函数的结果。第一个检查加法功能，验证 2 加 2 返回 4；第二个确认该函数也能正确返回负值，它把 −8 与 4 相加，期望结果为 −4。

你可能还想检查 sum 函数的返回类型。通常我们会这么做，但由于使用了 TypeScript，就没有必要添加这个额外的测试用例了。我们可以在函数签名中定义返回类型，由 TSC 替我们验证。

### 执行（Act）

测试运行器一执行某个用例，测试步骤就代表我们采取行动：用该测试用例特有的数据调用被测代码。每个测试用例应当恰好测试系统的一个功能或一种变体。这一步骤就是那行调用待执行函数的代码。代码清单 8-5 将它添加到 sum.test.ts 的测试用例中。

```typescript
import {sum} from "../helpers/sum";

describe("the sum function", () => {

    test("two plus two is four", () => {
        let first = 2;
        let second = 2;
        let expectation = 4;
        let result = sum(first, second);
    });

    test("minus eight plus four is minus four", () => {
        let first = -8;
        let second = 4;
        let expectation = -4;
        let result = sum(first, second);
    });

});
```

*代码清单 8-5：包含执行步骤的测试套件*

新增的代码行调用 sum 函数，并把之前定义的值作为参数传入，返回值存储在 result 变量中。在你的编辑器里，TSC 应该会抛出类似 Expected 0 arguments, but got 2 的错误。这没有问题，因为 sum 函数目前只是一个空占位符，尚不接受任何参数。

### 断言（Assert）

测试用例的最后一步是断言（assertion）：确认代码符合我们定义的预期。断言由两部分组成：Jest 的 expect 函数，以及与之配合使用、来自 Jest 断言（assert）库的匹配器（matcher）函数——后者定义了我们正在测试的条件。根据单元测试的类别，这个条件可以是特定的返回值、一次状态变更，或对另一个函数的调用。常见的匹配器检查某个值是否为数字、字符串等等；我们也可以用它们断言某个函数返回 true 或 false。

Jest 的断言库内置了一套基本匹配器，我们还可以从 npm 仓库添加更多。最常见的断言包之一是 testing-library/dom，用于在 DOM 中查询特定节点并断言其特性，例如检查类名或属性，或处理原生 DOM 事件。另一个常见的断言包 testing-library/react 为 React 增加了实用工具，让我们能在断言中使用 render 函数和 React hooks。

由于每个测试用例只评估一个代码单元中的一个条件，我们把每个测试限制为一条断言。这样，当测试运行成功或失败、测试报告器生成报告时，我们就能轻松找出是哪条测试假设失败了。代码清单 8-6 为每个测试用例添加了一条断言，请将其粘贴到 sum.test.ts 文件中。

```typescript
import {sum} from "../helpers/sum";

describe("the sum function", () => {

    test("two plus two is four", () => {
        let first = 2;
        let second = 2;
        let expectation = 4;
        let result = sum(first, second);
        expect(result).toBe(expectation);
    });

    test("minus eight plus four is minus four", () => {
        let first = -8;
        let second = 4;
        let expectation = -4;
        let result = sum(first, second);
        expect(result).toBe(expectation);
    });

});
```

*代码清单 8-6：包含断言步骤的测试套件*

这些代码行使用 expect 断言函数配合 toBe 匹配器，比较预期结果是否与我们设定的期望值相同。至此测试用例已经完整：每个用例都遵循准备、执行、断言模式，并验证一个条件。附录 C 列出了更多匹配器。

## 运用 TDD

我们的测试用例还没有真正执行过；如果运行 npm test，测试运行器会立即失败。TSC 会检查代码，并针对 sum 函数缺失的参数声明抛出错误：

```
FAIL  __tests__/sum.test.ts
  • Test suite failed to run
--snip--
Test Suites: 2 failed, 2 total
Tests:       0 total
Snapshots:   0 total
```

现在到了实现 sum 函数的时候。遵循 TDD 原则，我们将以增量方式为代码添加功能，并在每次添加后运行测试套件，如此往复，直到所有测试通过。首先补上缺失的参数。用 代码清单 8-7 的内容替换 sum.ts 中的代码。

```typescript
const sum = (a: number, b: number) => {};

export {sum};
```

*代码清单 8-7：添加了参数的 sum 函数*

我们已经添加了参数并把它们声明为 number 类型。现在重新运行测试用例，不出所料，它们失败了。控制台输出告诉我们 sum 函数没有返回预期的结果。这不足为奇，因为我们的 sum 函数根本不返回任何东西：

```
FAIL  __tests__/sum.test.ts (5.151 s)
  the sum function
    × two plus two is four (6 ms)
    × minus eight plus four is minus four (1 ms)

  • the sum function › two plus two is four
    Expected: 4
    Received: undefined

  • the sum function › minus eight plus four is minus four
    Expected: -4
    Received: undefined

Test Suites: 1 failed, 1 total
Tests:       2 failed, 2 total
Snapshots:   0 total
Time:        5.328 s, estimated 11 s
```

代码清单 8-8 中的代码为 sum.ts 文件添加了这一功能。我们把函数的返回类型声明为 number，并加上了两个参数。

```typescript
const sum = (a: number, b: number): number => a + b;

export {sum};
```

*代码清单 8-8：完整的 sum 函数*

如果重新运行 npm test，Jest 应当报告所有测试用例都成功：

```
PASS  __tests__/sum.test.ts (8.045 s)
  the sum function
    ✓ two plus two is four (2 ms)
    ✓ minus eight plus four is minus four (2 ms)

Test Suites: 1 passed, 1 total
Tests:       2 passed, 2 total
Snapshots:   0 total
Time:        8.291 s
```

可以看到，一切正常。

### 重构代码

在需要重构代码时，单元测试尤其有用。举个例子，我们来改写 sum 函数，让它不再接收两个参数，而是接收一个数字数组，函数应返回数组所有项的总和。

我们先把现有的测试用例改写成更紧凑的形式，然后扩展测试套件来验证新行为。用 代码清单 8-9 替换 sum.test.ts 文件中的代码。

```typescript
import {sum} from "../helpers/sum";

describe("the sum function", () => {

    test("two plus two is four", () => {
        expect(sum([2, 2])).toBe(4);
    });

    test("minus eight plus four is minus four", () => {
        expect(sum([-8, 4])).toBe(-4);
    });

    test("two plus two plus minus four is zero", () => {
        expect(sum([2, 2, -4])).toBe(0);
    });

});
```

*代码清单 8-9：重构后的 sum 函数的测试套件*

注意，我们把测试用例改写成了更紧凑的形式。把准备、执行、断言语句明确地拆分到多行可能更易读，但对于 代码清单 8-9 这样简单的测试用例，我们常常写成一行。我们还修改了它们的功能以适应新需求：sum 函数不再接收两个值，而是接收一个数字数组。同样，TSC 会立即提醒我们，测试套件中调用的 sum 函数与实际实现的参数不匹配。

测试写好之后，就可以改写代码了。代码清单 8-10 展示了 helpers/sum.ts 文件的代码：sum 函数现在接收一个数字数组作为参数，并返回一个数字。

```typescript
const sum = (data: number[]): number => {
    return data[0] + data[1];
};

export {sum};
```

*代码清单 8-10：helpers/sum.ts 文件中改写后的 sum 函数*

我们把参数改成了数字数组，这修复了 代码清单 8-9 中的测试套件引发的 TypeScript 错误。但由于我们在遵循 TDD、每次只做一处功能改动，所以保留了函数原有的两数相加行为。不出所料，用 npm test 运行自动化测试时，有一个测试用例失败了：

```
FAIL  __tests__/sum.test.ts (7.804 s)
  the sum function
    ✓ two plus two is four (7 ms)
    ✓ minus eight plus four is minus four (1 ms)
    ✕ two plus two plus minus four is zero (9 ms)

  • the sum function › two plus two plus minus four is zero
    Expected: 0
    Received: 4

Test Suites: 1 failed, 1 total
Tests:       1 failed, 2 passed, 3 total
Snapshots:   0 total
Time:        8.057 s, estimated 9 s
```

失败的正是测试新需求的第三个测试用例。我们不仅预料到了这个结果，而且希望它失败——这样我们才知道测试本身是有效的。如果相应的功能尚未实现测试就通过了，那说明测试用例本身有问题。

以这个失败的测试为基线，现在可以着手重构代码来满足新需求了。把 代码清单 8-11 中的代码粘贴到 sum.ts 文件里，我们将 sum 函数重构为返回数组中所有值之和。

```typescript
const sum = (data: number[]): number => {
    return data.reduce((a, b) => a + b);
};

export {sum};
```

*代码清单 8-11：使用 array.reduce 修正后的 sum 函数*

虽然可以用 for 循环遍历数组，但我们使用现代 JavaScript 的 array.reduce 函数。这个原生数组函数会对每个数组元素运行一个回调函数；回调接收上一次迭代的返回值和当前数组项作为参数——这正是计算总和所需要的。

运行测试套件中的所有测试用例，验证它们都按预期工作：

```
PASS  __tests__/sum.test.ts (7.422 s)
  the sum function
    ✓ two plus two is four (2 ms)
    ✓ minus eight plus four is minus four
    ✓ two plus two plus minus four is zero

Test Suites: 1 passed, 1 total
Tests:       3 passed, 3 total
Snapshots:   0 total
Time:        7.613 s
```

测试运行器应当显示代码通过了所有测试。

### 评估测试覆盖率

为了准确衡量测试套件覆盖了哪些代码行，Jest 会生成测试覆盖率报告。测试评估的代码比例越高，测试就越彻底，我们对应用程序的质量和可维护性也越有信心。根据一般经验，代码覆盖率应达到 90% 或以上，并且代码中最关键的部分要有很高的覆盖率。当然，测试用例应当通过测试代码的功能来体现价值；单纯为了提高测试覆盖率而堆砌测试并不是我们追求的目标。但只要对代码库进行了充分的测试，你就可以放心地重构现有功能、实现新功能，而不必担心引入回归缺陷。高代码覆盖率能证实改动没有隐藏的副作用。

修改 package.json 文件中的 npm test 脚本，为其添加 --coverage 标志，如 代码清单 8-12 所示。

```javascript
  "scripts": {
      "dev": "next dev",
      "build": "next build",
      "start": "next start",
      "lint": "next lint",
      "test": "jest --coverage"
  },
```

*代码清单 8-12：在 package.json 文件中启用 Jest 的测试覆盖率功能*

如果重新运行测试套件，Jest 应当显示单元测试覆盖的代码百分比。它会生成一份代码覆盖率报告并保存在 coverage 文件夹中。把你的输出与下面的内容对比一下：

```
PASS  __tests__/sum.test.ts (7.324 s)
  the sum function
    ✓ two plus two is four (2 ms)
    ✓ minus eight plus four is minus four
    ✓ two plus two plus minus four is zero (1 ms)
----------|---------|----------|---------|---------|-------------------
File      | % Stmts | % Branch | % Funcs | % Lines | Uncovered Line #s
----------|---------|----------|---------|---------|-------------------
All files |     100 |      100 |     100 |     100 |
  sum.ts  |     100 |      100 |     100 |     100 |
----------|---------|----------|---------|---------|-------------------
Test Suites: 1 passed, 1 total
Tests:       3 passed, 3 total
Snapshots:   0 total
Time:        7.687 s, estimated 8 s
```

报告按语句、分支、函数和行分别列出覆盖率。可以看到，我们这个简单的 sum 函数在所有类别上都达到了 100% 的代码覆盖率。因此，我们知道没有遗漏任何未测试的代码，可以相信这些测试用例如实反映了函数的质量。

## 用假实现、桩和模拟替换依赖

前面提到，测试应当隔离运行，不依赖外部代码。你可能想过该如何处理导入的模块：毕竟，一旦导入了代码，就等于给被评估的单元增加了一个依赖。这些第三方模块可能无法按预期工作，而我们不希望自己的代码建立在它们都能正常运作的假设之上。因此，你应当为每个导入的模块提供一组测试用例来验证其功能——它们同样是被测试的单元。

另外，我们不应把模块直接导入其他代码单元，而是要用测试替身（test double）替换它们，这些替身返回为测试量身定制的一组静态数据。测试替身替代某个对象或函数，从而切实移除一处依赖。由于它们返回的是确定的数据集，其响应是已知且可预测的。你可以把它们比作电影中的特技替身演员。

除了替代对象或函数之外，测试替身还有第二个重要用途：它们会记录自己的调用，让我们得以侦听（spy）。因此我们可以用它们来测试替身是否被调用过、被调用了多少次、收到了哪些参数。测试替身主要有三种类型：假实现（fake）、桩（stub）和模拟（mock）。不过，你有时也会听到用 mock 这个词统称这三者。

### 创建一个带依赖的模块

为了练习在 sum 函数中使用测试替身，我们将创建一个新函数，计算斐波那契数列中指定数量的值。斐波那契数列的特点是每个后续数字都是前两个数字之和，这是 sum 模块的一个简单用例。

每个开发者都必须弄清楚测试用例需要细到什么程度。斐波那契数列就是一个很好的例子：试图测试传入函数的每一个可能的数字是徒劳的，因为这个数列没有尽头。我们要做的是验证函数能否正确处理边界情况、底层功能是否正常。例如，我们会检查它如何处理长度为 0 的输入——此时函数应返回一个空字符串；然后测试它如何计算任意长度大于 3 的斐波那契数列。请在 __tests__ 文件夹中创建 fibonacci.test.ts 测试套件，并加入 代码清单 8-13 中的代码。

```typescript
import {fibonacci} from "../helpers/fibonacci";

describe("the fibonacci sequence", () => {

    test("with a length of 0 is ", () => {
        expect(fibonacci(0)).toBe(" ");
    });

    test("with a length of 5 is '0, 1, 1, 2, 3' ", () => {
        expect(fibonacci(5)).toBe("0, 1, 1, 2, 3");
    });

});
```

*代码清单 8-13：fibonacci 函数的测试套件*

我们定义了两个测试用例：一个检查长度输入为 0 的情况，另一个计算包含五个数字的斐波那契数列。两个测试都遵循前面用过的紧凑版准备、执行、断言模式。

测试用例创建好之后，就可以开始编写斐波那契函数的代码了。在 helpers 文件夹中、sum.ts 文件旁边创建 fibonacci.ts 文件，并加入 代码清单 8-14 中的代码。

```typescript
import {sum} from "./sum";

const fibonacci = (length: number): string => {
    const sequence: number[] = [];
    for (let i = 0; i < length; i++) {
        if (i < 2) {
            sequence.push(sum([0, i]));
        } else {
            sequence.push(sum([sequence[i - 1], sequence[i - 2]]));
        }
    }
    return sequence.join(", ");
};

export {fibonacci};
```

*代码清单 8-14：fibonacci 函数*

我们从本章前面创建的模块导入 sum 函数。它现在是一个依赖，稍后需要用测试替身替换。接着我们实现 fibonacci 函数：它接收要计算的数列长度，返回一个字符串。我们把当前数列保存在一个数组里，这样就有一条简单的途径来访问计算下一个值所需的两个先前值。注意，数列的第一个数字恒为 0，第二个恒为 1。最后，我们返回一个包含所请求数量数值的字符串。保存这段代码并重新运行测试套件，sum.test.js 和 fibonacci.test.ts 应该都能成功通过。

### 创建替身文件夹

由于斐波那契模块导入了 sum 函数，我们的代码有了一个外部依赖。这对测试来说是个问题：如果 sum 函数坏了，即使斐波那契实现的逻辑是正确的，斐波那契数列的测试也会失败。

为了让测试与依赖解耦，我们将用一个测试替身替换 fibonacci.ts 文件中的 sum 函数。在测试运行期间，只要测试文件旁边有一个 __mocks__ 子目录，其中存放着同名文件，Jest 就能替换对应的模块。请在 helpers 文件夹中、测试文件旁边创建这样一个文件夹，并在其中放一个 sum.ts 文件。暂时让该文件保持为空。

要启用测试替身，我们调用 jest.mock 函数，把保存原始模块的路径传给它。在 代码清单 8-15 中，我们把这次调用添加到 fibonacci.test.ts 里。

```typescript
import {fibonacci} from "../helpers/fibonacci";

jest.mock("../helpers/sum");

describe("the fibonacci sequence", () => {
    test("with a length of 0 is ", () => {
        expect(fibonacci(0)).toBe(" ");
    });
    test("with a length of 5 is '0, 1, 1, 2, 3' ", () => {
        expect(fibonacci(5)).toBe("0, 1, 1, 2, 3");
    });

});
```

*代码清单 8-15：带测试替身的 fibonacci 函数测试套件*

这行新代码用测试替身替换了 sum 模块。下面我们来创建全部三种基本类型的测试替身，把它们的代码依次添加到 __mocks__ 文件夹中的文件里。

### 使用桩（stub）

桩（stub）只是返回某些预定义数据的对象。这让它们实现起来非常简单，但用途有限：仅仅返回相同的数据往往不足以模仿依赖原本的行为。代码清单 8-16 展示了 sum 函数测试替身的一个桩实现。请把代码粘贴到 __mocks__ 文件夹内的 sum.ts 文件中。

```typescript
const sum = (data: number[]): number => 999;

export {sum};
```

*代码清单 8-16：sum 函数的桩*

打桩后的函数与原函数具有相同的签名：它接收相同的参数（一个数字数组），并返回一个字符串。然而与原函数不同，这个测试替身无论收到什么数据，总是返回同一个数字 999。

要让测试套件用这个桩函数成功运行，我们需要调整对代码行为的预期。它返回的不再是斐波那契数列中的五个数字，而是字符串 999, 999, 999, 999, 999。如果看到这样的字符串，我们就知道 sum 函数被调用了五次。请用这个桩做些实验，修改测试套件中的期望使其与之匹配，然后再把匹配器恢复成 代码清单 8-15 中所示的样子，供接下来的测试使用。

### 使用假实现（fake）

假实现（fake）是最复杂的一类测试替身。它们是原有功能的可用实现，但与真实实现不同，它们只提供单元测试所需的功能。其实现是简化版的，通常不照顾边界情况。

sum 的假实现手动把数组的第一项和第二项相加，而不使用 array.reduce。这一简化实现使 sum 函数失去了对两个以上数据点求和的能力，但对斐波那契数列来说已经足够。复杂度的降低使它易于理解且更不容易出错。用 代码清单 8-17 中的代码替换 __mocks__ 文件夹内 sum.ts 文件的内容。

```typescript
const sum = (data: number[]): number => {
    return data[0] + data[1];
}
export {sum};
```

*代码清单 8-17：sum 函数的假实现*

我们的假实现使用简单的数学加号运算符（+）把 data 参数的第一项与第二项相加。它的主要好处是返回的结果与真实实现相近。现在我们可以运行测试套件，它们应当无需调整期望就能成功通过，并返回斐波那契数列。

### 使用模拟（mock）

模拟（mock）介于桩和假实现之间：虽然不如假实现精细，但比桩返回的数据更接近真实。它们虽然不模拟依赖的真实行为，却能对接收到的数据作出反应。

例如，我们为 sum 函数编写一个朴素的 mock 实现，它将从硬编码的哈希映射中返回结果。用 代码清单 8-18 中的代码替换 __mocks__/sum.ts 文件中的代码，它会检查请求，使斐波那契计算器能够配合原始的测试套件工作。

```typescript
type resultMap = {
    [key: string]: number;
}

const results : resultMap= {
    "0 + 0": 0,
    "0 + 1": 1,
    "1 + 0": 1,
    "1 + 1": 2,
    "2 + 1": 3
};

const sum = (data: number[]): number => {
    return results[data.join("+")];
}

export {sum};
```

*代码清单 8-18：sum 函数的模拟*

我们创建了一个名为 resultMap 的类型，以字符串为键、数字为值；然后用这个新类型定义一个哈希映射，存放我们想要的各种响应。接着，我们定义与原实现具有相同接口的 mock 函数。在 mock 内部，我们根据收到的参数计算出哈希映射中要使用的键，从而返回正确的数据集，生成真实的斐波那契数列。用这个 mock 替代 sum 的主要好处在于，我们可以控制其结果，因为它从一个已知的数据集中返回值。

方便的是，Jest 为我们提供了处理测试替身的辅助工具。jest.mock 函数用 mock 替换导入的模块；jest.fn API 创建一个基本的 mock，可返回任何预定义的数据；jest.spyOn 则让我们在不修改函数的情况下记录对它的调用。我们将在第 146 页的练习 8 中用到所有这些工具。

在日常开发场景中，你不必纠结桩、假实现和 mock 之间的细微差别，通常会把 mock 用作测试替身的统称。不要花费过多时间过度设计你的 mock——它们只是帮助你测试代码的工具。

## 其他类型的测试

到目前为止本章介绍的测试，是全栈开发者最常遇到的类型。本节简要说明其他类型的测试及其适用时机。它们并非要取代单元测试，而是对单元测试的补充，覆盖实现中那些用其他方式无法测试的特定方面。例如，单元测试因为隔离运行，无法评估模块之间的交互。理论上，如果每个函数和模块都通过了各自的测试，整个程序就应当按预期工作；但实际上，你经常会遇到由错误的模块文档引起的问题——文档声称某个 API 返回特定类型，而实际实现返回的却是另一种类型。

### 功能测试

单元测试从开发者的视角审视功能的实现，而功能测试（functional test）站在用户的视角，验证代码是否按用户期望的方式工作。换句话说，这类测试检查给定输入是否产生预期输出。大多数功能测试属于黑盒测试（black-box test）：忽略模块的内部代码、副作用和中间结果，只测试接口。功能测试不生成代码覆盖率报告。一般来说，质量保障（QA）负责人会在系统测试阶段编写并使用功能测试；相比之下，单元测试由开发者在开发过程中编写和使用。

### 集成测试

你已经了解到，单元测试的目标是在隔离状态下检查尽可能小的代码片段。集成测试（integration test）则恰恰相反：它验证整个子系统的行为，无论子系统是代码层（例如应用的数据存储机制），还是由多个模块组成的特定功能。集成测试在当前环境的上下文中检查子系统的集成情况，因此它们从不在隔离状态下运行，而且通常不使用测试替身。

集成测试有助于发现三类问题。第一类是与模块间通信相关的问题，即模块之间的通信。常见问题包括内部 API 集成有缺陷，以及未察觉的副作用，例如某个函数在向文件系统写入新数据之前没有删除旧文件。第二类是与环境相关的问题。环境指代码运行所在的软硬件配置；不同的软件版本或硬件配置可能给你的代码带来重大问题。对全栈开发者而言，最常见的问题涉及 Node.js 版本的差异和模块中过时的依赖。

第三类是与网关通信相关的问题，涉及与第三方 API 网关之间的任何 API 通信。与外部 API 的所有通信都应当用集成测试来检验。这是集成测试唯一可能使用测试替身的情形，例如使用外部 API 的桩版本，来模拟特定的 API 行为，比如超时或请求成功。与功能测试一样，集成测试通常也由质量保障负责人编写和使用，开发者较少为之。

### 端到端测试

你可以把端到端测试（end-to-end test）看作功能测试与集成测试的结合。作为另一种黑盒测试，它在特定环境中横跨从前端到后端的整个技术栈来检验应用的功能。这类面向业务的测试应当让人确信，整个应用程序仍然按预期工作。

端到端测试在特定环境中运行应用程序。依赖众多带来的复杂性常常会增加测试不稳定（flaky test）的风险：应用程序本身工作正常，但环境导致测试失败。因此，端到端测试是创建和维护最耗时的测试。由于其复杂性，我们必须精心设计。在执行时，它们以运行缓慢、容易超时著称，而且与几乎所有黑盒测试一样，无法提供详细的错误报告。所以，它们只测试最关键的面向业务的场景。通常由质量保障负责人编写这类测试。

### 快照测试

本章前面介绍的测试都是对照某种断言来检查代码。快照测试（snapshot test）则不同，它把应用程序当前的视觉（即用户界面）状态与之前的版本进行比较，因此也被称为视觉回归测试（visual regression test）。在每次测试中，我们创建新的快照，然后与先前存储的快照比较，这为测试用户界面组件和完整页面提供了一种廉价的方式。我们不必手动创建并维护描述界面每个属性（如组件的高度、宽度、位置和颜色）的测试，只需建立一个包含所有这些属性的快照即可。

执行这类测试的一种方式是创建并比较截图。通常由无头浏览器（headless browser）渲染组件，测试运行器等待页面渲染完成后再截取图像。遗憾的是，这一过程相对缓慢，而且无头浏览器并不稳定。Jest 为快照测试采取了另一种思路：它不使用无头浏览器和图像文件，而是把 React 用户界面组件渲染到虚拟 DOM，将其序列化，并以纯文本形式保存在 __snapshots__ 目录下的 snap 文件中。因此，Jest 的快照测试性能好得多，缺陷也更少。你将在第二部分构建的 Food Finder 应用就会使用快照测试来验证构建的完整性，并测试你的 React 组件。

## 练习 8：为天气应用添加测试用例

只要遵循我们讨论过的基本原则，测试代码就没有对错之分。单元测试、快照测试和端到端测试都是你工具箱中的不同工具，你必须在使用它们的价值与编写测试所花的时间之间取得平衡。对于该测试什么，也没有共识可言。虽然你应当力争 90% 或更高的代码覆盖率，但一般的经验法则是：至少用单元测试覆盖应用程序最关键的部分，然后再编写一些集成测试，验证应用程序在每次部署时都能正常工作。

对于我们的天气应用，我们希望测试用例覆盖四个核心方面。首先，我们将添加单元测试来评估中间件和服务。尽管 REST API 端点和 React 用户界面组件很容易直接在浏览器中测试，我们仍会为两者分别添加测试用例：为用户界面组件添加一个基本的快照测试，为 REST API 端点 /v1/weather/[zipcode].ts 添加一个端到端测试。

出于简单考虑，我们选择测试 REST 端点而非 GraphQL API，因为每个 REST 端点都有自己独立的文件，而所有 GraphQL API 共享一个入口，测试起来更为复杂。不过，在本章学完之后，测试这个 GraphQL API 将是探索端到端测试的绝佳练习。

### 用侦听器测试中间件

连接数据库的中间件是应用程序的核心部分，但我们无法直接访问它，因为它不暴露任何 API。我们只能通过检查数据库，或者通过 Mongoose、某个服务或 API 端点运行查询，来间接测试它。这些方法都可行，但如果我们想以单元测试的方式测试数据库连接，就需要尽可能隔离该组件。

为此，我们将使用 Jest 内置的侦听器（spy）来验证中间件是否成功调用了建立 MongoDB 内存服务器连接所需的所有函数。进入 __tests__ 文件夹，在其中新建一个 middleware 文件夹和一个 db-connect.test.ts 文件，然后把 代码清单 8-19 中的代码复制进去。

```typescript
/**
 * @jest-environment node
 */

import dbConnect from "../../middleware/db-connect";
import mongoose from "mongoose";
import {MongoMemoryServer} from "mongodb-memory-server";

describe("dbConnect ", () => {
    
    let connection: any;

    afterEach(async () => {
        jest.clearAllMocks();
        await connection.stop();
        await mongoose.disconnect();
    });

    afterAll(async () => {
        jest.restoreAllMocks();
    });

    test("calls MongoMemoryServer.create()", async () => {
        const spy = jest.spyOn(MongoMemoryServer, "create");
        connection = await dbConnect();
        expect(spy).toHaveBeenCalled();
    });

    test("calls mongoose.disconnect()", async () => {
        const spy = jest.spyOn(mongoose, "disconnect");
        connection = await dbConnect();
        expect(spy).toHaveBeenCalled();
    });

    test("calls mongoose.connect()", async () => {
        const spy = jest.spyOn(mongoose, "connect");
        connection = await dbConnect();
        const MONGO_URI = connection.getUri();
        expect(spy).toHaveBeenCalledWith(MONGO_URI, {dbName: "Weather"});
    });

});
```

*代码清单 8-19：用于数据库连接的 __tests__/middleware/db-connect.test.ts 测试套件*

这段代码的大部分与你本章前面编写的测试套件类似，但这次我们测试的不是简化的示例代码，而是真实的代码，这就要求我们做一些调整。

首先把 Jest 的测试环境设置为 node，它模拟一个 Node.js 运行时。稍后编写快照测试时，我们将使用 Jest 的默认环境 jsdom，它通过提供 window 对象以及所有常见的 DOM 属性和函数来模拟浏览器。始终在文件中显式设置这些环境，可以避免因用错环境而导致的问题。然后，像往常一样导入所需的包。

现在可以开始为 dbConnect 函数编写测试套件了。我们在测试套件的作用域中定义一个 connection 变量来保存数据库连接，这样就能访问 MongoDB 服务器实例及其方法和属性。例如，我们将用它们在每次测试之后停止连接并与服务器断开，以保证每个测试用例相互独立。

为了能保存连接，我们首先需要让 db-connect.ts 文件中的 dbConnect 函数返回 mongoServer 常量。打开该文件，在 dbConnect 函数的右花括号（}）之前加入 return mongoServer 这一行。有时你需要修改先前编写的代码，以满足测试的要求。换句话说，你需要调整代码，使之可被测试。

现在我们利用刚才暴露出来的连接，并设置 afterEach 钩子（在每个测试用例之后运行），把 mock 函数重置为其初始 mock 状态，从而清除之前收集的所有数据。这一步是必要的，否则侦听器会报告此前调用中获得的信息，因为它们的状态会在所有测试套件之间保留。此外，我们为每个测试用例重新创建数据库连接，因此每次测试之后需要停止当前连接并显式地从数据库断开。然后我们设置 afterAll 钩子，通过 restoreAllMocks 函数移除所有 mock 并还原原始函数。

我们的测试用例都应当遵循准备、执行、断言模式。在逐一讲解时，你不妨打开 middleware 文件夹中的 db-connect.ts 文件对照阅读。第一个测试用例验证对 MongoMemoryServer 上 create 函数的调用，因为它是我们在 db-connect.ts 文件中调用的第一个函数。为此，我们用 jest.spyOn 方法创建一个侦听器。该方法接收的参数是对象名称和要侦听的对象方法。接着我们对被测代码执行操作，调用 dbConnect 函数；最后断言该侦听器已被调用。

第二个测试用例的原理类似，只是侦听的是另一个方法。我们用它检查在 dbConnect 执行期间 mongoose.disconnect 是否被成功调用。第三个测试用例引入了一个新的匹配器：我们不再只用 toHaveBeenCalled 验证调用本身，还用 toHaveBeenCalledWith 验证调用的参数。这里我们直接从连接中取出连接字符串，存入 MONGO_URI 变量，同时硬编码要连接的数据库。然后调用该匹配器，传入期望的参数，验证它们符合我们的预期。

现在用 npm test 运行测试套件。所有测试都应当通过，测试覆盖率达到 100%。

### 创建 mock 来测试服务

我们为中间件编写的测试相当简单，而服务测试要复杂一些。如果打开 mongoose/weather/services.ts 文件，你会看到这些服务依赖于 WeatherModel，它是 Mongoose 通向 MongoDB 集合的网关。每个服务都会调用模型上的某个方法，而该方法又需要数据库连接。我们不会在这里重新评估这些数据库连接；这个测试套件的目标是验证服务函数调用了正确的 WeatherModel 函数。为此，我们将创建一个 mock 版的 WeatherModel，它以 mock 函数的形式暴露同一组 API。

我们先编写被 mock 的模型。按照惯例，创建 mongoose/weather/__mocks__/model.ts 文件，并加入 代码清单 8-20 中的代码。

```typescript
import {WeatherInterface} from "../interface";

type param = {
    [key: string]: string;
};

const WeatherModel = {
    create: jest.fn((newData: WeatherInterface) => Promise.resolve(true)),
    findOne: jest.fn(({zip: paramZip}: param) => Promise.resolve(true)),
    updateOne: jest.fn(({zip: paramZip}: param, newData: WeatherInterface) =>
        Promise.resolve(true)
    ),
    deleteOne: jest.fn(({zip: paramZip}: param) => Promise.resolve(true))
};
export default WeatherModel;
```

*代码清单 8-20：WeatherModel 的 mock*

我们实现了 WeatherInterface，并定义了新的 param 类型——一个由键值对构成的对象，用于为第一个参数标注类型。我们把被 mock 的 WeatherModel 设为默认导出，它是一个对象，实现了真实 WeatherModel 的四个方法，每个方法都与原始方法接收相同的参数，并与原 Mongoose 模型的方法一一对应。由于它们是异步函数，我们返回一个 resolve 为 true 的 Promise。

现在可以编写服务的测试套件了。这些测试检查每个服务在成功时返回 true，并且调用了被 mock 的 WeatherModel 的正确方法。创建 /__tests__/mongoose/weather/services.test.ts 文件，并加入 代码清单 8-21 中的代码。

```typescript
/**
 * @jest-environment node
 */
import {WeatherInterface} from "../../../mongoose/weather/interface";
import {
    findByZip,
    storeDocument,
    updateByZip,
    deleteByZip,
} from "../../../mongoose/weather/services";

import WeatherModel from "../../../mongoose/weather/model";
jest.mock("../../../mongoose/weather/model");

describe("the weather services", () => {

    let doc: WeatherInterface = {
        zip: "test",
        weather: "weather",
        tempC: "00",
        tempF: "01",
        friends: []
    };

    afterEach(async () => {
        jest.clearAllMocks();
    });

    afterAll(async () => {
        jest.restoreAllMocks();
    });

    describe("API storeDocument", () => {
        test("returns true", async () => {
            const result = await storeDocument(doc);
            expect(result).toBeTruthy();
        });
        test("passes the document to Model.create()", async () => {
            const spy = jest.spyOn(WeatherModel, "create");
            await storeDocument(doc);
            expect(spy).toHaveBeenCalledWith(doc);
        });
    });

    describe("API findByZip", () => {
        test("returns true", async () => {
            const result = await findByZip(doc.zip);
            expect(result).toBeTruthy();
        });
        test("passes the zip code to Model.findOne()", async () => {
            const spy = jest.spyOn(WeatherModel, "findOne");
            await findByZip(doc.zip);
            expect(spy).toHaveBeenCalledWith({zip: doc.zip});
        });
    });

    describe("API updateByZip", () => {
        test("returns true", async () => {
            const result = await updateByZip(doc.zip, doc);
            expect(result).toBeTruthy();
        });
        test("passes the zip code and the new data to Model.updateOne()", async () => {
            const spy = jest.spyOn(WeatherModel, "updateOne");
            const result = await updateByZip(doc.zip, doc);
            expect(spy).toHaveBeenCalledWith({zip: doc.zip}, doc);
        });
    });

    describe("API deleteByZip", () => {
        test("returns true", async () => {
            const result = await deleteByZip(doc.zip);
            expect(result).toBeTruthy();
        });
        test("passes the zip code Model.deleteOne()", async () => {
            const spy = jest.spyOn(WeatherModel, "deleteOne");
            const result = await deleteByZip(doc.zip);
            expect(spy).toHaveBeenCalledWith({zip: doc.zip});
        });
    });

});
```

*代码清单 8-21：__tests__/mongoose/weather/services.test.ts 中更新后的测试套件*

与前一个测试套件一样，我们先设置环境并导入模块。我们还导入了 WeatherModel，并用我们创建的被 mock 模型的路径调用 jest.mock，从而有效地替换被测代码中的原始模型。然后创建一个包含测试数据的文档，保存在常量 doc 中，稍后传给被 mock 模型的方法。与之前一样，我们用 afterEach 钩子在每次测试后重置所有 mock，用 afterAll 钩子在所有测试用例结束后移除 mock 并还原原始函数。

我们为四个服务分别创建一个嵌套的测试套件。每个套件都包含相同的两个单元测试：一个用 toBeTruthy 匹配器验证成功时的返回值，另一个侦听特定的 WeatherModel mock 函数。代码遵循与前一个测试套件相同的模式，使用的匹配器也相同。

运行 npm test 后得到的代码覆盖率报告显示，我们测试了大约 70% 的服务代码。查看最后一列列出的未覆盖行，你会发现它们包含 console.log(err); 输出。只要对模型方法的异步调用失败，就会用到这个输出：

```
PASS  __tests__/mongoose/weather/services.test.ts
PASS  __tests__/middleware/dbconnect.test.ts (7.193 s)

--------------------|---------|----------|---------|---------|-------------------
File                | % Stmts | % Branch | % Funcs | % Lines | Uncovered Lines
--------------------|---------|----------|---------|---------|-------------------
All files           |   83.63 |      100 |   88.23 |   82.35 |
 middleware         |     100 |      100 |     100 |     100 |
  db-connect.test.ts|     100 |      100 |     100 |     100 |
 mongoose/weather.  |   77.41 |      100 |     100 |   75.86 |
  services.test.ts  |   70.83 |      100 |     100 |   70.83 |8,20-22,33-35,43-45
--------------------|---------|----------|---------|---------|-------------------
```

就本章而言，我们将让这些行保持未覆盖。否则，我们可以修改被 mock 的模型使其抛出错误（例如传入一个无效的文档），然后为每个服务添加第三个测试用例来验证该错误。

### 对 REST API 执行端到端测试

复杂的 API 测试可能会使用专门的 API 测试库（如 SuperTest），它为 HTTP 状态码提供了匹配器并简化了请求和响应的处理；也可能使用 Postman 这样的 GUI 工具。在本例中，我们只使用原生 fetch 方法来测试返回的数据是否符合预期。

与前面的测试不同，这项测试不隔离任何单个组件，因为我们的目标是验证系统的所有组件能协同按预期工作。为了检查 API 在给定输入时能否从数据库返回正确的响应，我们的端到端测试做了如下假设：所有层都已被独立测试过、数据库中包含初始种子数据、应用程序运行在 http://localhost:3000/。

为了核实第一个假设，请打开 API 端点文件 pages/api/v1/weather/[zipcode].ts。你会注意到这段 API 代码导入了两个函数：服务模块中的 findByZip 和中间件的 dbConnect，两者我们都已测试过。第二个假设同样成立：数据库在每次启动时都会加载初始种子数据。在 __tests__/pages/api/v1/weather/ 中创建 zipcode.e2e.test.ts 文件，并加入 代码清单 8-22 中的代码。

```typescript
/**
 * @jest-environment node
 */

describe("The API /v1/weather/[zipcode]", () => {
    test("returns the correct data for the zipcode 96815", async () => {
        const zip = "96815";
        let response = await fetch(`http://localhost:3000/api/v1/weather/${zip}`);
        let body = await response.json();
        expect(body.zip).toEqual(zip);
    });
});

export {};
```

*代码清单 8-22：REST API 的测试套件*

我们把环境设置为 node，然后定义包含一个测试用例的测试套件。在这个用例中，我们提供一个与某个初始种子数据集匹配的邮政编码（ZIP code），接着使用原生 fetch 方法（自 Node.js 17.5 版起可用）调用本地主机上的天气 API，检查返回的邮政编码是否与传入参数一致。最后添加一条空的 export 语句，把这个文件定义为 ES6 模块。

这项测试应当通过，并达到 100% 的代码覆盖率。既然我们已经确信应用程序的核心按预期工作，接下来就可以测试用户界面组件了。

使用 fetch 时，你可能会遇到两种常见的错误信息。第一种是 ECONNREFUSED，它告诉你 fetch 无法连接到你的应用程序，因为应用没有运行。此时请用 npm run dev 启动应用；如果你没有使用 3000 端口，请调整 fetch 调用中的端口。第二种错误提示测试超过了 5,000 毫秒的超时限制。如果你是为测试而启动应用程序（而非使用已经在运行的应用），Next.js 会在测试首次调用该 API 路由时对其进行编译。视环境而定，这可能超出默认超时时间。请在文件顶部 describe 方法之前添加一行 jest.setTimeout(20000);，把超时时间提高到 20,000 毫秒而不是 5,000 毫秒。

### 用快照测试评估用户界面

快照测试验证页面的渲染 HTML 在两次测试运行之间没有变化。要用 Jest 做到这一点，我们必须先准备环境。向项目添加 jsdom 环境、react-testing-library 和 react-test-renderer：

```bash
$ npm install --save-dev jest-environment-jsdom
$ npm install --save-dev @testing-library/react @testing-library/jest-dom
$ npm install --save-dev @types/react-test-renderer react-test-renderer
```

在测试用例中模拟浏览器环境并渲染 React 组件时需要用到它们。现在相应地修改根目录下的 jest.config.js 文件，用 代码清单 8-23 中的代码替换其内容。

```javascript
const nextJest = require("next/jest");
const createJestConfig = nextJest({});

module.exports = createJestConfig(nextJest({}));
```

*代码清单 8-23：更新后的 jest.config.js 文件*

这段代码导入 next/jest 包，并导出一个带有 Next.js 项目默认属性的 Jest 配置。这是与 Next.js 兼容的最简单的 Jest 配置形式。如果你查看 https://nextjs.org/docs/testing 上的 Next.js 官方设置指南，会发现它列出了一些基本配置选项，而这些我们都用不到。

#### 第一个版本

快照测试会渲染一个组件或页面，把它的序列化 JSON 作为快照，保存在测试套件旁边的 __snapshots__ 文件夹中。在后续每次运行时，Jest 都会把当前快照与存储的参考快照进行比较；只要二者相同，快照测试即通过。要生成初始快照，请新建文件夹 __tests__/pages/components 和文件 weather.snapshot.test.tsx，然后加入 代码清单 8-24 中的代码。

```typescript
/**
 * @jest-environment node
 */

import {act, create} from "react-test-renderer";
import PageComponentWeather from "../../../pages/components/weather";

describe("PageComponentWeather", () => {
    test("renders correctly", async () => {
        let component: any;
        await act(async () => {
            component =
                await create(<PageComponentWeather></PageComponentWeather>);
        });
        expect(component.toJSON()).toMatchSnapshot();
    });
});
```

*代码清单 8-24：PageComponentWeather 的快照测试*

快照测试的开头几行把环境设置为 jsdom，并导入测试渲染器的 act 和 create 方法来测试 React 组件；组件本身在下一行导入。

接下来编写模拟的用户行为，并把组件的创建包裹在异步的 act 函数中。你可能已经猜到，这个函数的名字来自准备、执行、断言模式，它确保所有相关的 DOM 更新都已应用之后才继续执行测试用例。所有会引起 React 状态更新的语句都需要它；在这里，它会把测试执行推迟到 useEffect 钩子运行完毕之后。

然后编写一个测试用例，等待渲染 JSX 组件的 create 函数完成。这样我们就能在模拟的浏览器环境中生成 HTML 并把结果存入变量。我们等待组件渲染完成，确保在继续执行测试用例之前 HTML 已可用于后续交互。接着把渲染出的组件序列化为 JSON 字符串，并使用一个新的匹配器 toMatchSnapshot，将当前 JSON 字符串与存储的参考快照进行比较。

试运行一次，可以看到所有测试都成功了。我们注意到两个有意思的地方：测试创建了一个快照，而且测试覆盖率达到 81%：

```
PASS  __tests__/mongoose/weather/services.test.ts
PASS  __tests__/pages/api/v1/weather/zipcode.e2e.test.ts
PASS  __tests__/middleware/dbconnect.test.ts (7.193 s)
PASS  __tests__/pages/components/weather.snapshot.test.tsx

---------------------|---------|----------|---------|---------|-------------------
File                 | % Stmts | % Branch | % Funcs | % Lines | Uncovered Lines
---------------------|---------|----------|---------|---------|-------------------
All files            |   83.63 |      100 |   88.23 |   82.35 |
 middleware          |     100 |      100 |     100 |     100 |
  db-connect.test.ts |     100 |      100 |     100 |     100 |
 mongoose/weather    |   77.41 |      100 |     100 |   75.86 |
  services.test.ts   |   70.83 |      100 |     100 |   70.83 |8,20-22,33-35,43-45
 pages/api/v1/       |         |          |         |         |
  weather            |         |          |         |         |
    [zipcode].ts     |     100 |      100 |     100 |     100 |
 pages/components    |   81.81 |      100 |      60 |      80 |
  weather.tsx        |   81.81 |      100 |      60 |      80 |8,12
---------------------|---------|----------|---------|---------|-------------------
Snapshot Summary
 › 1 snapshot written from 1 test suite.
```

你可以打开 __snapshots__ 文件夹中的 weather.snapshot.test.tsx.snap 文件查看创建的快照。它应当与 代码清单 8-25 中的代码颇为相似：无非是渲染出来的 HTML 被保存为多行模板字符串而已。你的 HTML 未必与这里展示的完全一致；重要的是，每次测试运行时 react-test-renderer 渲染组件的结果看起来都相同。

```javascript
// Jest Snapshot v1, https://goo.gl/fbAQLP

exports[`PageComponentWeather renders correctly 1`] = `
<h1
    data-testid="h1"
    onClick={[Function]}
>
    The weather is
    sunny
    , and the counter shows
    0
</h1>
`;
```

*代码清单 8-25：包含序列化 HTML 的 weather.snapshot.test.tsx.snap 文件*

我们还看到计数器被设为 0，这表明在创建快照之前 useEffect 尚未运行。如果打开组件文件查看未覆盖的行，你会发现它们与增加状态变量的点击处理函数以及（正如所怀疑的）useEffect 钩子有关。我们也想测试这些核心功能。

#### 第二个版本

我们将修改测试代码，覆盖之前未测试的功能。把 代码清单 8-26 中的代码粘贴到快照测试文件中。

```typescript
/**
 * @jest-environment node
 */

import {act, create} from "react-test-renderer";
import PageComponentWeather from "../../../pages/components/weather";

describe("PageComponentWeather", () => {
    test("renders correctly", async () => {
        let component: any;
        await act(async () => {
            component = await create(<PageComponentWeather></PageComponentWeather>);
        });
        expect(component.toJSON()).toMatchSnapshot();
    });
 
    test("clicks the h1 element and updates the state", async () => {
        let component: any;
        await act(async () => {
            component = await create(<PageComponentWeather></PageComponentWeather>);
            component.root.findByType("h1").props.onClick();
        });
        expect(component.toJSON()).toMatchSnapshot();
    });
 
});
```

*代码清单 8-26：更新后的快照测试*

在更新后的代码中，我们添加了另一个测试用例：找到页面上的标题元素并模拟用户点击它。回忆前几章的内容，这会增大状态变量 counter。这里我们同样等待组件创建完成，并使用了 act 函数。

如果重新运行测试，你会看到一次失败。测试运行器告诉我们快照不匹配：

```
FAIL  __tests__/pages/components/weather.snapshot.test.tsx
  • PageComponentWeather › renders correctly
--snip--
 › 1 snapshot failed.
--snip--
Snapshot Summary
 › 1 snapshot failed from 1 test suite.
› Inspect your code changes or run `npm test -- -u` to update them.
```

由于我们修改了测试用例，让它等待 useEffect 钩子并把状态变量 counter 设为 1 而不是 0，DOM 也随之改变。按照测试运行器的建议，用 npm test -- -u 重新运行测试，创建一个新的、更新后的快照。现在测试应当成功，并报告组件的测试覆盖率达到 100%。

试着用刚学到的知识做些实验。例如，你能为 pages 目录中的页面路由编写快照测试，或为 GraphQL API 编写一组端到端测试吗？

## 总结

现在，你应当能够使用 Jest 创建自动化测试，并能更广泛地自行设计测试方案，在投入与收益之间取得平衡。我们讨论了 TDD 和单元测试的好处，然后运用准备、执行、断言模式，遵循测试驱动原则开发了一个简单的 sum 函数。接着，在计算斐波那契数列时，我们用三种类型的测试替身替换了 sum 函数。最后，我们在现有的 Next.js 应用中添加了单元测试和快照测试，创建了一个 Mongoose 模型的 mock，并使用侦听器验证了我们的假设。

要进一步了解 Jest 和自动化测试，请查阅 Jest 官方文档 https://jestjs.io/docs/getting-started。下一章将探讨授权（authorization）与认证（authentication）的区别，以及如何在应用程序中利用 OAuth。