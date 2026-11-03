# 附录 A TypeScript 编译器选项

*Appendix A: TypeScript Compiler Options*

将以下任意选项传入 tsconfig.json 文件的 `compilerOptions` 字段，即可配置 TSC 将 TypeScript 代码转译（transpile）为 JavaScript 的方式。有关此过程的更多信息，请参阅第 3 章。

这里我们介绍最常用的选项。更多信息及完整列表可在官方文档 https://www.typescriptlang.org/tsconfig 中查阅。

- `allowJs`　一个布尔值（Boolean），指定项目是否可以导入 JavaScript 文件。

- `baseUrl`　一个字符串（string），定义解析模块路径时使用的根目录。例如，如果将其设置为 `"./"`，TypeScript 将从根目录开始解析文件导入。

- `esModuleInterop`　一个布尔值，指定 TypeScript 应当无缝地导入 CommonJS、AMD 或 UMD 模块，还是将其与 ES.Next 模块区别对待。一般来说，如果你使用的第三方库不支持 ES.Next 模块，就需要启用它。

- `forceConsistentCasingInFileNames`　一个布尔值，指定文件导入是否区分大小写。当一部分开发者使用大小写敏感的文件系统、而另一部分不是时，这项配置很重要，可以确保所有人的文件加载行为保持一致。

- `incremental`　一个字符串，定义 TypeScript 编译器是否应保存上一次编译的项目图（project graph）、使用增量类型检查（incremental type checks），并在连续构建时执行增量更新。这可以加快转译速度。

- `isolatedModules`　一个布尔值，指定 TypeScript 是否应对与第三方转译器（transpiler，如 Babel）不兼容的代码发出警告。这类警告最常见的原因是代码使用了并非模块的文件；例如，文件中没有任何 `import` 或 `export` 语句。该值不会改变实际生成的 JavaScript 的行为；它只会对无法被正确转译的代码发出警告。

- `jsx`　一个字符串，指定 TypeScript 如何处理 JSX。它只作用于 `.tsx` 文件以及 TypeScript 编译器输出这些文件的方式；例如，默认值 `react` 会使用 `React.createElement` 转换并输出代码，而 `preserve` 不会转换组件中的代码，而是原样输出。

- `lib`　一个数组（array），通过垫片（polyfill）添加缺失的语言特性。一般来说，polyfill 是一些代码片段，用于为目标环境原生不支持的特性和函数提供支持。当我们面向兼容性较差的系统（例如旧版浏览器或旧版 Node）时，需要模拟现代 JavaScript 特性。编译器会把 `lib` 数组中定义的 polyfill 添加到生成的代码中。

- `module`　一个字符串，设置转译后代码采用的模块语法。例如，如果将其设置为 `commonjs`，TSC 会把该项目转译为使用旧式 CommonJS 模块语法的代码，以 `require` 导入、以 `module.exports` 导出；而设置为 `ES2015` 时，转译后的代码将使用 `import` 和 `export` 关键字。这与 `target` 属性相互独立，后者定义的是除模块语法之外的所有可用语言特性。

- `moduleResolution`　一个字符串，指定模块解析（module resolution）策略。该策略还决定了 TSC 在编译时如何定位模块的类型定义文件。更换解析方式可以解决模块导入导出中的一些边缘问题。

- `noEmit`　一个布尔值，定义 TSC 应当生成输出文件，还是只检查项目中的类型。如果你希望由 webpack、Babel.js 或 Parcel 等第三方工具代替 TSC 来转译代码，请将其设置为 `false`。

- `resolveJsonModule`　一个布尔值，指定 TypeScript 是否导入 JSON 文件。启用后，它会根据文件内的 JSON 生成类型定义，并在导入时校验类型。我们需要手动启用 JSON 导入，因为 TypeScript 默认无法导入 JSON 文件。

- `skipLibCheck`　一个布尔值，定义 TypeScript 编译器是否对所有类型声明文件（type declaration files）执行类型检查。将其设置为 `false` 可以缩短编译时间，也是你处理无类型的第三方依赖时的逃生通道（escape hatch）。

- `target`　一个字符串，指定 TypeScript 代码应当转译到的语言特性目标。例如，如果将其设置为 `es6`（等价于 `ES2015`），TSC 会把该项目转译为兼容 ES2015 的 JavaScript，例如使用 `let` 和 `const`。
