# 第 3 章 TypeScript

*Chapter 3: TypeScript*

TypeScript 是一门为动态类型的 JavaScript 语言添加静态类型（static typing）的编程语言。它是 JavaScript 在语法上的严格超集（superset），这意味着所有现有的 JavaScript 代码都是合法的 TypeScript 代码；反过来，TypeScript 却不是合法的 JavaScript，因为它提供了额外的特性。

本章将带你了解使用 JavaScript 动态类型时的种种陷阱，并解释 TypeScript 的静态类型如何帮助我们在早期发现错误，从而提升代码的稳定性。全栈开发者已经广泛接纳了 TypeScript：在最近一次 Stack Overflow 开发者调查中，它在“最受期待（most wanted）”类别中名列第二；在一项 State of JS 调查中，78% 的受访者表示正在使用它。根据 https://builtwith.com 的统计，访问量排名前 10,000 的网站中有 7% 基于 TypeScript。

我们将介绍构建全栈应用所必需的 TypeScript 基础与进阶概念。在此过程中，你会了解这门语言最常见的配置选项、最重要的类型，以及如何、何时使用 TypeScript 的静态类型特性。

## TypeScript 的优势

TypeScript 让我们使用 JavaScript 的类型系统时更不容易出错，因为它的编译器（compiler）能让我们立即看到类型错误。JavaScript 是动态类型的，声明变量时无须指定其类型；运行时（runtime）一旦执行脚本，就会根据用法检查这些类型。但这意味着，由无效类型导致的错误（例如，对一个保存着数字而非数组的变量调用 `array.map`）只有到运行时才会被发现，而此时整个程序已经崩溃。

除了动态类型之外，JavaScript 还是弱类型（weakly typed）的，也就是说它会隐式地把变量转换成看起来最合理的值。代码清单 3-1 展示了从数字到字符串的一次隐式转换。

```typescript
let string = "1";
let number = 1;
let result;

result = number + number;
console.log("value: ", result, " type of ", typeof(result));

result = number + string;
console.log("value: ", result, " type of ", typeof(result));
```

*代码清单 3-1：JavaScript 中从数字到字符串的隐式转换*

我们声明了三个变量：第一个赋值为字符串，第二个赋值为数字，第三个赋值为用算术加号（+）运算符把 number 与自身相加的结果。随后我们把这次求和运算的结果及其类型输出到控制台。如果执行这段代码，你会看到值是数字，且运行时给该变量指定了 number 类型。

接着，我们再次使用同一个运算符，不过这次不是把数字与 number 变量相加，而是把一个字符串与它相加。你会看到输出的值是 11，而不是你可能预期的 2。此外，该变量的类型也变成了 string。这是因为运行时环境需要处理一个不可能完成的任务：把数字和字符串相加。它的解决办法是隐式地把数字转换为字符串，然后用加号运算符拼接这两个字符串。如果没有 TypeScript，我们只有运行代码时才会注意到这次转换。

无类型变量导致的另一类常见问题与函数和 API 契约（contract）有关，也就是关于代码接受什么、返回什么的约定。函数接受参数时，它隐式地期望特定类型的参数；但如果没有 TypeScript，就无法确保参数类型正确。函数的返回值也存在同样的问题。为了说明这一点，代码清单 3-2 修改了代码清单 3-1 的代码，改用函数来计算 result 变量的值。

```typescript
let string = "1";
let number = 1;
let result;

const calculate = (a, b) => a + b;

result = calculate(number, number);
console.log("value: ", result, " type of ", typeof(result));

result = calculate(number, string);
console.log("value: ", result, " type of ", typeof(result));
```

*代码清单 3-2：因隐式类型转换而可能返回无效类型的函数*

新的 calculate 函数接受两个参数 a 和 b，并像之前一样把两个值相加。与代码清单 3-1 一样，一旦我们传入一个数字和一个字符串作为参数，函数返回的就是字符串而不是数字。我们的函数或许期望两个参数都是数字，但如果不按代码清单 3-3 那样的逻辑手动检查类型，就无法验证这一点。

```typescript
let string = "1";
let number = 1;
let result;

const calculate = (a, b) => {
    if (Number.isInteger(a) === false || Number.isInteger(b) === false) {
        throw new Error("Invalid type: a parameter is not an integer");
    } else {
        return a + b;
    }
};

result = calculate(number, number);
console.log("value: ", result, " type of ", typeof(result));

result = calculate(number, string);
console.log("value: ", result, " type of ", typeof(result));
```

*代码清单 3-3：重构后的类型安全函数*

这里我们使用 Number 对象原生的 `isInteger` 函数来验证参数 a 和 b 是否为整数。第一次调用传入两个整数，应当会按预期计算出结果；第二次调用传入一个整数和一个字符串，在编辑器里看起来没有任何问题。然而一旦运行代码，运行时环境就会抛出错误 `Invalid type: a parameter is not an integer`。

手动检查类型主要有两个问题。第一，它给代码带来了大量噪音：每次处理函数或 API 契约（例如接受参数或返回值）时，我们都要检查所有可能的类型。第二，开发过程中我们得不到问题的提醒。要想看到动态类型语言中的错误，必须执行代码，让解释器（interpreter）在运行时把错误报告给我们。

与动态类型语言不同，静态类型语言在编译阶段、也就是运行时之前执行类型检查。TypeScript 编译器（TSC）负责这项工作；它可以在代码编辑器或 IDE 的后台运行，立即报告所有由无效类型使用引起的错误。因此，你能够尽早捕获错误，并看到每个变量被赋予的类型和数据结构。

即便不配置这样的即时反馈，代码也必须先经过 TSC 处理才能使用，这就确保了这类错误能比不使用 TSC 时更早被发现。相比 JavaScript，使用 TypeScript 最重要的好处之一就是能够检查这类错误。我们将在第 38 页的“类型注解”（type annotation）一节中讨论如何从类型注解中受益以及何时使用它们。

## 配置 TypeScript

TypeScript 的语法不是合法的 JavaScript，因此常规的 JavaScript 运行时环境无法直接执行它。要在 Node.js 或浏览器中运行 TypeScript，我们首先需要用 TSC 把它转换成常规的、向后兼容的 JavaScript，然后再执行得到的 JavaScript。

尽管名叫编译器，TSC 实际上并不是把 TypeScript 编译（compile）成 JavaScript，而是对它进行转译（transpile）。区别在于抽象层级：编译器生成低层级代码，而转译器（transpiler）是一种源到源编译器，它在抽象程度大致相同的语言之间产出等价的源代码。例如，可以把 ES.Next 转译为旧式 JavaScript，或把 Python 2 转译为 Python 3。（当然，transpiling 和 compiling 这两个术语经常被混用。）

除了把 TypeScript 转换为 JavaScript，TSC 还会检查代码中的类型错误，并验证函数之间的契约。转译与类型检查是相互独立进行的：无论你定义了什么类型，TSC 都会产出 JavaScript。TypeScript 的错误只是构建过程中发出的警告，只要 JavaScript 本身没有错误，它们就不会中断转译步骤。

使用 TypeScript 不会影响代码的性能。编译器会在转译阶段移除类型和类型操作，实质上是把所有 TypeScript 语法从实际的 JavaScript 代码中剥离。因此，类型不可能影响运行时行为，也不会影响最终代码的体积。所以 TypeScript 并不比 JavaScript 慢，只是转译过程可能需要一些时间。

### 在 Node.js 中安装

如果使用 Node.js，你应当在项目的 package.json 文件中，用 `--save-dev` 标志把 TypeScript 和所有类型定义声明为开发依赖（development dependency）。没有必要全局安装 TypeScript，直接用下面的 npm 命令把 TypeScript 加入项目即可：

```bash
$ npm install --save-dev typescript
```

TypeScript 文件使用 `.ts` 扩展名。由于 TypeScript 是 JavaScript 的超集，所有合法的 JavaScript 代码自动就是合法的 TypeScript 代码。因此，你可以把 `.js` 文件重命名为 `.ts`，立即对现有代码启用静态类型检查器（type checker）。

tsconfig.json 文件定义了 TSC 的配置选项，我们将在下一小节介绍其中最重要的几个。现在，先运行以下命令生成一个采用默认配置的新文件：

```bash
$ npx tsc -init
```

TSC 会在当前路径以及所有父级目录中查找该文件。可选的 `-p` 标志可以让 TypeScript 编译器直接指向该文件；随后 TSC 会从这个文件读取配置信息，并把该文件所在的目录当作 TypeScript 的根目录。

> **注**：如果你想跟随本章的示例，又不想专门创建一个项目，可以在在线 playground（https://www.typescriptlang.org/play）中运行代码，而不必在本地安装 TypeScript。

### tsconfig.json 文件

我们来看一下 tsconfig.json 文件的基本结构。生成文件的内容取决于你安装的 TypeScript 版本，配置属性大约有 100 个，但对大多数项目来说，只有下面这几个是相关的：

```json
{
    "extends": "@tsconfig/recommended/tsconfig.json",
    "compilerOptions": {},
    "include": [],
    "exclude": []
}
```

`extends` 选项是一个字符串，用来配置另一个类似配置文件的路径。通常，这个属性会扩展一个预设（preset）作为模板，再做少量项目特定的调整。它的工作方式类似于面向对象编程中基于类（class）的继承：预设覆盖基础配置，而配置中的键值对又覆盖预设。这里的示例使用 TypeScript 推荐配置文件来覆盖默认设置。

`compilerOptions` 字段配置转译步骤，它的选项列于附录 A 中。`include` 的值是一个字符串数组，指定要纳入转译的模式或文件名；`exclude` 的值也是一个字符串数组，指定要排除的模式或文件名。注意，TSC 是在根据 include 模式找到的文件列表上应用这些模式的。通常我们不需要包含或排除文件，因为整个项目都会由 TypeScript 代码组成，所以可以把这两个数组留空。

### TypeScript 的动态反馈

大多数现代代码编辑器都支持 TypeScript，会把 TSC 产生的错误直接显示在代码中。还记得我们用来解释 TypeScript 如何验证函数契约的 calculate 函数吗？图 3-1 是 Visual Studio Code 的截图，其中高亮显示了类型错误并提示了解决方案。

*图 3-1：在 Visual Studio Code 中使用 TypeScript*

你可以使用任何喜欢的代码编辑器或 IDE 来编写 TypeScript 代码，不过建议选用能提供这种动态反馈的编辑器。

## 类型注解

类型注解（type annotation）是一种可选的方式，用来显式地告诉运行时环境应当期望哪些类型。添加注解遵循这样的模式：`variable: type`（变量：类型）。下面的示例展示了 calculate 函数的一个版本，我们把两个参数都标注为 number：

```typescript
const calculate = (a: number, b: number) => a + b;
```

有些开发者倾向于给代码里的所有东西都加上类型，这样做反而增加了噪音，降低了代码的可读性。这种反模式（antipattern）叫作过度标注类型（over-typing），它源于对类型注解工作方式的误解。TypeScript 编译器会根据用法推断类型，因此你不需要显式地为所有内容标注类型。代码编辑器会在后台运行 TSC，并利用其结果显示推断出的类型信息和编译器错误，就像你在“TypeScript 的动态反馈”一节中看到的那样。

类型注解真正的用途，是确保代码遵守 API 契约。有三种场景需要验证契约，其中只有一种特别重要：第一种场景是在声明变量时，通常不推荐这样做；第二种是标注函数的返回值，属于可选；第三种是标注函数的参数，这是必不可少的。下面我们详细讨论这三种情况。

### 声明变量

给变量标注类型最显而易见的位置是赋值或声明时。代码清单 3-4 演示了这一点：把变量 weather 显式标注为 string，然后赋给它一个字符串值。

```typescript
let weather: string = "sunny";
```

*代码清单 3-4：变量声明时的过度标注*

然而在大多数情况下，这是一种过度标注，因为你完全可以利用编译器的类型推断（type inference）。代码清单 3-5 展示了使用类型推断的替代写法。

```typescript
let weather = "sunny";
```

*代码清单 3-5：根据变量的值推断其类型*

由于 TSC 会自动推断这个变量的类型，当你把鼠标悬停在变量上时，代码编辑器会显示类型信息。省去显式注解后，语法简洁得多，也避免了冗余的类型声明给代码带来的噪音，从而提升了代码的可读性。这正是通常要避免这种过度标注的原因。

### 声明返回值

尽管 TypeScript 能够推断函数的返回类型，你通常还是应该显式地标注它。这种代码模式能确保函数的契约得到遵守，因为编译器会在函数定义处（而不是函数使用处）显示实现错误。

在这种情况下使用类型注解的另一个原因是：作为程序员，你必须显式地定义函数要做什么。通过厘清函数的输入和输出类型，你会更清楚地理解自己究竟想让函数做什么。代码清单 3-6 展示了如何在声明函数时声明其返回类型。

```typescript
function getWeather(): string {
    const weather = "sunny";
    return weather;
}
```

*代码清单 3-6：在声明时标注函数的返回值*

我们创建了一个函数，返回前面声明过的 weather 变量。weather 变量具有推断出的 string 类型，因此该函数返回一个字符串。我们的类型定义显式地设定了函数的返回类型。

### 声明函数参数

给函数的参数添加注解是必不可少的，因为在大多数情况下，TypeScript 没有足够的信息来推断函数参数。通过给这些参数标注类型，你是在告诉编译器：在调用函数并传入实参时要检查类型。请看代码清单 3-7 中这个模式的实际运用。

```typescript
const weather = "sunny";
function getWeather(weather: string): string {
    return weather;
};
getWeather(weather);
```

*代码清单 3-7：标注函数的参数*

这一次，我们不再在函数内部把 weather 变量声明为常量，而是希望返回值是动态的。因此，我们修改函数让它接受一个参数并立即返回该参数，然后用 weather 常量作为参数调用这个函数。

好的 TypeScript 代码会避免噪音，依赖类型推断：始终标注函数的参数，倾向于标注返回值，但从不标注局部变量。

## 内置类型

在使用 TypeScript 及其注解之前，你需要了解有哪些类型可用。TypeScript 的主要好处之一是让你能够显式声明 JavaScript 的任何原始类型（primitive type）。此外，TypeScript 还加入了自己的类型，其中最重要的是联合类型（union type）、元组（tuple）、any 和 void。你也可以定义自定义类型和接口（interface）。

### JavaScript 原始类型

JavaScript 有五种原始类型：字符串（string）、数字（number）、布尔值（Boolean）、undefined 和 null。这门语言中的其他一切都被视为对象（object）。代码清单 3-8 展示了在定义这些 JavaScript 原始类型的变量时附加 TypeScript 类型注解的语法。（请记住，大多数情况下这种场景直接依赖编译器的类型推断即可。）

```typescript
let stringType: string = "bar";
let booleanType: boolean = true;
let integerType: number = 1;
let floatType: number = 1.5;
let nullType: null = null;
let undefinedType: undefined = undefined;
```

*代码清单 3-8：带有 TypeScript 类型注解的 JavaScript 原始类型*

首先，我们用 TypeScript 注解定义了一个字符串变量和一个布尔值，它们与 JavaScript 中的字符串和布尔值完全相同。接着我们定义了两个数字。与 JavaScript 一样，TypeScript 对数字只使用一个通用类型，不区分整数和浮点数。最后我们来看 TypeScript 的 null 和 undefined 类型，它们的行为与 JavaScript 中同名的原始类型一致。null 指向一个为空或不存在的值，表示值的有意缺席；相反，undefined 表示值的无意缺席。在代码清单 3-5 中我们没有为 undefined 类型赋值，因为我们并不知道这个值。

### union 类型

还有一些额外的类型你应当了解，因为类型注解越精确，TSC 就越能帮上忙。TypeScript 把联合类型引入了 JavaScript 生态系统。联合类型是指可以拥有多种数据类型的变量或参数。代码清单 3-9 展示了一个既可以是字符串又可以是数字的联合类型示例。

```typescript
let stringOrNumberUnionType: string | number;
stringOrNumberUnionType = "bar";
stringOrNumberUnionType = 1;
stringOrNumberUnionType = true;
```

*代码清单 3-9：TypeScript 的联合类型*

我们声明了一个联合类型变量，它既可以包含字符串，也可以包含数字，但不能是其他类型。一旦我们赋给它一个布尔值，TSC 就会抛出错误，IDE 会显示消息 `Type 'boolean' is not assignable to type 'string | number'`。

尽管你会发现联合类型在标注函数参数和可包含不同类型元素的数组时很有用，但仍应节制使用，并尽可能避免。这是因为，在处理联合类型的值之前，你需要执行额外的手动类型检查，否则它们可能引发错误。例如，如果你遍历一个由字符串或数字组成的数组并把所有项相加，就必须先把所有字符串转换成数字；否则，JavaScript 会像本章前面展示的那样，把数字隐式转换为字符串。

### array 类型

TypeScript 提供了一个泛型（generic）的数组类型，提供与 JavaScript 数组类似的数组函数。不过，请仔细看看代码清单 3-10 中为数组标注类型的语法，你会注意到数组的类型取决于数组项的类型。

```typescript
let genericArray: [] = [];
genericArray.push(1);

let numberArray: number[] = [];
numberArray.push(1);
```

*代码清单 3-10：带类型的数组*

首先，我们定义了一个没有指定项类型的数组。遗憾的是，这个看似泛型数组的定义会在后续引发问题：一旦我们试图添加值，TSC 就会抛出错误 `Argument of type 'number' is not assignable to parameter of type 'never'`，因为这个数组没有被标注类型。

因此，我们需要给数组中的项标注类型。于是我们创建了 numberArray 数组，其中每一项的类型都是 number。现在我们可以向数组添加数值而不会遇到错误了。

### object 类型

TypeScript 内置的 object 类型与 JavaScript 的 object 相同。虽然你可以为各个属性定义类型，让 TSC 进行类型检查，但编译器无法保证属性的顺序。尽管如此，它仍会对属性进行类型检查，如代码清单 3-11 所示。

```typescript
let weatherDetail: {
    weather: string,
    zipcode: string,
    temp: number
} = {weather: "sunny", zipcode: "00000", temp: 1};
weatherDetail.weather = 2;
```

*代码清单 3-11：带类型的对象*

这里我们定义了一个具有三个属性的对象：两个属性接受字符串，另一个接受数字。然后我们试图把一个数字赋给标注为 string 的 weather 属性。这时 TSC 会报错，指出我们赋了错误类型的值。

注意，通常应当避免像本例这样内联（inline）地为对象标注类型。更好的做法是创建自定义类型（custom type），它可复用，还能避免代码杂乱，提高可读性。我们将在第 44 页的“自定义类型与接口”一节讨论如何创建和使用它们。

### tuple 类型

TypeScript 为 JavaScript 增添的另一种常见类型是元组（tuple）类型。如代码清单 3-12 所示，元组是具有指定数量、且各项带类型的数组。TypeScript 的元组与你在 Python、C# 等编程语言中可能遇到过的元组类似。

```typescript
let validTuple: [string, number] = ["bar", 1];
let invalidTuple: [string, number] = [1, "bar"];
```

*代码清单 3-12：TypeScript 的元组类型*

我们定义了两个元组，二者的第一项都是字符串，第二项都是数字。如果加入元组的项在类型、顺序或数量上与元组的声明不一致，TSC 就会抛出错误。这里第一个赋值是可以接受的，而第二个赋值会抛出两个错误，指出类型不匹配。

### any 类型

TypeScript 的 any 类型是泛化的（generic），意思是它可以接受任何值，你应当避免使用它。如代码清单 3-13 所示，它接受所有值而不抛出任何错误，这使静态类型失去了意义。

```typescript
let indifferent: any = true;
indifferent = 1;
indifferent = [];
```

*代码清单 3-13：TypeScript 的 any 类型*

使用 any 似乎是个省事的选择，把它当作逃生舱口也很有诱惑力，但要不惜一切代价避免这样做。当你把 any 作为值传给某个函数（举例来说）时，就破坏了你在函数声明中指定的契约；而当你用 any 来定义契约时，实际上就等于没有契约。

我们来看一个使用 any 类型引发问题的场景，见代码清单 3-14。

```typescript
const calculate = (a: any, b: any): any => a + b;
console.log(calculate (1,1));
console.log(calculate ("1",1));
```

*代码清单 3-14：any 类型引发的问题*

我们复用了那个把两个数字相加的 calculate 函数。当传入两个数字时，会得到预期的输出 2。在前面的示例中，我们把参数标注为 number，从而防止了把无效类型用作实参。

然而，当我们用 any 代替 number 并把一个字符串传给函数时，TSC 不会报错。JavaScript 会把数字隐式转换为字符串，返回出人意料的值 11。我们在本章开头那个无类型的函数版本中见过这种行为。可以看到，使用 any 与完全不使用类型没有区别。

any 类型虽然方便，却会在编程过程中掩盖你的 bug、隐藏你的类型设计，使类型检查形同虚设；它还会阻止 IDE 显示错误和无效类型。

### void 类型

TypeScript 的 void 类型与 any 相反：它表示完全没有类型。它唯一的用例是标注本不应有返回值的函数的返回值，如代码清单 3-15 所示。

```typescript
function log(msg: string): void {
    console.log(msg);
}
```

*代码清单 3-15：TypeScript 的 void 类型*

我们在这里定义的自定义 log 函数把一个参数传递给控制台，它不返回任何东西，因此我们用 void 作为返回类型。

要进一步了解 TypeScript 的类型以及这门语言的其他重要细节，请参阅《The TypeScript Handbook》（https://www.typescriptlang.org/docs/handbook/intro.xhtml）。

## 自定义类型与接口

前面几节介绍的 TypeScript 知识已足够你开始使用这门语言。不过，再了解一些更进阶的概念会对你有所帮助。本节将展示如何创建自定义类型，以及如何在 TypeScript 代码中使用没有类型的第三方库。你还将学到何时该创建新类型、何时该使用自定义接口。

使用 TypeScript 时请记住：没有顶层 import 或 export 的 TypeScript 文件不是模块（module），因此它运行在全局作用域（global scope）中，它的所有声明在其他模块中也可以访问。相反，带有顶层 import 或 export 的 TypeScript 文件自身就是一个模块，其所有声明都被限制在模块作用域内，也就是说只在当前模块的作用域内可用。

### 定义自定义类型

TypeScript 允许你使用 type 关键字定义自定义类型。自定义类型是简化代码的好办法。为了说明这一点，请回顾代码清单 3-8 中创建带类型对象的代码，再看看代码清单 3-16，它用自定义类型定义优化了这段代码。你会发现它整洁得多，也更好读。

```typescript
type WeatherDetailType = {
    weather: string;
    zipcode: string;
    temp?: number;
};

let weatherDetail: WeatherDetailType = {
    weather: "sunny",
    zipcode: "00000",
    temp: 30
};
const getWeatherDetail = (data: WeatherDetailType): WeatherDetailType => data;
```

*代码清单 3-16：在 TypeScript 中为带类型的对象使用自定义类型*

我们用 type 关键字创建了自定义类型 WeatherDetailType。注意，其整体语法与定义对象时类似；我们用等号（=）把定义赋给这个自定义类型。

这个自定义类型有两个必需属性：weather 和 zipcode；此外还有一个可选属性 temp，由问号（?）标示。现在创建 getWeatherDetail 函数时，我们可以把参数 weatherDetail 标注为一个类型为 WeatherDetailType 的对象。利用这种技巧，我们避免了内联注解，之后还能复用这个自定义类型，比如用它标注函数的返回类型。

### 定义接口

除了类型，TypeScript 还有接口（interface）。不过，类型与接口之间的区别比较模糊，你可以自由决定用哪一个，只要在代码中遵循一致的约定即可。

一般来说，我们认为类型定义回答的是“这个数据是哪种类型？”这个问题，可能的答案可以是联合类型或元组。接口则是一种描述数据形状（shape）的方式，比如对象的属性，它回答的是“这个对象有哪些属性？”。最实际的差异在于：与接口不同，类型在声明之后无法直接修改。要深入了解二者的区别，请查阅《The TypeScript Handbook》。

根据经验，应该用接口来定义新对象或对象的方法。更宽泛地说，可优先考虑用接口而不是类型，因为接口能提供更精确的错误消息。接口在 React 中的经典用例是定义某个组件的属性（props）。代码清单 3-17 展示了如何用 interface 关键字创建一个新接口，来替换代码清单 3-16 中的类型。

```typescript
interface WeatherProps {
    weather: string;
    zipcode: string;
    temp?: number;
}

const weatherComponent = (props: WeatherProps): string => props.weather;
```

*代码清单 3-17：TypeScript 函数的自定义接口*

这里我们用 interface 关键字定义了一个新接口。与自定义类型的定义不同，接口的定义不使用等号把接口的属性赋给其名称。随后我们用这个自定义接口为 weatherComponent 的属性对象 props 标注类型，该组件返回一个字符串。

### 使用类型声明文件

为了让自定义类型随处可用，你可以把它们定义在类型声明文件（type declaration file）中，这类文件的扩展名是 `.d.ts`。与使用 `.ts` 或 `.tsx` 扩展名的常规 TypeScript 文件不同，类型声明文件不应包含任何实现代码；TSC 会用这些类型定义来理解自定义类型并执行类型检查。它们不会被转译成 JavaScript，也永远不会成为所执行脚本的一部分。

当你需要使用外部代码库时，类型声明文件就派上用场了。第三方库往往不是用 TypeScript 编写的，因此不会为自己的代码库提供类型声明文件。幸运的是，DefinitelyTyped 仓库（http://definitelytyped.github.io）为 7000 多个库提供了类型声明文件。使用这些文件就能为这些库添加 TypeScript 支持。

类型声明文件统一收录在 npm 的 @types 作用域（scope）下。这个作用域保存了来自 DefinitelyTyped 的所有声明，因此很容易找到，并且在你的 package.json 文件里彼此挨在一起。@types 作用域下的所有类型声明文件都应视为项目的开发依赖，所以我们在 npm install 命令中加上 `--save-dev` 标志来安装它们。

代码清单 3-18 展示了一个最小的类型声明文件示例，它为某个 API 导出一个类型和一个接口。

```typescript
interface WeatherQueryInterface {
    zipcode: string;
}

type WeatherDetailType = {
    weather: string;
    zipcode: string;
    temp?: number;
};
```

*代码清单 3-18：定义自定义类型与接口*

把这些定义保存到根目录下一个名为 custom.d.ts 的文件中，TSC 会自动加载这些定义。现在你就可以在自己的 TypeScript 模块中使用该文件里的类型和接口了。

## 练习 3：用 TypeScript 扩展 Express.js

让我们运用刚学到的 TypeScript 知识，重写你在练习 1 和练习 2 中创建的 Express.js 服务器。除了添加类型注解，我们还将使用自定义类型为服务器添加一条新路由。

### 准备工作

首先按照第 36 页“配置 TypeScript”一节中描述的步骤，把 TypeScript 加入项目。接着，由于 Express.js 本身没有类型，运行以下命令把来自 DefinitelyTyped 的类型定义添加到项目中：

```bash
$ npm install --save-dev @types/express
```

你的 package.json 文件现在应该类似于这样：

```json
{
    "name": "sample-express",
    "version": "1.0.0",
    "description": "sample express server",
    "license": "ISC",
    "type": "module",
    "dependencies": {
        "express": "^4.18.2",
        "node-fetch": "^3.2.6"
    },
    "devDependencies": {
        "@types/express": "^4.17.15",
        "typescript": "^4.9.4"
    }
}
```

现在你可以为项目创建配置文件和类型声明文件了。

### 创建 tsconfig.json 文件

在 sample-express 文件夹中、index.ts 文件旁边新建一个 tsconfig.json 文件，或者打开你之前创建的那个文件，然后用以下代码添加或替换其内容：

```json
{
    "compilerOptions": {
        "esModuleInterop": true,
        "module": "es6",
        "moduleResolution": "node",
        "target": "es6",
        "noImplicitAny": true
    }
}
```

我们为这个简单的 Express.js 服务器配置 TypeScript，这只需要少数几项设置。我们的 TypeScript 代码使用 ES.Next 模块，并且希望在把 TypeScript 转译成 JavaScript 之后仍保留模块形式，因此把 module 和 target 都设为 es6。express 包是一个 CommonJS 模块，所以我们需要使用 esModuleInterop 选项，并把 moduleResolution 设为 node。最后，我们使用 noImplicitAny 选项来禁止隐式使用 any 类型，要求显式标注类型。附录 A 更详细地描述了这些配置选项。

### 定义自定义类型

对于我们的服务器，我们将遵循一条简单的经验法则：每次使用对象时，都应考虑为项目添加一个自定义类型或接口。如果这个对象是函数参数，我们就创建自定义接口；如果这个特定对象会被使用不止一次，我们就创建自定义类型。

为了给这个示例项目定义自定义类型，我们在 sample-express 文件夹中、index.ts 文件旁边创建一个 custom.d.ts 文件，并加入代码清单 3-19 中的代码。

```typescript
type responseItemType = {
    id: string;
    name: string;
};

type WeatherDetailType = {
    zipcode: string;
    weather: string;
    temp?: number;
};

interface WeatherQueryInterface {
    zipcode: string;
}
```

*代码清单 3-19：custom.d.ts 文件*

我们创建了两个自定义类型和一个接口。其中一个类型定义了异步 API 调用的响应项；另一个类型和这个接口与本章前面给出的示例类似。它们是我们稍后要创建的新天气路由所必需的。

### 给 routes.ts 文件添加类型注解

接下来，我们必须给服务器代码添加类型注解。把 sample-express 文件夹中的 routes.js 文件重命名为 routes.ts，让 TSC 对这个文件生效。你应该会立刻看到编辑器中出现错误和警告。花点时间看看它们，然后把文件内容调整为与代码清单 3-20 一致。原书中所有类型注解均以粗体标出。

```typescript
import fetch from "node-fetch";

const routeHello = (): string => "Hello World!";

const routeAPINames = async (): Promise<string> => {
    const url = "https://www.usemodernfullstack.dev/api/v1/users";
    let data: responseItemType[];
    try {
        const response = await fetch(url);
        data = (await response.json()) as responseItemType[];
    } catch (err) {
        return "Error";
    }
    const names = data
        .map((item) => `id: ${item.id}, name: ${item.name}`)
        .join("<br>");
    return names;
};

const routeWeather = (query: WeatherQueryInterface): WeatherDetailType =>
    queryWeatherData(query);

const queryWeatherData = (query: WeatherQueryInterface): WeatherDetailType => {
    return {
        zipcode: query.zipcode,
        weather: "sunny",
        temp: 35
    };
};

export {routeHello, routeAPINames, routeWeather};
```

*代码清单 3-20：带类型的 routes.ts 文件*

按照第 38 页“类型注解”一节讨论的原则，我们只标注函数的参数和返回类型。只有在局部变量的类型无法推断时才对它进行标注，例如把 fetch 的响应转换为 JSON 的时候。这里我们需要用自定义的 responseItemType 显式标注该变量的类型，并把转换的返回值断言（cast）为 responseItemType 数组。

在这个清单的其余部分，我们为新增的天气路由创建函数。两个函数的参数都用自定义接口标注类型，返回类型则使用自定义类型。在这个基础示例中，查询函数返回的基本是静态数据，只有 ZIP 编码来自传入的参数。在正式的实现中，应该用这个 ZIP 编码去查询数据库并取回真实数据。

最后，我们把天气端点的新路由加入 export 语句。

### 给 index.ts 文件添加类型注解

把 sample-express 文件夹中的 index.js 文件重命名为 index.ts，并调整代码使其与代码清单 3-20 一致。除了添加必要的类型注解之外，还要创建一个新端点，并遵循 TypeScript 的约定给未使用的参数加上下划线（_）前缀，如代码清单 3-21 所示。

```typescript
import {routeHello, routeAPINames, routeWeather} from "./routes.js";
import express, {Request, Response} from "express";

const server = express();
const port = 3000;

server.get("/hello", function (_req: Request, res: Response): void {
    const response = routeHello();
    res.send(response);
});

server.get("/api/names",
    async function (_req: Request, res: Response): Promise<void> {
        let response: string;
        try {
            response = await routeAPINames();
            res.send(response);
        } catch (err) {
            console.log(err);
        }
    }
);

server.get(
    "/api/weather/:zipcode",
    function (req: Request, res: Response): void {
        const response = routeWeather({zipcode: req.params.zipcode});
        res.send(response);
    }
);

server.listen(port, function (): void {
    console.log("Listening on " + port);
});
```

*代码清单 3-21：带类型的 index.ts 文件*

首先，我们从现有路由中导入新的天气路由，并从 express 包导入 Request 和 Response 类型。这些都是命名导出（named export），因此我们使用花括号（{}）。

然后，我们遵循最佳实践添加代码注解，同时给未使用的 req 参数加上下划线前缀。TSC 会遵循函数式编程语言的约定，忽略这些参数。api/names 入口被标记为 async 函数，因此它需要返回一个包裹在 promise 中的值；由于实际上什么也不返回，我们用 void 作为 promise 的值。

在接下来的几行代码中，我们为新端点 /api/weather/:zipcode 创建了一条额外的路由。冒号（:）会在请求的 params 对象上创建一个参数。我们通过 `req.params.zipcode` 取得 zipcode 的值，并把它传递给 routeWeather 函数。注意，这一次请求参数没有下划线前缀。最后，我们使用与之前相同的函数启动 Express.js 服务器并监听 3000 端口。

### 转译并运行代码

要用 TypeScript 编译器把代码转译成 JavaScript，在命令行中用 npx 运行 TSC：

```bash
$ npx tsc
```

TSC 会从 TypeScript 文件生成两个新文件：index.js 和 routes.js。在命令行中用常规的 Node.js 调用启动服务器：

```bash
$ node index.js
Listening on 3000
```

现在在浏览器中访问 http://localhost:3000/api/weather/12345，你应该能看到 ZIP 编码为 12345 的天气详情，如图 3-2 所示。

*图 3-2：Node.js Web 服务器的浏览器响应*

成功了！你已经写出了第一个 TypeScript 应用。

## 总结

本章教给你创建全栈应用所需的 TypeScript 知识。我们在一个新项目中配置了 TypeScript 和 TSC，然后讨论了它最重要的配置选项。接着，你学会了高效地使用 TypeScript：利用类型注解推断来避免过度标注。

我们还讨论了原始类型和高级内置类型，以及如何创建自定义类型与接口。最后，你运用新学的知识，为前面练习中构建的 Express.js 服务器添加了 TypeScript，并用类型注解、自定义类型和接口重构了代码。

如果你想成为 TypeScript 专家，我推荐《The TypeScript Handbook》以及 https://www.typescripttutorial.net 上的教程。下一章你将认识 React——一个用于构建用户界面的声明式 JavaScript 库。
