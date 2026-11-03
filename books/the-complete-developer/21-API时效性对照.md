# 《The Complete Developer》API 时效性对照

> **对照基准**：书基于 Next.js 14 / React 18 / TS 5.x 时代（2024 出版）；生态现状核实日期 **2026-11-02**，来源为官方博客/文档/release notes。
> 判定图例：✅仍适用｜⚠️已过时有现代替代｜❌已废弃或改名｜🔄行为或默认值变更

| # | 书中用法（章节） | 判定 | 现代写法/现状 | 依据 |
|---|---|---|---|---|
| 1 | Pages Router 主路线：`pages/` + `getStaticProps`/`getServerSideProps` + `pages/api/`（第 5、7、12、14 章） | ⚠️ 仍可用但已非主线 | Pages Router 未废弃、仍随 Next.js 16 发布且无移除日期；但新功能全在 App Router，新项目默认用 async Server Component + `fetch` + Route Handlers（`route.ts`） | [Next 16 博客](https://nextjs.org/blog/next-16)、[官方迁移指南](https://nextjs.org/docs/pages/guides/migrating/app-router-migration) |
| 2 | `npx next export` / `npm run export` 静态导出（第 5 章） | ❌ 已移除 | Next 13.3 弃用、**14.0 起移除**（书自用版本即报错），改为 `next.config` 中 `output: 'export'`；已列入勘误清单 #10 | [Static Exports 版本历史](https://nextjs.org/docs/pages/guides/static-exports) |
| 3 | `npx create-next-app@latest --typescript`（第 5 章） | 🔄 输出已变 | 现在生成 Next 16 + App Router + Turbopack 样板，不再有 `pages/`；需加 `--pages` 标志才能还原书中练习环境 | [Next 16.4 博客](https://nextjs.org/blog/next-16-4) |
| 4 | 附录 B：App Router 初览，含 `{/* @ts-expect-error Async Server Component */}` | 🔄 类型限制已消失 | Next 15+/React 19 下 async Server Component 原生支持，无需 `@ts-expect-error`；App Router 已从"实验性附录"变为唯一默认路线 | [React 19 发布](https://react.dev/blog/2024/12/05/react-19)、[Next 16.4](https://nextjs.org/blog/next-16-4) |
| 5 | `next/head` + `pages/_app.tsx`（第 5 章） | ⚠️ 仅 Pages Router 内有效 | App Router 用 `metadata` 导出/`generateMetadata`，`_app.tsx` 被 `app/layout.tsx` 取代 | 官方迁移指南 |
| 6 | React 18 基础：`createRoot`、`useState`/`useEffect`、虚拟 DOM 模型（第 4 章） | ✅ 仍适用 | React 最新 19.3（2026-09），内置 hooks 语义未变；React 19 主要是 RSC 稳定化 + Actions/use 增量特性 | [react.dev/versions](https://react.dev/versions)、[React 19](https://react.dev/blog/2024/12/05/react-19) |
| 7 | 手动性能优化语境（useMemo/useCallback，第 4 章） | 🔄 方向性变化 | React Compiler v1.0（2025-10）自动记忆化，官方鼓励默认启用；手动 memoization 定位为补充 | [React Compiler v1.0](https://react.dev/blog/2025/10/07/react-compiler-1) |
| 8 | "安装最新 Node.js LTS"（第 1 章，书时代 18/20） | 🔄 仅版本号过时 | 现役：Node 26 "Lithium" Active LTS（2026-10 起），24/22 Maintenance；Node 20 已 EOL；建议本身仍正确 | [nodejs/Release 时刻表](https://github.com/nodejs/Release) |
| 9 | Mongoose connect/CRUD + mongodb-memory-server（第 7、12 章） | ✅ 仍适用 | Mongoose 9.0（2025-11）为当前大版本；其破坏性变更（回调式 pre 移除等）不触及书中基础用法 | [Mongoose 9.0](https://github.com/Automattic/mongoose/releases/tag/9.0.0)、[版本支持](https://mongoosejs.com/docs/version-support.html) |
| 10 | Jest + ts-jest + 附录 C 匹配器（第 8、16 章） | ✅ 仍适用 | Jest 30（2025-06）下 ts-jest 流程与核心匹配器不变；注意 30 要求 TS 5.4+、个别 CLI 改名 | [Jest 30 博客](https://jestjs.io/blog/2025/06/04/jest-30) |
| 11 | `docker compose up`（v2 语法）+ `mongo:latest`（第 10、11、16 章） | ✅ 仍适用 | Compose v2 语法正确；建议把 `mongo:latest` 固定为 `mongo:8` 之类 tag 避免 future break（latest 现指 8.3） | [Docker Hub mongo](https://hub.docker.com/_/mongo/) |
| 12 | TypeScript 5.x + 附录 A tsconfig（第 3 章、附录 A） | ✅ 仍适用（一处细节） | TS 语言特性在 6.0（2026-03）与 7.0（Go 原生版，2026-08 GA）中完全兼容；仅附录 A "`jsx` 默认 `react` 用 createElement"已不符合现代实践（现用 `preserve`/`react-jsx`） | [TS 6.0](https://devblogs.microsoft.com/typescript/announcing-typescript-6-0/)、[TS releases](https://github.com/microsoft/typescript/releases) |
| 13 | next-auth v4：`[...nextauth].ts` + `useSession`/`signIn`（第 9、15 章） | ✅ 可运行，⚠️ 项目进入维护模式 | `npm install next-auth` 仍装 v4（4.24.15，2026-07 仍发安全补丁），书内用法可直接运行；但 Auth.js 2025-09 并入 Better Auth 团队进入维护模式，v5 永久停在 beta，新项目官方推荐 Better Auth | [Auth.js 公告](https://github.com/nextauthjs/next-auth/discussions/13252)、[2026-07 安全更新](https://better-auth.com/blog/security-update-july-2026) |
| 14 | 概念性内容：REST/GraphQL 设计、OAuth 流程、容器思想、业务中间件分层（第 6、9、10、12 章） | ✅ 稳定可学 | 概念与设计思想基本未变；唯一相关改名是 Next.js 的 `middleware.ts`→`proxy.ts`（Next 16，弃用非移除），但第 12 章讲的是应用内业务胶水层，不受影响 | [Next 16 博客](https://nextjs.org/blog/next-16) |

## 总体结论

这本书 2024 年出版、基于 Next.js 14/React 18，距今生态只演进了一代半（Next 16 / React 19.3），老化程度"中等偏轻"：14 项核查中仅 1 项硬废弃（`next export`，勘误已捕获），2 项方向性变化（Pages Router 让位 App Router、React Compiler），其余核心 API 用法均可运行。可直接学的占大头：React hooks/组件模型、TypeScript、Mongoose CRUD、Jest、Docker Compose、OAuth 概念、REST/GraphQL 思想。需要重点补丁的只有一处主线差异——第 5 章的 Pages Router 主路线应按"历史路线 + 附录 B 现代路线"来讲，读者动手时建议 `create-next-app --pages` 还原环境或直接以附录 B 为准。另有两个生态级提示值得加译注：next-auth 已进入维护模式（新项目推荐 Better Auth），Node.js 应装 26/24 而非书中时代的 18/20。

---
*生成方式：1 个时效对照代理（Exa 核实 23 次检索），主 session 汇编。全部结论带来源 URL，查询日期 2026-11-02。*
