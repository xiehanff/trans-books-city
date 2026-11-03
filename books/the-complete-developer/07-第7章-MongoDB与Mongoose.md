# 第 7 章 MongoDB 与 Mongoose

*Chapter 7: MongoDB and Mongoose*

大多数应用程序都依赖数据库管理系统（database management system，简称数据库）来组织数据集并授予其访问权限。在本章中，你将使用 MongoDB 这一非关系型数据库（non-relational database），以及与之配套的对象映射器（object mapper）Mongoose。

由于 MongoDB 以 JSON 形式返回数据，并且使用 JavaScript 编写数据库查询，它对全栈 JavaScript 开发者来说是一种自然而然的选择。在接下来的几节中，你将学习如何创建 Mongoose 模型（model），通过它查询数据库、简化与 MongoDB 的交互，并编写连接前端与后端数据库的中间件（middleware）。你还将编写服务函数，在数据库上实现四种 CRUD 操作。

在第 125 页的练习 7 中，你将为第 6 章创建的 GraphQL API 添加一个数据库，替换其当前使用的静态数据存储。

## 应用程序如何使用数据库和对象关系映射器

应用程序需要一个数据库来存储和操作数据。到目前为止，本书中的应用程序 API 只返回保存在文件里、无法更改的预定义数据集。我们通过请求中的参数向数据集添加内容，但无法在不同 API 调用之间保存数据（称为持久化数据，persisting the data）。例如，如果想更新应用程序的天气信息，我们就需要一个数据库来持久化数据，这样下一次 API 调用才能读取它。在全栈开发中，我们通常用数据库存储与用户相关的数据。数据库的另一个例子是你的邮件客户端用来存储邮件的数据库。

要使用数据库，我们首先需要连接到它并完成身份验证。一旦能够访问数据，我们就可以执行查询（query）来请求特定的数据集。查询返回的结果包含应用程序可以显示或以其他方式使用的数据。这些步骤在实践中如何执行，取决于所使用的具体数据库。

直接使用数据库自身的 API 查询数据往往很笨拙，因为即使只是建立并维护连接，通常也需要大量样板代码。因此，我们经常使用对象关系映射器（object-relational mapper，ORM）或对象数据建模工具（object data modeling tool，ODM），通过抽象部分细节来简化数据库的使用。例如，MongoDB 的对象数据建模工具 Mongoose 会替我们处理数据库连接，省去了每次交互时检查数据库连接是否打开的麻烦。

Mongoose 还让我们更容易应对 MongoDB 运行在独立数据库服务器上这一事实。使用分布式系统需要进行异步调用，你在第 2 章已经学过。借助 Mongoose，我们可以用面向对象的 async/await 接口访问数据，而不必使用笨重的回调函数。

此外，MongoDB 是无模式的（schema-less）：它不要求我们预先定义并严格遵循模式（schema）。这种灵活性虽然方便，却也是常见的错误来源，尤其是在大型应用程序或开发者流动频繁的项目中。第 3 章讨论过用 TypeScript 为 JavaScript 添加类型的好处。Mongoose 会以类似的方式为 MongoDB 的数据模型提供类型并校验其完整性，你将在第 118 页的“定义 Mongoose 模型”一节中看到这一点。

## 关系型与非关系型数据库

数据库可以用多种方式组织数据，这些方式可归为两大类：关系型（relational）与非关系型（non-relational）。MySQL 和 PostgreSQL 等关系型数据库将数据存储在一张或多张表中。你可以把这些数据库想象成 Excel 电子表格：与 Excel 一样，每张表都有一个唯一的名称，并包含列和行。列定义了属性（例如数据类型），适用于存储在该列中的所有数据；行则包含实际的数据集，每个数据集都由一个唯一的 ID 标识。关系型数据库使用结构化查询语言（Structured Query Language，SQL）的某种变体来执行数据库操作。

MongoDB 是一种非关系型数据库。与传统的关系型数据库不同，它以 JSON 文档（document）而非表的形式存储数据，且不使用 SQL。非关系型数据库有时也被称为 NoSQL，能够以多种不同的格式存储数据。例如，流行的 NoSQL 数据库 Redis 和 Memcached 采用键值（key-value）存储，这使它们具备极高的性能和良好的可扩展性，因此常被用作内存缓存。另一个 NoSQL 数据库 Neo4j 是图数据库，它利用图论将数据存储为节点，这个概念我们在第 6 章提到过。这些只是非关系型数据库的几个例子。

MongoDB 是使用最广泛的文档数据库（document database）；它用集合（collection）、文档和字段（field）来组织数据，而不是表、行和列。字段是数据库中最小的单位，它定义数据类型及其他属性，并包含实际的数据。你可以把它大致等同于 SQL 表中的一列。文档由字段组成，类似于 SQL 表中的行，我们有时也称其为记录（record）；MongoDB 使用 BSON（JSON 对象的二进制表示形式）来存储文档。集合大致相当于 SQL 表，但它聚合的是文档，而不是行和列。

由于非关系型数据库能以不同的格式存储数据，每种数据库都使用各自特定的、经过优化的查询语言来执行 CRUD 操作。这些低层 API 关注的是访问和操作数据，而不一定关注开发者体验。相比之下，对象关系映射器提供了高层抽象，为查询语言提供了简洁而简化的接口。因此，虽然 MongoDB 有自己的 MongoDB 查询语言（MongoDB Query Language，MQL），我们仍将使用 Mongoose 来访问它。

## 搭建 MongoDB 与 Mongoose

在开始使用 MongoDB 和 Mongoose 之前，必须先把它们添加到示例项目中。为简单起见，我们将使用 MongoDB 的内存实现（in-memory implementation），而不是在自己的机器上安装并维护一个真正的数据库服务器。这对测试本章的示例来说是合适的，但不适用于部署真正的应用程序，因为它不会在重启之间持久化数据。在第二部分构建 Food Finder 应用程序时，你将获得搭建真实 MongoDB 服务器的经验。第 11 章将向你展示如何使用一个包含 MongoDB 服务器的预构建 Docker 容器。

在第 6 章重构后的 Next.js 应用程序的根目录下运行以下命令：

```bash
$ npm install mongodb-memory-server mongoose
```

然后在根目录中、package.json 文件旁边创建两个新文件夹：一个用于存放 Mongoose 代码，命名为 mongoose，其中包含子文件夹 weather；另一个命名为 middleware，用于存放所需的中间件。

## 定义 Mongoose 模型

为了校验数据的完整性，我们必须创建一个基于模式的 Mongoose 模型，它充当与数据库中某个 MongoDB 集合直接交互的接口。所有与数据库的交互都将通过该模型进行。不过在创建模型之前，我们需要先创建模式本身，它定义数据库数据的结构，并将 Mongoose 实例映射到集合中的文档。

我们的 Mongoose 模式将与第 6 章为 GraphQL API 创建的模式保持一致。这是因为我们将在第 125 页的练习 7 中把 GraphQL API 连接到数据库，用从数据库查询到的数据集替换静态 JSON 对象。

### 接口

在用 TypeScript 编写 Mongoose 模型和模式之前，先声明一个 TypeScript 接口。如果没有匹配的接口，我们将无法为 TSC 提供模型或模式的类型，代码也就无法编译。将代码清单 7-1 所示的代码粘贴到 mongoose/weather/interface.ts 文件中。

```typescript
export declare interface WeatherInterface {
    zip: string;
    weather: string;
    tempC: string;
    tempF: string;
    friends: string[];
};

```

*代码清单 7-1：Mongoose 天气模型的接口*

这段代码是一个普通的 TypeScript 接口，其属性与 GraphQL 模式和 Mongoose 模式一致。

### 模式

代码清单 7-2 展示了 Mongoose 模式。它的顶层属性代表文档中的字段。每个字段都有一个类型，以及一个表示该字段是否必填的标志。字段还可以带有其他可选属性，例如自定义或内置的校验器（validator）。这里我们使用内置的 required 校验器；其他常用的内置校验器还有用于字符串的 minlength 和 maxlength，以及用于数字的 min 和 max。将代码添加到 mongoose/weather/schema.ts 文件中。

```typescript
import {Schema} from "mongoose";
import {WeatherInterface} from "./interface";

export const WeatherSchema = new Schema<WeatherInterface>({
    zip: {
        type: "String",
        required: true,
    },
    weather: {
        type: "String",
        required: true,
    },
    tempC: {
        type: "String",
        required: true,
    },
    tempF: {
        type: "String",
        required: true,
    },
    friends: {
        type: ["String"],
        required: true,
    },
});

```

*代码清单 7-2：Mongoose 天气模型的模式*

我们把一个对象传给模式构造函数来创建模式，并将 WeatherInterface 设置为它的 SchemaType。因此，我们需要从 mongoose 包中导入 Schema 函数，并导入之前创建的接口。

与为 JavaScript 添加自定义类型的 TypeScript 类似，Mongoose 会把每个属性转换为与之关联的 SchemaType，后者提供了模型的配置。可用的类型既包括内置的 JavaScript 类型（如 Array、Boolean、Date、Number 和 String），也包括自定义类型（如 Buffer 和 ObjectId）；其中 ObjectId 指向 Mongoose 在创建每个文档时自动添加的默认唯一属性 _id。它类似于你可能熟悉的关系型数据库中的主键（primary key）。

我们在第 6 章创建的天气 API 返回一个包含四个属性的对象：zip、weather、tempC 和 tempF，它们都是字符串。此外，friends 属性中还有一个字符串数组。在这个模式中，我们定义了相同的属性，然后导出该模式。

### 模型

有了模式之后，我们就可以创建 Mongoose 模型了。这个模式的包装器将为所有 CRUD 操作提供对集合中 MongoDB 文档的访问。我们把模型写在 mongoose/weather/model.ts 文件中，其代码见代码清单 7-3。请记住，此时我们还没有把它连接到服务器上的 MongoDB 数据库。

```typescript
import mongoose, {model} from "mongoose";
import {WeatherInterface} from "./interface";
import {WeatherSchema} from "./schema";

export default mongoose.models.Weather ||
    model<WeatherInterface>("Weather", WeatherSchema);

```

*代码清单 7-3：Mongoose 天气模型*

首先，我们从 mongoose 包导入 Mongoose 模块和 model 构造函数，以及之前创建的接口和模式。然后，我们设置 Weather 模型，并用 WeatherInterface 为其提供类型。我们传入两个参数：模型的名称 Weather，以及定义模型内部数据结构的模式。Mongoose 会把新建的模型绑定到 MongoDB 实例的集合上。Weathers 集合位于 Weather 数据库中，两者都由 Mongoose 创建。注意，在创建新模型之前，我们需要检查 mongoose.models 上是否已存在 Weather 模型，否则 Mongoose 会抛出错误。我们导出这个模型，以便在后续模块中使用它。

### 数据库连接中间件

到目前为止，本书已多次提到，全栈开发涵盖应用程序的前端、后端和中间件，后者通常也被称为“应用粘合剂”。现在是时候创建我们的第一个专门的中间件了。

这个中间件将打开与数据库的连接，然后使用 Mongoose 的异步辅助函数来维护该连接。接着，它会把 Mongoose 的模型映射到 MongoDB 集合，让我们能够通过 Mongoose 访问它们。方便的是，这个连接辅助函数会缓冲操作，并在必要时重新连接数据库，因此我们无需自己处理连接问题。将代码清单 7-4 中的代码粘贴到 middleware/db-connect.ts 文件中。

```typescript
import mongoose from "mongoose";
import {MongoMemoryServer} from "mongodb-memory-server";

async function dbConnect(): Promise<any | String> {
    const mongoServer = await MongoMemoryServer.create();
    const MONGOIO_URI = mongoServer.getUri();
    await mongoose.disconnect();
    await mongoose.connect(MONGOIO_URI, {
        dbName: "Weather"
    });
}

export default dbConnect;

```

*代码清单 7-4：Mongoose 中间件*

我们导入了 mongoose 包和 mongodb-memory-server 数据库。我们定义并导出的 async 函数 dbConnect 通过 mongoose.connect 函数管理与数据库服务器的连接。如前所述，我们创建 MongoMemoryServer 的实例，把数据保存在内存中，而不是使用真正的数据库服务器。然后，我们把连接字符串存储在常量 MONGOIO_URI 中。由于我们使用的是内存服务器，这个字符串是动态的；而对于远程数据库，它将是一个表示数据库服务器地址的静态字符串。接着，我们关闭所有现有连接，并使用 Mongoose 打开一个新连接。Mongoose 模型已经完成映射并可供使用，我们准备好执行第一次查询了。

## 查询数据库

现在是时候编写数据库查询了。你不应该把这些查询散落在应用程序代码中，或者直接写在 GraphQL 解析器（resolver）里，而应该把它们抽取为服务（service）。

服务是一个函数，它执行 Mongoose 模型上实际的 CRUD 操作并返回结果。每个 GraphQL 解析器随后都可以调用一个服务函数，所有内部数据库访问都应该通过这些函数进行。此外，每个服务应该只负责某一个特定的 CRUD 操作。Mongoose 会自动将命令排队并执行、维护连接，并在与数据库建立连接后立即处理队列。

本节介绍服务函数和基本的 Mongoose 命令。但这并不是一份完整的参考手册。当你开始在自己的项目中使用 Mongoose 时，请到 Mongoose 文档中查阅所需的全部函数。

### 创建文档

第一个也是最基础的操作是“创建”（create）操作。它的名称恰好是 mongoose.create，而且幸运的是，我们既可以用它创建数据集，也可以用它更新数据集。这是因为当数据库条目（也就是文档）尚不存在时，Mongoose 会自动创建一个新的数据库条目。因此，我们不需要先检查某个数据集是否存在，再有条件地创建它，然后再更新。

代码清单 7-5 展示了一个把数据集存储到数据库中的服务函数的基本实现。将代码放入 mongoose/weather/services.ts 文件。

```typescript
import WeatherModel from "./model";
import {WeatherInterface} from "./interface";

export async function storeDocument(doc: WeatherInterface): Promise<boolean> {
    try {
        await WeatherModel.create(doc);
    } catch (error) {
        return false;
    }
    return true;
}

```

*代码清单 7-5：通过 Mongoose 创建文档*

为了存储文档，我们创建并导出 async 函数 storeDocument，它以数据集作为参数，这里将其类型定为 WeatherInterface。然后，我们在模型上调用 create 函数，并把数据集传给它。该函数会在 WeatherModel（即 MongoDB 实例中的天气集合）中创建并插入文档。最后，它返回一个布尔值来指示操作的状态。

### 读取文档

为了实现“读取”（read）操作，我们通过 Mongoose 的 findOne 函数查询 MongoDB。它接受一个参数——包含要查找属性的对象——并返回第一个匹配项。将代码清单 7-6 中的代码添加到 mongoose/weather/services.ts 文件中。它定义了一个 findByZip 函数，用于从 Weathers 集合中查找并返回第一个 zip 属性与作为参数传入的邮政编码相匹配的文档。

```typescript
export async function findByZip(
    paramZip: string
): Promise<Array<WeatherInterface> | null> {
    try {
        return await WeatherModel.findOne({zip: paramZip});
    } catch (err) {
        console.log(err);
    }
    return [];
}

```

*代码清单 7-6：通过 Mongoose 读取数据*

我们在 services.ts 文件的服务中添加并导出 async 函数 readByZip。该函数接受一个字符串参数（邮政编码），返回一个包含文档的数组或空数组。在这个新的服务函数内部，我们在模型上调用 Mongoose 的 findOne 函数，并传入一个筛选对象，查找 zip 字段与参数值匹配的文档。最后，该函数返回结果或 null。

### 更新文档

前面提到过，我们可以使用 create 函数来更新文档。不过，这项任务还有一个专门的 API：updateOne。它接受两个参数：第一个是筛选对象，类似于我们在 findOne 中使用的筛选器；第二个是包含新值的对象。你可以把 updateOne 看作“查找”和“创建”两个函数的组合。将代码清单 7-7 中的代码添加到 mongoose/weather/services.ts 文件中。

```typescript
export async function updateByZip(
    paramZip: string,
    newData: WeatherInterface
): Promise<boolean> {
    try {
        await WeatherModel.updateOne({zip: paramZip}, newData);
        return true;
    } catch (err) {
        console.log(err);
    }
    return false;
}

```

*代码清单 7-7：通过 Mongoose 更新数据*

我们添加到服务中的 updateByZip 函数接受两个参数。第一个是字符串 paramZip，即我们用来查询待更新文档的邮政编码。第二个参数是新数据集，我们将其类型定为 WeatherInterface。我们在模型上调用 Mongoose 的 updateOne 函数，向它传入筛选对象和最新数据。该函数应返回一个布尔值来指示状态。

### 删除文档

我们要实现的最后一个 CRUD 操作是一个删除文档的服务。为此，我们使用 Mongoose 的 deleteOne 函数，并将代码清单 7-8 中的代码添加到 mongoose/weather/services.ts 文件中。它类似于 findOne 函数，只不过会直接删除查询的结果。Mongoose 会将操作排队，并在有可用连接时自动从数据库中删除该文档。

```typescript
export async function deleteByZip(
        paramZip: string
    ): Promise<boolean> {
    try {
        await WeatherModel.deleteOne({zip: paramZip});
        return true;
    } catch (err) {
        console.log(err);
    }
    return false;
}

```

*代码清单 7-8：通过 Mongoose 删除数据*

async 函数 deleteByZip 接受一个字符串参数 zip。我们用它查询模型并找到要删除的文档，把筛选器传给 Mongoose 的 deleteOne 函数。该函数应返回一个布尔值。

## 创建端到端查询

在全栈开发中，端到端（end-to-end）通常指数据能够从应用程序的前端（或它的某个 API）出发，途经中间件到达后端，然后再一路返回其原始来源的能力。作为练习，让我们使用 REST API 的 /zipcode 端点创建一个简单的端到端示例。

我们将修改这个 API，让它从 URL 中获取查询参数，在数据库中查找所请求邮政编码对应的天气对象，然后把它返回，实际上就是用动态查询结果取代静态 JSON 响应。修改 pages/api/v1/weather/[zipcode].ts 文件，使其与代码清单 7-9 一致。

```typescript
import type {NextApiRequest, NextApiResponse} from "next";
import {findByZip} from "./../../../../mongoose/weather/services";
import dbConnect from "./../../../..//middleware/db-connect";
dbConnect();

export default async function handler(
    req: NextApiRequest,
    res: NextApiResponse
): Promise<NextApiResponse<WeatherDetailType> | void> {
    let data = await findByZip(req.query.zipcode as string);
    return res.status(200).json(data);
}

```

*代码清单 7-9：端到端的 REST API*

注意这个修改后的 API 处理程序。我们对它做了两处重大修改。首先，我们调用 dbConnect 来连接数据库。然后，我们使用导入的 findByZip 服务，并把查询参数转换为字符串类型后传给它。与之前的静态 JSON 对象不同，我们现在返回从服务函数接收到的动态数据。

在 API 调用能够返回数据之前，我们还需要执行一个步骤：填充数据库（seeding），即向其中添加初始数据集。为简单起见，我们使用 storeDocuments 服务，直接在 dbConnect 函数中进行填充。修改 middleware/db-connect.ts 文件，使其与代码清单 7-10 中的代码一致；该清单导入了 storeDocument 服务，并在建立数据库连接后添加数据集。

```typescript
import mongoose from "mongoose";
import {MongoMemoryServer} from "mongodb-memory-server";
import {storeDocument} from "../mongoose/weather/services";

async function dbConnect(): Promise<any | String> {
    const mongoServer = await MongoMemoryServer.create();
    const MONGOIO_URI = mongoServer.getUri();
    await mongoose.disconnect();

    let db = await mongoose.connect(MONGOIO_URI, {
        dbName: "Weather"
    });

    await storeDocument({
        zip: "96815",
        weather: "sunny",
        tempC: "25C",
        tempF: "70F",
        friends: ["96814", "96826"]
    });
    await storeDocument({
        zip: "96814",
        weather: "rainy",
        tempC: "20C",
        tempF: "68F",
        friends: ["96815", "96826"]
    });
    await storeDocument({
        zip: "96826",
        weather: "rainy",
        tempC: "30C",
        tempF: "86F",
        friends: ["96815", "96814"]
    });

}
export default dbConnect;

```

*代码清单 7-10：dbConnect 函数中的朴素数据填充*

现在我们可以执行端到端请求了。在浏览器中访问 REST API 端点 http://localhost:3000/api/v1/weather/96815。你应该会看到来自 MongoDB 数据库的数据集作为 API 响应返回。试着把 URL 中的查询参数改成另一个有效的邮政编码，你应该会在响应中得到另一个数据集。

## 练习 7：将 GraphQL API 连接到数据库

让我们改造天气应用程序的 GraphQL API，使它从数据库而不是静态 JSON 文件中读取响应数据。代码看起来会很眼熟，因为我们将使用与上一节 REST API 示例相同的模式。

首先，确认你已经把 MongoDB 内存实现和 Mongoose 添加到项目中。如果没有，请按照第 117 页“搭建 MongoDB 与 Mongoose”一节的说明添加它们。接下来，检查你是否已经创建了本章所述的 middleware 和 mongoose 文件夹中的文件，并且其中包含代码清单 7-1 到 7-10 的代码。

现在，要把 GraphQL API 连接到数据库，我们需要做两件事：实现数据库连接，并重构 GraphQL 解析器以使用数据库中的数据集。

### 连接到数据库

要通过 GraphQL API 查询数据库，我们需要先建立与数据库的连接。正如你在第 6 章学到的，所有 API 调用都使用同一个端点 /graphql。这一点现在会给我们带来极大的便利：由于所有请求都经过同一个入口，我们只需要处理一次数据库连接。因此，我们打开 api/graphql.ts 文件，将其修改为与代码清单 7-11 一致。

```typescript
import {ApolloServer} from "@apollo/server";
import {startServerAndCreateNextHandler} from "@as-integrations/next";
import {resolvers} from "../../graphql/resolvers";
import {typeDefs} from "../../graphql/schema";
import {NextApiHandler, NextApiRequest, NextApiResponse} from "next";
import dbConnect from "../../middleware/db-connect";
//@ts-ignore
const server = new ApolloServer({
    resolvers,
    typeDefs
});

const handler = startServerAndCreateNextHandler(server);

const allowCors = (fn: NextApiHandler) =>
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

const connectDB = (fn: NextApiHandler) =>
    async (req: NextApiRequest, res: NextApiResponse) => {
        await dbConnect();
        return await fn(req, res);
    };

export default connectDB(allowCors(handler));

```

*代码清单 7-11：包含数据库连接的 api/graphql.ts 文件*

我们对这个文件做了三处修改。首先，我们从中间件导入了 dbConnect 函数；然后，我们创建了一个类似于 allowCors 函数的新包装器，并用它确保每次 API 调用都会连接到数据库。我们可以放心这样做，因为我们实现的 dbConnect 在同一时刻只会保留一个数据库连接。最后，我们用新包装器包装了 handler，并将其作为默认导出。

### 将服务添加到 GraphQL 解析器

现在是把服务添加到解析器的时候了。在第 6 章中你了解到，查询（query）解析器负责实现数据的读取，而变更（mutation）解析器负责实现数据的创建、更新和删除。

在那一章中，我们还定义了两个解析器：一个返回给定邮政编码对应的天气对象，另一个更新某个位置的天气数据。现在，我们把本章创建的服务 findByZip 和 updateByZip 添加到解析器中。我们将修改这两个解析器，让它们通过服务查询和更新 MongoDB 文档，取代使用静态数据对象的朴素实现。

代码清单 7-12 展示了 graphql/resolvers.ts 文件修改后的代码，我们在其中重构了这两个解析器。

```typescript
import {WeatherInterface} from "../mongoose/weather/interface";
import {findByZip, updateByZip} from "../mongoose/weather/services";

export const resolvers = {
    Query: {
        weather: async (_: any, param: WeatherInterface) => {
            let data = await findByZip(param.zip);
            return [data];
        },
    },
    Mutation: {
        weather: async (_: any, param: {data: WeatherInterface}) => {
            await updateByZip(param.data.zip, param.data);
            let data = await findByZip(param.data.zip);
            return [data];
        },
    },
};

```

*代码清单 7-12：使用服务的 graphql/resolvers.ts 文件*

我们用相应的服务取代了朴素的 array.filter 功能。为了查询数据，我们使用 findByZip 服务，把请求载荷中的 zip 变量传给它，然后把结果数据包装在数组中返回。对于变更操作，我们使用 updateByZip 服务。按照类型定义，weather 变更要返回更新后的数据集。为此，我们再次使用 findByZip 服务查询被修改的文档，并把结果作为数组项返回。

访问 GraphQL 沙盒 http://localhost:3000/api/graphql，试用这些 API 端点，从 MongoDB 数据库中读取和更新文档。

## 总结

在本章中，你学习了如何使用非关系型数据库 MongoDB 及其对象数据建模工具 Mongoose，它让你能够添加并强制执行模式，还可以对 MongoDB 实例执行 CRUD 操作。我们介绍了关系型数据库与非关系型数据库的差异，以及它们各自如何存储数据。随后，你创建了 Mongoose 模式和模型，把 Mongoose 连接到 MongoDB 实例，并编写了在 MongoDB 集合上执行操作的服务。

最后，你把 REST 和 GraphQL API 都连接到了 MongoDB 数据库。现在，所有 API 返回的都不再是静态数据集，而是动态文档，并且你既可以通过它们读取文档，也可以更新文档。

MongoDB 和 Mongoose 都是功能庞大的技术，拥有海量的功能。想进一步了解它们，可以查阅官方文档 https://mongoosejs.com，并阅读 https://www.geeksforgeeks.org/mongoose-module-introduction/ 上的文章。

下一章将介绍 Jest，这是一个现代化的测试框架，可用于进行单元测试、快照测试和集成测试。
