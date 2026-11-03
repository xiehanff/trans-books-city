# 第 6 章 REST 与 GraphQL API

*Chapter 6: REST and GraphQL APIs*

API 是一种用来连接计算机或计算机程序的通用模式。与用户界面不同，API 的设计初衷不是供用户访问，而是供另一段软件访问。API 的目的之一是隐藏系统内部运作的细节，同时对外暴露一个访问系统数据或功能的标准入口。

作为全栈开发者，你通常会与两类 API 打交道，或者说“消费”两类 API：内部 API 和第三方 API。查询内部 API 时，你消费的是来自自己系统的数据，通常来自自己的数据库或服务。私有 API 不对外部各方开放。例如，你的银行可能会用私有 API 在其内部系统中查询你的信用评分或账户余额，并显示在你的网上银行资料页中。

第三方 API 提供对外部系统数据的访问。例如，你将在第二部分实现的 OAuth 登录就使用了 API。你可能还会用 API 从外部提供商获取社交媒体动态或天气信息，显示到自己的网站上。由于外部 API 面向公众开放，你可以通过公开的 URL 访问它们，而这些 API 会在一份 API 契约（API contract）中写明访问其数据应遵循的约定。这份契约定义了通信的格式、API 期望的参数，以及每个请求可能收到的响应。我们在第 3 章简要讨论过 API 契约与函数契约，以及为什么要为它们标注类型。

全栈 Web 开发主要使用两种类型的 API：REST（表述性状态转移）和 GraphQL，二者都通过 HTTP 传输数据。本章将介绍它们。

## REST API

REST 是一种用来设计 RESTful Web API 的架构模式。这类 API 本质上是一组 URL，其中每个 URL 提供对单一资源的访问。它们依赖 HTTP 方法（HTTP method）和标准 HTTP 状态码（status code）来传输数据，并接受 URL 编码的参数或请求头参数。通常，它们以 JSON 或纯文本的形式返回所请求的数据。

事实上，你已经构建了自己的第一个 REST API。回想一下你在第 89 页的练习 5 中创建的 Next.js 服务器，它提供了 api/weather/:zipcode 这个端点（endpoint）。到目前为止，我们一直用这个端点来体验 Next.js 的路由、理解动态 URL，并学习如何访问查询参数。然而你很快会看到，这个 API 正遵循着 REST 约定：我们使用 HTTP GET 方法访问该 URL 端点，并收到了带有 HTTP 状态码 200: OK 的 JSON 响应。常见的状态码区间有：2XX 表示请求成功，3XX 表示重定向。如果请求失败，我们会看到表示客户端相关错误的 4XX 区间，例如 401: Unauthorized，以及表示服务器错误的 5XX 区间，最常见的是笼统的 500: Internal Server Error。

作为全栈开发者，我们有时会创建自己的 API；但更常见的情况是消费第三方 API。比如说，我们为什么要消费一个第三方天气 API 呢？设想我们想让应用显示多个远程地点的当前天气。如果靠自己搭建并维护各种气象站、再从传感器读取数据，就要为每个气象站既提供又消费 API；与之相比，我们可以直接消费现有天气服务提供的第三方 API。我们的代码可以调用该 API，传入一个邮政编码作为参数，然后以预定格式收到该地点的天气数据，再把这些数据显示到网站上。

RESTful API 让我们无需了解数据是如何存储的、底层用了什么技术，就能与数据交互。只要遵循某个 API 的规范，即便底层技术或架构发生变化，你也应当能收到所请求的数据。除此之外，一个 API 要被视为 RESTful 的，还需满足若干要求。

### URL

一个唯一的 URL 为 RESTful API 提供了接口。提供商的各个 API 通常共享同一个基础 URL，称为根入口点（root entry point），例如 http://localhost:3000/api。你可以把它看作这些 API 的“家族姓氏”。根入口点后面常常会加上版本号，因为提供商可能同时维护一个 API 的多个版本。例如，可能有一个旧的 http://localhost:3000/api/v1 和一个更新后的 http://localhost:3000/api/v2。为了遵循这一模式，你可以在 api 文件夹里创建一个 v1 文件夹，并把 REST API 的代码移进去。

> **NOTE**
>
> 其他常见的 API 版本管理方式还包括自定义请求头和查询字符串。前一种方式下，客户端会带上自定义的 Accept-Version 请求头发起 API 请求，并收到匹配的 Content-Version 请求头。后一种方式下，API 请求会在 URL 中使用 ?version=1.0.0 这样的查询参数。

URL 的下一部分是路径（path），通常称为端点。它指定了我们要查询的资源（例如天气 API）。API 规范通常只提到端点本身，例如 /v1/weather，而把根入口点隐去。URL 一般还接受参数：可以是作为 URL 一部分的路径参数（path parameter），比如我们的邮政编码 API 端点 /v1/weather/{zipcode}；也可以是查询参数（query parameter），即以编码后的键值对形式附加在问号之后，如 /v1/weather?zipcode=<zipcode>。按照惯例，路径参数通常用来指代一个或多个资源，查询参数则用来对返回的数据执行操作，比如排序或过滤。

### 规范

资源本身与返回给客户端的表示（representation）是彼此分离的。换句话说，服务器可以用 HTML、XML、JSON 等格式发送数据，而与数据在应用数据库中的存储方式无关。你可以在某个 API 的规范（specification）中了解它的响应格式——规范相当于一本 API 的使用手册。OpenAPI 格式是编写 API 文档的绝佳方式之一，它在业界被广泛使用，并且隶属于 Linux 基金会。你可以在 https://editor.swagger.io 使用 Swagger 图形化编辑器来体验它。

例如，代码清单 6-1 展示了 /v1/weather/{zipcode} 端点的一份规范，以 JSON 编写。把这段代码粘贴到 Swagger 编辑器中，就能以更友好的方式浏览生成的文档。

```json
{
    "openapi": "3.0.0",
    "info": {
        "title": "Sample Next.js - OpenAPI 3.x",
        "description": "The example APIs from our Next.js application",
        "version": "1.0.0"
    },
    "servers": [
        {"url": "https://www.usemodernfullstack.dev/api/"},
        {"url": "http://localhost:3000/api/"}
    ],
    "paths": {
        "/v1/weather/{zipcode}": {
            "get": {
                "summary": "Get weather by zip code",
                "parameters": [
                    {
                        "name": "zipcode",
                        "in": "path",
                        "description": "The zip code for the location as string.",
                        "required": true,
                        "schema": {
                          "type": "string",
                          "example": 96815
                        }
                    }
                ],
                "responses": {
                    "200": {
                        "description": "Successful operation",
                        "content": {
                            "application/json": {
                                "schema": {
                                    "$ref": "#/components/schemas/weatherDetailType"
                                }
                              }
                        }
                    }
                }
            }
          }
    },
    "components": {
        "schemas": {
            "weatherDetailType": {
                "type": "object",
                "properties": {
                    "zipcode": {
                        "type": "string",
                        "example": 96815
                    },
                    "weather": {
                        "type": "string",
                        "example": "sunny"
                    },
                    "temp": {
                        "type": "integer",
                        "format": "int64",
                        "example": 35
                    }
                }
            }
        }
    }
}

```

*代码清单 6-1：/v1/weather/{zipcode} 端点的 OpenAPI 规范*

首先我们定义通用信息，例如 API 的标题和描述。这里最重要的值是 API 版本。在第 108 页的练习 6 中，我们会调整服务器来体现这个版本。接下来设置的属性是服务器（server），即 API 的根入口点。我们在这里使用 localhost，因为我们的 Next.js 应用目前是在本地运行的。

然后，我们在 paths 下指定各个唯一的 API 端点，为每个端点设置路径、参数和响应。在这个例子中，我们为其中一个端点 /v1/weather/{zipcode} 指定了最少必要的数据，并说明它使用 GET 方法。花括号（{}）表示 URL 参数，但我们也在路径中显式设置了名为 zipcode 的参数。此外，我们为该参数定义了 schema（即格式），它应当是一个字符串。

接着，在 responses 部分，我们设置了当 HTTP 状态码为 200: OK 时 API 应返回的响应。这部分内容采用 application/json 格式，类型为 weatherDetailType——你在前面的章节中应该已经见过它。它类似于我们在 custom.d.ts 文件中定义的自定义类型，只不过这里用的是 JSON 而不是 TypeScript。

注意，Swagger 编辑器还会基于这份规范生成一个交互式测试场（playground），让我们可以针对运行中的服务器测试 API 的各个端点。此外，我们可以直接在编辑器界面中生成服务器和客户端。生成的服务器将提供规范中描述的 REST API，而客户端则会生成一个库，任何应用都可以用它来消费规范所描述的 API。这个交互式测试场和生成的代码让使用第三方 API 变得非常容易。

### 状态与认证

RESTful API 是无状态的（stateless），也就是说，它们不在服务器上存储会话信息。会话信息指任何关于用户先前交互的数据。例如，设想一家网店的购物车。在有状态（stateful）的设计中，应用会把购物车的内容存储在服务器上，并在你添加新商品时随时更新。而在 RESTful 设计中，客户端改为在每个请求中携带全部相关的会话数据。用户与服务器的每次交互都被独立理解，不依赖先前请求的上下文。

即便如此，公开的 RESTful API 通常仍需要某种形式的认证。为了区分已认证用户和未认证用户的请求，这些 API 通常会发放一个令牌（token），用户应在其后的请求中携带它。消费方会把该令牌作为请求数据的一部分发送，或放在 HTTP Authorization 请求头中。我们将在第 9 章详细介绍授权令牌及其工作原理。

这种无状态设计意味着，无论客户端是直接向末端服务器、代理还是负载均衡器请求数据，认证都能正常工作。因此，RESTful API 能够应对分层系统。无状态架构在高流量场景下也很理想，因为它免除了服务器从数据库检索会话信息所带来的负载。

### HTTP 方法

在 REST 中，与数据集交互有四种标准方式：创建（create）、读取（read）、更新（update）和删除（delete）。这些交互通常被称为 CRUD 操作。REST API 使用以下 HTTP 方法对请求的资源执行这些操作：

GET　　用于从资源检索数据。这是最常见的请求；每次你在浏览器中访问一个网站，都是向该网站地址发起了一次 GET 请求。

POST　　用于向集合资源添加新元素。多次发送同一个 POST 请求，每次都会创建一个新元素，从而产生多个内容相同的元素。当你发送电子邮件或提交网页表单时，客户端通常在幕后发送的就是 POST 请求，因为你正在数据库中创建新资源。

PUT　　用于覆盖或更新现有资源。多次发送同一个 PUT 请求，只会用更新后的内容创建或覆盖同一个元素。例如，当你在 Instagram 或 Facebook 上重新上传一张图片时，发送的可能就是 PUT 请求。

PATCH　　用于部分更新现有资源。与 PUT 不同，你只发送与当前数据集有差异的数据，因此这是一种更小、更高效的操作。例如，在社交媒体页面上更新你的个人资料，可能就是通过 PATCH 请求完成的。

DELETE　　用于删除资源（例如删除 Instagram 上的一张图片）。

REST API 请求与所有 HTTP 请求一样受到性能因素的制约。开发者必须考虑网络带宽、延迟和服务器负载等关键因素。虽然应用通常无法影响网络延迟或用户带宽，但可以通过缓存请求、用先前缓存的结果进行响应来提升性能。

总的来说，推荐的做法是激进地缓存请求。通过避免额外的服务器请求，我们可以显著加快应用速度。遗憾的是，并非所有 HTTP 请求都可缓存。GET 请求的响应默认可缓存，但 PUT 和 DELETE 的响应完全不可缓存，因为它们无法保证可预测的响应：在两个相似的 PUT 请求之间，资源可能已被某个 DELETE 请求删除，反之亦然。POST 和 PATCH 请求的响应在理论上可以缓存，前提是响应提供了 Expire 头或 Cache-Control 头，且你后续对同一资源的调用是 GET 请求。即便如此，服务器通常也不会缓存这两类请求。

## 实践 REST

让我们通过这个假想的天气服务来练习使用 REST。假设我们读了 API 契约，得知已认证的用户可以通过其公开的 REST API 获取和更新该服务的数据集。API 返回 JSON 数据，服务器的 URL 是 https://www.usemodernfullstack.dev，/api/v2/weather/{zipcode} 端点接受 GET 和 PUT 请求。本节中，我们将完整走查两类请求与响应：用 GET 请求获取某个邮政编码的当前天气数据，以及用 PUT 请求更新已存储的天气数据。

### 读取数据

要获取你所在位置的天气，你可以发起一个包含邮政编码 96815 和授权令牌的 GET 请求。我们可以用命令行工具 cURL 发起这样的 GET 请求，你的系统里应该已经有它；如有需要，可以从 https://curl.se 安装。一个典型的 cURL 请求如下所示：

```bash
$ curl -i url
```

-i 标志会显示我们感兴趣的请求头细节。我们可以用 -X 标志设置 HTTP 方法，用 -H 标志发送额外的请求头。使用转义字符可以发送多行命令（macOS 上是 \，Windows 上是 ^）。注意不要在转义字符后面添加空格。如果你有兴趣，可以用 cURL 查询你在第 89 页的练习 5 中创建的应用里的某个 API 端点。向 https://www.usemodernfullstack.dev/api 上的天气 API v2/weather/{zipcode} 发起 GET 请求的 cURL 调用如下所示：

```bash
$ curl -i \
    -X GET \
    -H "Accept: application/json" \
    -H "Authorization: Bearer 83dedad0728baaef3ad3f50bd05ed030" \
    https://www.usemodernfullstack.dev/api/v2/weather/96815

```

我们向 https://www.usemodernfullstack.dev/api 服务器上的 API 端点 v2/weather/{zipcode} 发起请求，邮政编码包含在 URL 中。我们在 Accept 头中把返回格式设为 JSON，并在 Authorization 头中传入访问令牌。由于这是一个示例 API，它接受任意令牌；如果不提供令牌，API 会返回状态码 401。

以下是 API 对我们 GET 请求的响应：

```json
HTTP/2 200
content-type: application/json ; charset=utf-8
access-control-allow-origin: *

{"weather":"sunny","tempC":"25","tempF":"77","friends":["96814","96826"]}

```

API 以 HTTP 状态码 200 响应，表示请求成功。我们请求的是 JSON 响应，content-type 头也确认响应数据确实是该类型。

我们在第 5 章讨论过的 Access-Control-Allow-Origin 头在这里允许任意域名访问。有了这个设置，只要客户端 JavaScript 想访问这个 API，无论网站属于哪个域名，浏览器都会放行这些请求。如果没有这个 CORS 头，浏览器就会拦截请求、阻止脚本访问响应，并抛出一个 CORS 错误。

最后可以看到，响应体中包含一个 JSON 字符串，即 API 的响应内容。

### 更新数据

现在设想你想把自家街区（96814）和相邻街区（96826）的展示数据添加到你的网站上。遗憾的是，这些邮政编码的数据在 API 上还没有。好在它是开源的，我们可以接入自己的气象站来扩展这个系统。假设我们已经架设好天气传感器并连接到 API，天气一有变化就把数据集添加进去。

下面是我们为更新邮政编码 96814 的天气而发起的 PUT 请求。PUT 请求把数据存储在请求体中，因此我们在 cURL 命令中使用 -d 标志来发送编码后的 JSON：

```bash
$ curl -i \
    -X PUT \
    -H "Accept: application/json" \
    -H "Authorization: Bearer 83dedad0728baaef3ad3f50bd05ed030" \
    -H "Content-Type: application/json" \
    -d "{\"weather\":\"sunny\",\"tempC\":\"20\",\"tempF\":\"68\",
        \"friends\":\"['96815','96826']\"}" \
    https://www.usemodernfullstack.dev/api/v2/weather/96815

```

我们请求的是同一个 API 端点 /api/v2/weather/，只是把 GET 方法换成了 PUT，因为我们不是要从数据库获取数据，而是要添加数据。我们用 Content-Type 头告诉 API 提供方：请求体中的载荷是一个 JSON 字符串。API 会更新数据集，并返回状态码 200 以及一个带有额外状态详情的 JSON 对象：

```json
HTTP/2 200
content-type: application/json ; charset=utf-8
access-control-allow-origin: *
{"status":"ok"}

```

你可以在 https://restfulapi.net 上进一步了解 RESTful API，那里涵盖更具体的话题，例如压缩和安全模型，并会指导你设计自己的 RESTful API。现在，让我们把注意力转向 GraphQL——另一种更先进的 API 类型。

## GraphQL API

与 REST 不同，GraphQL 不只是一种架构模式。它是一套完整的、开源的 API 数据查询与操作语言，也是全栈 Web 开发中最流行的 REST 替代方案，被 Airbnb、GitHub、PayPal 以及许多其他公司使用。据报道，排名前 1 万的网站中有 10% 在使用 GraphQL。本节只介绍它的部分特性，但应当能让你扎实理解 GraphQL 的原理。

> **NOTE**
>
> 尽管名字里有“图”（graph）一词，GraphQL 并不要求使用 Neo4j 之类的图数据库。我们可以用它查询连接到 GraphQL 服务器的任何数据源，包括 MySQL 和 MongoDB 等常见数据库。

与 REST 一样，GraphQL API 也通过 HTTP 运作。但一个 GraphQL 实现只暴露一个 API 端点，通常称为 /graphql，用于访问所有资源并执行所有 CRUD 操作。相比之下，REST 为每个资源提供一个专门的端点。

另一个区别是，我们只使用 POST 请求连接 GraphQL 服务器。我们不通过 HTTP 方法来声明想要的 CRUD 操作，而是在 POST 请求体中使用查询（query）和变更（mutation）。查询是读取操作，而变更是创建、更新和删除数据的操作。

与依赖标准 HTTP 状态码的 REST 不同，GraphQL 只有在某个操作完全无法执行时才返回 500，即 Internal Server Error；否则即使查询或变更存在问题，响应也使用 200。原因在于解析器（resolver）可能在遇到问题之前已经部分执行完毕。在生产环境部署 GraphQL API 时请牢记这一点：许多标准的运维实践和工具可能需要针对这种行为做出调整。

### Schema

GraphQL API 在其 schema 中定义可用的查询和变更，schema 相当于 REST API 的规范。schema 也叫 typedef，使用模式定义语言（Schema Definition Language，SDL）编写。SDL 的核心元素是类型（type）——即包含带类型字段、用以定义其属性的对象——以及可选的指令（directive），指令用于附加额外信息，例如为查询指定缓存规则，或将字段标记为已弃用。

代码清单 6-2 展示了我们的假想天气 API 的 GraphQL schema，它返回某个位置的天气数据。

```javascript
export const typeDefs = gql`

    type LocationWeatherType {
        zip: String!
        weather: String!
        tempC: String!
        tempF: String!
        friends: [String]!
    }

    input LocationWeatherInput {
        zip: String!
        weather: String
        tempC: String
        tempF: String
        friends: [String]
    }

    type Query {
        weather(zip: String): [LocationWeatherType]!
    }

    type Mutation {
        weather(data: LocationWeatherInput): [LocationWeatherType]!
    }
`;

```

*代码清单 6-2：天气 API 的 GraphQL schema*

你会注意到，这个 schema 是一个标签模板字面量（tagged template literal），你在第 2 章学过它。我们首先描述自定义的 GraphQL 对象类型。这些对象类型表示 API 返回的数据，类似于我们在 TypeScript 中定义的自定义类型。一个类型有名字，并且可以实现接口。每个自定义对象类型都包含若干字段（field），每个字段有名字和类型。GraphQL 内置了 Int、Float、String、Boolean 和 ID 这几个标量类型。感叹号（!）表示不可为空的字段，方括号（[]）中的列表表示数组。

第一个自定义对象类型是 LocationWeatherType，它描述一次天气查询的位置信息。这里我们用 String! 表达式把 zip 字段标记为不可为空。因此，GraphQL 服务总会为这个字段返回一个值。我们把 friends 字段定义为字符串数组，用来通过邮政编码表示相关的气象站。它同样不可为空，因此当它出现在返回值中时，总会包含一个数组（含零个或多个元素）。friends 数组里的 String 确保每个元素都是字符串。

然后，我们为第一个变更定义输入类型对象。变更需要这类类型，它们表示从 API 消费方接收的输入。由于消费方应当能够只传入自己想更新的字段，我们省略了感叹号，让这些字段变为可选。在 GraphQL 中，我们需要用内置类型分别为输入对象和返回值类型下定义；与 TypeScript 不同，我们不能使用泛型的自定义类型。

schema 还定义了查询和变更函数。它们是消费方可以发送给 API 的操作。weather 查询接受一个邮政编码作为参数，总是返回一个 WeatherLocationType 对象数组。weather 变更接受一个 WeatherLocationInput 参数，总是返回修改后的 WeatherLocationType 对象。

我们的 schema 没有包含任何指令，本章也不会深入其语法。不过，使用指令的原因之一是缓存。由于 GraphQL 查询使用 POST，而默认的 HTTP 缓存无法缓存 POST，我们必须在服务器端手动实现缓存。我们可以在类型定义上用 @cacheControl 指令静态配置缓存，也可以在解析器函数中用 cacheControl.setCacheHint 动态配置。

### 解析器

在 GraphQL 中，解析器（resolver）是实现 schema 的函数。每个解析器函数映射到一个字段。查询解析器实现数据的读取，而变更解析器实现数据的创建、更新和删除。它们共同提供完整的 CRUD 功能。

要理解解析器的工作方式，你可以把每个 GraphQL 操作想象成一棵嵌套函数调用树。在这样的抽象语法树（abstract syntax tree，AST）中，操作的每个部分代表一个节点。例如，考虑一个复杂的嵌套 GraphQL 查询：它询问某个位置的当前天气，以及它所有邻居的天气。这个示例所用的 GraphQL schema 如 代码清单 6-3 所示。

```javascript
export const typeDefs = gql`

    type FriendsType {
        zip: String!
        weather: String!
    }

    type LocationWeatherType {
        zip: String!
        weather: String!
        tempC: String!
        tempF: String!
        friends: [FriendsType]!
    }

    type Query {
        weather(zip: String): [LocationWeatherType]!
    }
`;

```

*代码清单 6-3：嵌套 GraphQL 查询示例所用的 GraphQL schema*

在这个示例 schema 中，我们替换了 friends 数组的内容：我们希望它包含的不再是简单字符串，而是带有邮政编码和天气信息的对象。因此我们定义了新的 FriendsType，并把数组元素的类型设为它。

代码清单 6-4 定义了这个复杂示例查询。

```graphql
query GetWeatherWithFriends {
    weather(zip: "96815") {
        weather
        friends {
            weather
        }
    }
}

```

*代码清单 6-4：嵌套的 GraphQL 查询*

这个查询接受 zip 参数 96815，然后以字符串形式返回它的 weather 属性，以及它所有 friends 的 weather 属性。但这个查询在底层是如何运作的呢？

图 6-1 展示了解析器链和对应的 AST。GraphQL 服务器会先把查询解析成这种结构，然后将 AST 与类型定义 schema 进行校验，确保查询能够执行而不会陷入逻辑问题。最后，服务器执行该查询。

我们来看这个查询的解析器链。Query.weather 函数接受一个参数——邮政编码，并返回该邮政编码对应的位置对象。接着，服务器沿每个分支分别继续。对于查询中的 weather，它返回位置对象的 weather 属性（Location.weather），到此该分支结束。查询的第二部分要求从位置对象获取所有 friends 及其 weather 属性，它会执行 Location.Friends 查询，然后对每个结果返回 Friends.weather 属性。每一步的解析器对象中都包含其父字段解析器返回的结果。

现在回到我们的天气 schema，定义解析器。我们让它们保持简单。在 代码清单 6-5 中可以看到，它们的名字与 schema 中定义的名字一致。

```javascript
export const resolvers = {
    Query: {
        weather: async (_: any, param: WeatherInterface) => {
            return [
                {
                    zip: param.zip,
                    weather: "sunny",
                    tempC: "25C",
                    tempF: "70F",
                    friends: []
                }
            ];
        },
    },
    Mutation: {
        weather: async (_: any, param: {data: WeatherInterface}) => {
            return [
                {
                    zip: param.data.zip,
                    weather: "sunny",
                    tempC: "25C",
                    tempF: "70F",
                    friends: []
                }
            ];
        }
    },
};

```

*代码清单 6-5：天气 API 的 GraphQL 解析器*

我们首先为 Query 和 Mutation 属性定义 async 函数，并把该对象赋给 const resolvers。每个函数接受两个参数。第一个参数表示解析器链中上一个解析器对象。我们没有使用嵌套或复杂查询，因此这里它总是 undefined。为此，我们用 any 类型来避免 TypeScript 报错，并按照第 3 章学过的下划线（_）约定把它标记为未使用。第二个参数是一个对象，包含函数被调用时传入的数据。对 weather 查询和 weather 变更来说，它是一个实现了 WeatherInterface 的对象。

目前这两个函数在很大程度上都忽略了这个参数，只使用 zip 属性来反映输入。而且，它们返回一个静态 JSON 对象，与我们前面几个代码清单中创建的 REST API 类似。这些静态数据只是占位符，稍后我们会用数据库查询的结果替换它们。响应遵守了我们在 GraphQL schema 中定义的 API 契约，因为这些数据就是由天气位置数据集组成的数组。

## GraphQL 与 REST 的比较

我们已经在 Next.js 应用中实现过 RESTful API，而且正如你在本章看到的，REST 用起来相当简单。你可能会问，那为什么还要考虑使用 GraphQL？原因在于，GraphQL 解决了 REST API 中常见的两个问题：过度获取（over-fetching）和获取不足（under-fetching）。

### 过度获取

当客户端查询一个 REST 端点时，API 总是返回该端点的完整数据集。这意味着 API 常常会交付超出必要范围的数据，这一常见的性能问题称为过度获取（over-fetching）。例如，我们的示例 RESTful 天气 API /api/v2/weather/zip/96815 会返回某个邮政编码的全部天气数据，哪怕你需要的只是摄氏温度。接下来你还得手动过滤结果。而在 GraphQL 中，API 请求会显式定义想要返回的数据。

让我们看一个例子，看看 GraphQL 如何让我们把 API 响应数据保持在最少。下面的 GraphQL 查询只返回邮政编码 96815 位置的摄氏温度：

```graphql
query Weather {
    weather(zip: "96815") {
        tempC
    }
}

```

在 GraphQL 中，我们把查询作为 JSON 字符串随 POST 请求的数据一起发送：

```bash
$ curl -i \
    -X POST \
    -H "Accept: application/json" \
    -H "Authorization: Bearer 83dedad0728baaef3ad3f50bd05ed030" \
    -H "Content-Type: application/json" \
    -d '{"query":"\nquery Weather  {\n  weather(zip: \"96815\") {\n    tempC  \n  }\n}"}' \
    https://www.usemodernfullstack.dev/api/graphql

```

我们通过向 /api/graphql 端点发起 POST 请求来消费这个 API，然后把 Content-Type 头和 Accept 头都设为 JSON，显式告诉 API 我们在请求体中发送的是一个 JSON 对象，并期望 JSON 响应。与 RESTful 请求一样，我们在 Authorization 头中设置访问令牌。POST 体中包含查询天气数据的查询，其中的 \n 控制字符表示 GraphQL 查询中的换行。正如契约所定义的，该查询期望一个参数 zip，我们传入邮政编码 96815。此外，我们要求 API 只返回 weather 节点的 tempC 字段。

以下是 GraphQL API 的响应：

```json
HTTP/2 200
content-type: application/json ; charset=utf-8
access-control-allow-origin: *

{"data":{"weather":[{"tempC":"25C"}]}}

```

API 以状态码 200 响应。我们在请求的查询中指明，只关心 weather 对象的 tempC 字段，因此收到的也正是这个字段。API 没有返回邮政编码、华氏温度、天气字符串或 friends 数组。

### 获取不足

反过来，REST 数据集可能不包含你需要的全部数据，从而要求你发送后续请求。这个问题称为获取不足（under-fetching）。设想你的朋友们也有气象站，你想获取他们所在邮政编码的当前天气。RESTful 天气 API 会返回一个包含相关邮政编码的数组（friends），但你还需要为每个邮政编码单独发起请求才能拿到它们的天气信息，这可能引发性能问题。

GraphQL 把数据集当作图（graph）中的节点来处理，节点之间存在关系。因此，扩展单个查询以获取相关数据非常简单。我们的示例 GraphQL 服务器的解析器已设置成：只要请求的查询包含 friends 字段，就去获取关于 friends 的额外数据。我们如下定义 GraphQL 查询：

```graphql
query Weather {
    weather(zip: "96815") {
        tempC
        friends {
            tempC
        }
    }
}

```

下面展示了一个通过 friends 数组获取所有相关节点的示例请求。同样，我们定义了要返回的数据，并且只查询 friends 的 tempC 字段：

```bash
$ curl -i \
    -X POST \
    -H "Accept: application/json" \
    -H "Authorization: Bearer 83dedad0728baaef3ad3f50bd05ed030" \
    -H "Content-Type: application/json" \
    -d '{"query":"query Weather  {\n  weather(zip: \"96815\")
        {\n    tempC\n    friends {\n      tempC\n    }\n  }\n}"}' \
    https://www.usemodernfullstack.dev/api/graphql

```

POST 体中一行包含了对邮政编码 96815 天气数据的查询，并像上一个请求一样要求返回 tempC 字段。为了扩展这个查询，我们在 friends 字段上添加了一个子选择（sub-selection）。现在 GraphQL 会遍历相关节点及其字段，返回那些邮政编码出现在 96815 节点的 friends 数组中的节点的 tempC 字段。

以下是 GraphQL 服务器的响应。可以看到，其中包含了来自相关节点的数据：

```json
HTTP/2 200
content-type: application/json ; charset=utf-8
access-control-allow-origin: *

{"data":{"weather":[{"tempC":"25C","friends":
[{"tempC":"20C"},{"tempC":"30C"}]}]}}

```

如你所见，GraphQL 让我们只需调整请求中的数据，就能轻松扩展查询。

## 练习 6：为 Next.js 添加 GraphQL API

让我们把天气应用的 API 改造成使用 GraphQL。为此，必须先在项目中加入 GraphQL。GraphQL 不是一种模式，而是一个由服务器和查询语言组成的环境，这两者都必须添加到 Next.js 中。

我们将安装独立的 Apollo 服务器——最受欢迎的 GraphQL 服务器之一，它也提供了 Next.js 集成。打开终端，导航到你在第 5 章构建的重构版应用。在该目录的顶层、package.json 文件旁边，执行以下命令：

```bash
$ npm install @apollo/server @as-integrations/next graphql graphql-tag
```

这条命令还会安装我们需要的 GraphQL 语言模块和 GraphQL tag 模块。

### 创建 schema

如前所述，每个 GraphQL API 都从 schema 定义开始。在 Next.js 目录中、pages 文件夹旁边创建一个名为 graphql 的文件夹。所有与 GraphQL 相关的文件都将放在这里。

现在创建一个名为 schema.ts 的文件，把你之前在 代码清单 6-2 中写过的代码粘贴进去。我们已经定义并讨论过这里使用的类型定义。只需在文件顶部添加一行：

```javascript
import gql from "graphql-tag";
```

这一行导入了我们用来定义 schema 的 gql 标签模板字面量。

### 添加数据

我们希望 API 根据发送给它的查询的参数和属性返回不同的数据。因此，我们需要向项目添加数据集。GraphQL 可以查询任何数据库，甚至是静态 JSON 数据。那我们就实现一个 JSON 数据集。在 graphql 目录中创建 data.ts 文件，添加 代码清单 6-6 中的代码。

```javascript
export const db = [
    {
        zip: "96815",
        weather: "sunny",
        tempC: "25C",
        tempF: "70F",
        friends: ["96814", "96826"]
    },
    {
        zip: "96826",
        weather: "sunny",
        tempC: "30C",
        tempF: "86F",
        friends: ["96814", "96814"]
    },
    {
        zip: "96814",
        weather: "sunny",
        tempC: "20C",
        tempF: "68F",
        friends: ["96815", "96826"]
    }
];

```

*代码清单 6-6：GraphQL API 所用的 graphql/data.ts 文件*

这段 JSON 定义了三个天气位置及其属性。消费方将能够向我们的 API 查询这些数据集。

### 实现解析器

现在我们可以定义解析器了。在 graphql 目录中添加 resolvers.ts 文件，并粘贴 代码清单 6-7 中的代码。这与我们介绍解析器时讨论过的代码类似，区别在于：我们不再向消费方返回同一个静态 JSON 对象，而是查询新的数据集。

```javascript
import {db} from "./data";

declare interface WeatherInterface {
    zip: string;
    weather: string;
    tempC: string;
    tempF: string;
    friends: string[];
}

export const resolvers = {
    Query: {
        weather: async (_: any, param: WeatherInterface) => {
            return [db.find((item) => item.zip === param.zip)];
        }
    },
    Mutation: {
        weather: async (_: any, param: {data: WeatherInterface}) => {
            return [db.find((item) => item.zip === param.data.zip)];
        }
    }
};

```

*代码清单 6-7：GraphQL API 所用的 graphql/resolvers.ts 文件*

我们导入了之前创建的 JSON 对象数组，并为解析器定义了一个接口。查询解析器使用传入的邮政编码查找对象，并把它返回给 Apollo 服务器。变更解析器做同样的事情，只是参数结构略有不同：要通过 data 属性访问。遗憾的是，由于数据是一个静态 JSON 文件，我们实际上无法通过这个变更修改数据。这里实现变更只是为了演示。

### 创建 API 路由

Apollo GraphQL 服务器暴露一个端点 graphql/，我们现在就来实现它。在 api 文件夹中创建新文件 graphql.ts，添加 代码清单 6-8 中的代码。这段代码初始化 GraphQL 服务器，并添加一个 CORS 头，使我们能够从不同域名访问这个 API，稍后还能使用内置的 GraphQL 沙盒浏览器来把玩 GraphQL。你在前面的 cURL 响应中已经见过这个头。

```javascript
import {ApolloServer} from "@apollo/server";
import {startServerAndCreateNextHandler} from "@as-integrations/next";
import {resolvers} from "../../graphql/resolvers";
import {typeDefs} from "../../graphql/schema";
import {NextApiHandler, NextApiRequest, NextApiResponse} from "next";

//@ts-ignore
const server = new ApolloServer({
    resolvers,
    typeDefs
});

const handler = startServerAndCreateNextHandler(server);

const allowCors =
    (fn: NextApiHandler) => async (req: NextApiRequest, res: NextApiResponse) => {
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

export default allowCors(handler);

```

*代码清单 6-8：创建 GraphQL API 入口的 api/graphql.ts 文件*

这段代码就是创建 GraphQL 入口所需的全部内容。首先我们导入必要的模块，包括我们之前创建的 GraphQL schema 和解析器。然后用 typedefs 和 resolvers 初始化一个新的 GraphQL 服务器。

我们启动服务器，接着创建 API 处理器。为此，我们使用 Next.js 集成辅助函数来启动服务器并返回 Next.js 处理器。该集成辅助函数把无服务器（serverless）的 Apollo 实例连接到 Next.js 自定义服务器。在把默认导出定义为接受 API 请求对象和响应对象作为参数的异步函数之前，我们先创建一个包装器，为请求添加 CORS 头。函数内的第一段代码设置 CORS 头，并把允许的请求限定为 POST。我们需要这些 CORS 头来让 GraphQL API 公开可用；否则，我们将无法从运行在其他域名上的网站连接到这个 API，甚至无法使用服务器内置的 GraphQL 沙盒。

这里 CORS 设置的一部分是：对任何 OPTIONS 请求立即返回 200。CORS 模式把 OPTIONS 请求用作预检（preflight）。此时浏览器只请求头信息，然后检查响应中的 CORS 头，确认它调用 API 所在的域名被允许访问该资源，之后才发起真正的请求。

然而，我们的 Apollo 服务器只允许 POST 和 GET 请求，对预检 OPTIONS 请求会返回 405: Method Not Allowed。因此，我们不把这个请求传给 Apollo 服务器，而是直接结束请求，并连同前面的 CORS 头一起返回 200。浏览器随后应当会继续执行 CORS 流程。最后，我们启动服务器，并在目标路径 api/graphql 上创建 API 处理器。

### 使用 Apollo Sandbox

用 npm run dev 启动你的 Next.js 服务器。你应当看到 Next.js 应用运行在 http://localhost:3000 上。如果你访问 http://localhost:3000/api/graphql 处的 GraphQL API，就会看到用于查询该 API 的 Apollo 沙盒界面，如图 6-2 所示。

在左侧的 Documentation（文档）面板中，我们可以看到可用的查询，它们是我们先前定义的 query 对象的字段。不出所料，这里能看到 Weather 查询；点击它后，中间的 Operation（操作）面板会出现一个新查询。与此同时，界面会发生变化，显示可用的参数和字段，点击每一项都能看到更多信息。使用加号（+）按钮，我们可以把字段添加到 Query（查询）面板，并针对数据运行它们。

试着创建一个返回 zip 和 weather 属性的 Weather 查询。这个查询需要一个邮政编码作为参数：通过左侧的用户界面添加它，然后在下方窗格的 variables（变量）区域中，向 JSON 对象添加字符串形式的邮政编码 96826。接着点击 Operation 面板顶部的 Weather 按钮运行查询。你应当在右侧的 Response（响应）面板中以 JSON 形式收到这个邮政编码的结果。把你的屏幕与图 6-3 对比一下。

在进入下一章之前，多练习编写查询、访问属性，以及用无效参数制造错误，找找使用 GraphQL 的感觉。

## 总结

本章探讨了 RESTful 和 GraphQL Web API 及其在全栈开发中的角色。虽然前面几章我们使用的是 REST 设计，但现在你应当已经熟悉了无状态服务器的概念，以及在 REST 中执行 CRUD 操作的五种 HTTP 方法。你还练习了使用公开的 REST API 读取和更新数据，并分析了它的请求与响应。

GraphQL API 的实现要多花一点功夫，但它能减少 REST 中常见的过度获取和获取不足问题。你学会了用 schema 定义 API 契约，用解析器实现其功能，然后查询了一个 API，并在请求中定义了要返回的数据集。

最后，你通过添加 Apollo 服务器，为现有的 Next.js 应用加入了一个 GraphQL API。现在你应该能够创建自己的 GraphQL API，并消费第三方资源了。要进一步学习 GraphQL，我推荐 https://www.howtographql.com 上的教程，以及 https://graphql.org/learn/ 上的 GraphQL 官方入门。

下一章你将探索 MongoDB 数据库和对象数据建模库 Mongoose，用它们来存储数据。
