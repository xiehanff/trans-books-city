# 第 13 章 构建 GraphQL API

*Chapter 13: Building the GraphQL API*

在本章中，我们将通过定义 schema，并为每个 query（查询）和 mutation（变更）编写 resolver（解析器），为中间件添加一个 GraphQL API。这些 resolver 将与第 12 章创建的 Mongoose 服务形成互补。查询将是公开的；不过，我们会通过 OAuth 添加一层授权，把变更暴露为受保护的 API。

与第 6 章的 GraphQL API 不同，这次我们将采用模块化的模式来实现这些 schema 和 resolver：不会把所有内容都写进一个大文件，而是把各个部分拆分到单独的文件中。就像在现代 JavaScript 中使用模块一样，这种做法的好处是把代码分解成更小的逻辑单元，每个单元都有明确的关注点。这些单元提升了代码的可读性和可维护性。

## 准备工作

我们将使用 Apollo server 创建 API 的单一入口点 /api/graphql，它通过 `@as-integrations/next` 包集成到 Next.js 中。首先，从 npm registry 安装搭建 GraphQL 所需的各个包：

```bash
$ docker exec -it foodfinder-application npm install @apollo/server graphql graphql-tag
@as-integrations/next \
```

安装完成后，在应用根目录下、middleware 文件夹旁边创建 graphql/locations 文件夹。

## schema

编写 schema 的第一步是定义查询和变更的 type definition（类型定义，即 typedef），以及 schema 中用到的所有自定义类型。为此，我们将把 schema 拆分到 graphql/locations 文件夹下的三个文件中：`custom.gql.ts`、`queries.gql.ts` 和 `mutations.gql.ts`。然后，我们会用一个普通的模板字符串（template literal）把它们合并成最终的 schema 定义。

### 自定义类型与指令

将清单 13-1 中的代码添加到 `custom.gql.ts` 文件，以定义 GraphQL 查询的 schema。

```typescript
export default `
    directive @cacheControl(maxAge: Int) on FIELD_DEFINITION | OBJECT
    type Location @cacheControl(maxAge: 86400) {
        address: String
        street: String
        zipcode: String
        borough: String
        cuisine: String
        grade: String
        name: String
        on_wishlist: [String] @cacheControl(maxAge: 60)
        location_id: String
    }
`;
```

*清单 13-1：graphql/locations/custom.gql.ts 文件*

GraphQL API 将返回来自 Mongoose schema 的地点对象。因此，我们必须定义一个表示这些地点对象的自定义类型。请创建一个自定义的 Location 类型。为了指示服务器缓存取回的值，请为整个自定义类型设置 `@cacheControl` 指令，并为 `on_wishlist` 属性设置一个更短的缓存时间，因为我们预计这个属性会频繁变化。

### 查询 schema

现在，将清单 13-2 中的代码添加到 `queries.gql.ts` 文件，以定义查询的 schema。

```typescript
export default `
    allLocations: [Location]!
    locationsById(location_ids: [String]!): [Location]!
    onUserWishlist(user_id: String!): [Location]!
`;
```

*清单 13-2：graphql/locations/queries.gql.ts 文件*

我们定义了一个包含三个 GraphQL 查询的模板字符串，它们都是我们在第 12 章为 Mongoose locations 模型实现的服务的入口点。这些名称和参数与服务中的相似，并且这些查询遵循你在第 6 章学过的 GraphQL 语法。

### 变更 schema

要定义变更 schema，请将清单 13-3 中的代码粘贴到 `mutations.gql.ts` 文件中。

```typescript
export default `
    addWishlist(location_id: String!, user_id: String!): Location!
    removeWishlist(location_id: String!, user_id: String!): Location!
`;
```

*清单 13-3：graphql/locations/mutations.gql.ts 文件*

我们使用 GraphQL 语法以模板字符串的形式创建了两个变更：一个用于向用户愿望单中添加条目，另一个用于移除条目。两者都会使用我们在 location 服务中实现的 `updateWishlist` 函数，因此它们需要 `location_id` 和 `user_id` 作为参数。

## 将类型定义合并为最终的 schema

我们已经把 location schema 拆分成两个文件，一个存放查询、一个存放变更，并把它们的自定义类型放在了第三个文件中；但要启动 Apollo server，我们需要一个统一的 schema。幸运的是，这些 typedef 只不过是模板字符串，只要使用模板字符串占位符，解析器就能把它们插值成一个完整的字符串。为此，请在 graphql 文件夹中新建 `schema.ts` 文件，并添加清单 13-4 中的代码。

```typescript
import gql from "graphql-tag";

import locationTypeDefsCustom from "graphql/locations/custom.gql";
import locationTypeDefsQueries from "graphql/locations/queries.gql";
import locationTypeDefsMutations from "graphql/locations/mutations.gql";

export const typeDefs = gql`

    ${locationTypeDefsCustom}

    type Query {
        ${locationTypeDefsQueries}
    }

    type Mutation {
        ${locationTypeDefsMutations}
    }

`;
```

*清单 13-4：graphql/schema.ts 文件*

我们从 graphql-tag 包中导入 gql 标签。尽管在使用 Apollo server 时这一步是可选的，我们仍然把 gql 标签保留在标签模板之前，以确保与其他所有 GraphQL 实现保持兼容。这还能让 IDE 产生正确的语法高亮，因为 IDE 会把类型定义静态分析为 GraphQL 标签。

接下来，我们导入实现统一 schema 所需的依赖和 schema 片段。最后，我们用 gql 函数创建一个标签模板字符串，借助模板字符串占位符把各个 schema 片段合并进 schema 骨架。我们加入自定义的 Location 类型，然后把查询的类型定义合并进 Query 对象、把变更合并进 Mutation 对象，并将这个 schema 常量导出为 typeDefs。

## GraphQL 解析器

有了 schema 之后，我们接下来处理 resolver。我们将采用类似的开发模式：把查询和变更分别写在不同的文件里，再把它们合并成 Apollo server 所需的单个文件。首先在 graphql/locations 文件夹中创建 `queries.ts` 和 `mutations.ts` 文件，然后将清单 13-5 中的代码添加到 `queries.ts`。

```typescript
import {
    findAllLocations,
    findLocationsById,
    onUserWishlist,
} from "mongoose/locations/services";

export const locationQueries = {
    allLocations: async (_: any) => {
        return await findAllLocations();
    },
    locationsById: async (_: any, param: {location_ids: string[]}) => {
        return await findLocationsById(param.location_ids);
    },
    onUserWishlist: async (_: any, param: {user_id: string}) => {
        return await onUserWishlist(param.user_id);
    },
};
```

*清单 13-5：graphql/locations/queries.ts 文件*

我们从 Mongoose 文件夹导入服务，然后创建并导出 location 查询对象。每个查询的结构都遵循第 6 章讨论过的结构。我们为每个服务各编写一个查询，查询的参数与服务中的参数一一对应。

至于变更，请将清单 13-6 中的代码添加到 `mutations.ts` 文件。

```typescript
import {updateWishlist} from "mongoose/locations/services";

interface UpdateWishlistInterface {
    user_id: string;
    location_id: string;
}

export const locationMutations = {
    removeWishlist: async (
        _: any,
        param: UpdateWishlistInterface,
        context: {}
    ) => {
        return await updateWishlist(param.location_id, param.user_id,
            "remove"
        );
    },
    addWishlist: async (_: any, param: UpdateWishlistInterface, context: {}) => {
        return await updateWishlist(param.location_id, param.user_id, "add");
    },
};
```

*清单 13-6：graphql/locations/mutations.ts 文件*

这里我们只从服务中导入了 `updateWishlist` 函数。这是因为我们把它定义成了更新文档的唯一入口，并选择使用第三个参数（取值为 add 或 remove）来区分变更应执行的两类操作。我们还创建了 `UpdateWishlistInterface`，但并不导出它；我们只在本文件内部使用它，以避免在为各个函数的 param 参数定义接口时重复编写代码。

作为变更，我们在 `locationMutations` 对象上创建了两个函数：一个用于向用户愿望单添加条目，另一个用于移除条目。两者都使用 `updateWishlist` 服务，并提供与用户想执行的操作相对应的取值。`removeWishlist` 和 `addWishlist` 这两个变更还接收第三个名为 context 的对象。目前它是一个空对象，但在第 15 章中，我们将用验证操作执行者身份所需的会话信息来替换它。

在 graphql 文件夹中创建最终的 resolver 文件 `resolvers.ts`，并把清单 13-7 中的代码添加进去。这段代码将合并变更和查询的定义。

```typescript
import {locationQueries} from "graphql/locations/queries";
import {locationMutations} from "graphql/locations/mutations";

export const resolvers = {
    Query: {
        ...locationQueries,
    },
    Mutation: {
        ...locationMutations,
    },
};
```

*清单 13-7：graphql/resolvers.ts 文件*

除了 schema 之外，我们还必须像第 6 章讨论的那样，向 Apollo server 传入一个包含所有 resolver 的对象。为此，我们需要导入查询和变更，然后用展开运算符（spread operator）把导入的对象合并进要导出的 resolvers 对象。现在，有了 schema 和 resolvers 对象，我们就可以创建 API 端点并实例化 Apollo server 了。

## 在 Next.js 中添加 API 端点

在讨论 REST 与 GraphQL API 的区别时我们曾指出：REST 中每个 API 都有自己的端点，而 GraphQL 与之不同，只提供一个端点，通常暴露为 /graphql。要创建这个端点，我们将像第 6 章那样使用 Apollo server 的 Next.js 集成。

在 pages/api 文件夹中创建 `graphql.ts` 文件，并复制清单 13-8 中的代码，它定义了 API 处理程序及其单一入口点。

```typescript
import {ApolloServer, BaseContext} from "@apollo/server";
import {startServerAndCreateNextHandler} from "@as-integrations/next";

import {resolvers} from "graphql/resolvers";
import {typeDefs} from "graphql/schema";
import dbConnect from "middleware/db-connect";

import {NextApiHandler, NextApiRequest, NextApiResponse} from "next";

const server = new ApolloServer<BaseContext>({ // 1
    resolvers,
    typeDefs,
});

const handler = startServerAndCreateNextHandler(server, { // 2
    context: async () => {
        const token = {};
        return {token};
    },
});

const allowCors = // 3
    (fn: NextApiHandler) =>
    async (req: NextApiRequest, res: NextApiResponse) => {
        res.setHeader("Allow", "POST");
        res.setHeader("Access-Control-Allow-Origin", "*");
        res.setHeader("Access-Control-Allow-Methods", "POST");
        res.setHeader("Access-Control-Allow-Headers", "*");
        res.setHeader("Access-Control-Allow-Credentials", "true");

        if (req.method === "OPTIONS") {
            res.status(200).end();
        }
        return await fn(req, res);
    };

const connectDB = // 4
    (fn: NextApiHandler) =>
    async (req: NextApiRequest, res: NextApiResponse) => {
        await dbConnect();
        return await fn(req, res);
    };

export default connectDB(allowCors(handler));
```

*清单 13-8：pages/api/graphql.ts 文件*

我们导入了创建 API 处理程序所需的全部元素：Apollo server、Apollo 与 Next.js 集成的辅助工具、我们的 resolver、GraphQL schema 文件、用于连接数据库的函数，以及 Next.js 的 API 辅助工具。

我们用 resolver 和 schema 创建了一个新的 Apollo server（标记 1）。然后使用 Next.js 集成辅助工具（标记 2）来启动 Apollo server 并返回一个 Next.js 处理程序。这个集成辅助工具采用无服务器（serverless）的 Apollo 设置，从而平滑地集成到 Next.js 的自定义服务器中，而不必自行创建服务器。此外，我们向处理程序传递了带有空 token 的 context。之后，我们就将通过这种方式获取 OAuth 流程中收到的 JWT，并把它传给 resolver。

接着，我们创建第 6 章讨论过的包装函数，用于添加 CORS 头（标记 3），并确保每次 API 调用时都有可用的数据库连接（标记 4）。我们可以放心这样做，因为我们把数据库连接设置成会返回已有的缓存连接。最后，我们导出包装后返回的异步处理程序。

在进入下一章之前，请访问 `http://localhost:3000/api/graphql` 处的 Apollo sandbox，运行几个查询来测试这个 GraphQL API。如果你看到的是天气应用的查询和变更而不是 Food Finder 的，请清除浏览器缓存并强制刷新。

## 小结

我们已经成功地把 GraphQL API 添加到了中间件中。借助本章的代码，现在可以使用 Apollo sandbox 读取和更新数据库中的值。我们还为 Apollo 处理程序提供了一个空 token，为认证做好了准备。接下来，我们就可以使用第 15 章中从 OAuth 流程获得的 JWT token 来保护 API 的变更了。但在添加认证之前，我们先来构建前端。
