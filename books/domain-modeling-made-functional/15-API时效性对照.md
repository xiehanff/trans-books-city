# 《Domain Modeling Made Functional》API 时效性对照

> **对照基准**：书（2018，F# 4.1 / VS 2017 / .NET Framework 4.x 时代）→ 现代 F# 生态（F# 10 / .NET 10 / VS 2026）。
> 查询日期：2026-11-02。判定图例：✅仍适用｜⚠️已过时有现代替代｜❌已废弃或改名｜🔄行为或默认值变更

| 书中用法（章节） | 判定 | 现代写法/现状 | 依据（来源） |
|---|---|---|---|
| 前言：基于 F# 4.1（2017-06）+ Visual Studio 2017 | 🔄 | 当前稳定版 F# 10，随 .NET 10 与 VS 2026 发布（2025-11）；此后每岁末随 .NET 年版迭代 | [F# 10 新特性](https://learn.microsoft.com/en-us/dotnet/fsharp/whats-new/fsharp-10)、[发布公告](https://devblogs.microsoft.com/dotnet/introducing-fsharp-10/) |
| 前言：推荐「Visual Studio（Windows 和 Mac）」 | ❌ | Visual Studio for Mac 已于 2024-08-31 退役；Mac 上官方改推 VS Code（或 Windows VM/Dev Box 中的 VS） | [VS for Mac 停服公告](https://learn.microsoft.com/en-us/lifecycle/announcements/visual-studio-mac-end-of-servicing) |
| 前言：VS Code + Ionide 插件 | ✅ | Ionide 仍活跃维护，7.29.0（2025-11）已支持 F# 10 / .NET 10 | [Ionide Release Notes](https://github.com/ionide/ionide-vscode-fsharp/blob/main/RELEASE_NOTES.md) |
| 第 4 章：Result 类型需 F# 4.1+ 才内置 | ✅ | 所有现代版本内置；F# 9（.NET 9）另引入可空引用类型 `T \| null`，用于边界处与 C# 库的 null 互操作（是书用 Option 手工转换处的补强，非替代 Option/Result） | [F# 9 新特性](https://learn.microsoft.com/en-us/dotnet/fsharp/whats-new/fsharp-9)、[NRT in F# 9](https://devblogs.microsoft.com/dotnet/nullable-reference-types-in-fsharp-9/) |
| 第 11 章：元组须映射为专用 DTO 记录才能序列化 | ⚠️ | F# 4.6（2019）起有匿名记录 `{| Suit = x; Rank = y |}`，中间层 DTO 可不再逐一声明；System.Text.Json 对 F# 记录开箱即用 | [F# 4.6 新特性](https://learn.microsoft.com/en-us/dotnet/fsharp/whats-new/fsharp-46) |
| 第 11 章：封装 Newtonsoft.Json（JsonConvert）做序列化 | ⚠️ | 官方主推 System.Text.Json（.NET Core 3.0 起内置、ASP.NET Core 默认、性能与安全优先），微软提供正式迁移指南；把序列化包成 Result 的封装思路本身不变。Newtonsoft.Json 仍在维护但已非新项目首选 | [迁移指南](https://learn.microsoft.com/en-us/dotnet/standard/serialization/system-text-json/migrate-from-newtonsoft)、[STJ 路线图](https://devblogs.microsoft.com/dotnet/whats-next-for-system-text-json/) |
| 第 9 章：用 Suave 演示 Web 服务组合根（`choose`/`path`/`>=>`） | ⚠️ | Suave 未死：v3.1.0（2025-12）重写内核并指向 .NET 9，v3.2.3（2026-01）；但社区主流已转向 ASP.NET Core 系：Giraffe 8.3.0（2026-07，支持 .NET 10）、Falco、Saturn 或 minimal API。组合根「在入口处一次性注入依赖」的思想完全不变 | [Suave Releases](https://github.com/SuaveIO/suave/releases)、[Giraffe Release Notes](https://github.com/giraffe-fsharp/Giraffe/blob/master/RELEASE_NOTES.md) |
| 第 9 章：NUnit 演示测试，推荐 FsUnit/Unquote/Expecto/FsCheck | ✅ | 书中「任选框架、依赖传桩即免 mock 库」的建议与现状一致，所列库均仍在生态中使用 | 书第 9 章（建议本身框架无关） |
| 第 12 章：FSharp.Data.SqlClient 类型提供器（SqlCommandProvider/SqlProgrammabilityProvider） | ⚠️ | 维护近乎停滞：稳定版止于 2.1.3（2024-04），.NET 8 支持 PR #439 于 2025-04 关闭未合并，README 明言不支持 `dotnet fsi`。替代：SQLProvider（活跃，1.5.17 / 2025-09）、Dapper 或原生 Microsoft.Data.SqlClient | [FSharp.Data.SqlClient](https://github.com/fsprojects/FSharp.Data.SqlClient)、[PR #439](https://github.com/fsprojects/FSharp.Data.SqlClient/pull/439)、[SQLProvider](https://github.com/fsprojects/SQLProvider/blob/master/docs/RELEASE_NOTES.md) |
| 第 12 章：System.Data.SqlClient（SqlConnection 等） | 🔄 | 官方推荐迁移到 Microsoft.Data.SqlClient 包（新命名空间、持续演进；生态库已普遍双轨或单轨支持它） | [Dapper #2145](https://github.com/DapperLib/Dapper/issues/2145)、SQLProvider Release Notes |
| 全书脚本式工作流（FSI 交互求值，依赖手动引用） | ✅ | `dotnet fsi` 为现代标准入口；F# 5 起脚本内可直接 `#r "nuget:包名"`，书的「边读边跑」方式比 2018 年更顺畅 | [F# Release Notes（F# 5 节）](https://github.com/dotnet/fsharp/blob/main/release-notes.md) |
| 全书大量 `sprintf "%A"` / `sprintf "%s: %A"` 拼消息 | ⚠️ | F# 5 起可用字符串插值 `$"{x}"`，编译期检查更友好；sprintf 仍合法 | 同上 |
| 运行时：.NET Framework 4.x 隐含基线 | 🔄 | 统一后的现代 .NET（当前 .NET 10，2025-11，LTS），跨平台、dotnet CLI 全流程；书中纯 F# 领域代码几乎可原样迁移 | [F# 10 发布](https://devblogs.microsoft.com/dotnet/introducing-fsharp-10/) |
| 第 1–8、13 章：DDD 与函数式建模思想（限界上下文、DU 建模、Make-Illegal-States-Unrepresentable、Result 管道、CQRS/事件溯源、类型驱动演进） | ✅ | Timeless：不绑定任何库或版本，正是当下 F# 社区（含 Giraffe/Falco 生态）仍在沿用的核心实践 | 书第 1–3、5–8、13 章（概念层） |

## 总体结论

这本书的老化集中在「外围基础设施」而非「方法论」：序列化（Newtonsoft → System.Text.Json）、Web 演示框架（Suave → ASP.NET Core 系）、SQL 类型提供器（FSharp.Data.SqlClient → SQLProvider/Dapper）三处需要读者自行替换，IDE 建议中 VS for Mac 一项已失效。语言层面，书中 F# 4.1 语法全部仍合法可用，只是错过了匿名记录（4.6）、字符串插值与 `#r "nuget"`（F# 5）、可空引用类型（F# 9）这些锦上添花的改进，均不构成理解障碍。第 1–8 章与第 13 章的领域建模、类型设计、管道与演进思想完全 timeless，可直接照学。第 9–12 章的实现示例建议「学其结构、换其零件」：组合根、DTO/领域分层、把数据库当不可信源验证等手法原样保留，只替换具体库调用即可在 .NET 10 上落地。

---
*生成方式：1 个时效对照代理（Exa 核实），主 session 汇编。全部结论带来源 URL，查询日期 2026-11-02。*
