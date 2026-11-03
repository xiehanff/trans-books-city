# 第 9 章 用 OAuth 授权

*Chapter 9: Authorization with OAuth*

有些应用会在登录流程中存储用户数据。实现这种认证（authentication）与授权（authorization）的方式有很多，其中最简单的一种是使用 OAuth2，借助知名公司的现有账户。OAuth2（或简称 OAuth）是一项用于访问委托（access delegation）的开放标准，如果你用过应用里“使用 Facebook、GitHub 或 Google 账号登录”的功能，多半已经接触过它。

OAuth 协议的本质，是让我们的 Web 应用能够访问另一个应用的登录数据，而无需第三方把用户的凭证分享给我们。为此，用户通过创建访问令牌（access token），向我们的应用授予对其第三方账户的访问权限。OAuth 是基于授权的访问委托的公认标准，Amazon、Google、Facebook、Microsoft 和 GitHub 都支持 OAuth 工作流。

本章将向你介绍 OAuth 工作流，然后剖析其访问委托所使用的持有者令牌（bearer token）的结构，为在第 II 部分把 OAuth2 集成进 Food Finder 应用打好基础。在第 168 页的练习 9 中，我们不会为示例 Next.js 应用添加 OAuth 流程，而是手动走一遍 OAuth 授权过程。

## OAuth 的工作原理

在深入 OAuth 之前，你需要理解认证与授权的区别。简单来说，认证用于验证用户的身份，而授权则规定已认证用户拥有的权限，并强制执行这些权限。OAuth 允许把这一过程委托给用户已在其上拥有账户的第三方，从而简化用户的登录过程。

### 认证 vs. 授权

每当应用收到登录请求时，它都会先检查用户的凭证再允许访问，这个过程称为认证。凭证通常由用户名和密码组成，但也可以是硬件令牌，或涉及指纹、面部识别等生物特征因素。随后，应用会验证这些凭证是否与数据库中存储的相匹配。

最简单的认证形式是单因素认证（single-factor authentication），它只需要一个因素，通常是密码。遗憾的是，它也是实现认证时最不安全的方式。更稳健、值得推荐的形式是多因素认证（multifactor authentication），要求用户提供至少两个因素：可以是用户知道的东西（比如密码），加上用户拥有的东西（比如实体令牌），或者用户本身的特征（比如是指纹的主人）。你登录 PayPal 或 Google 时可能就用过多因素认证，两者都要求你提供密码和一个额外的一次性密码（one-time password, OTP）。

OTP 是基于你注册账户时与应用共享的秘密生成的代码，双方都会在很短的间隔内重新生成这对代码。你的 OTP 可能由认证器应用（如 Google Authenticator）生成，也可能通过短信接收。你持有账户的应用（例如 PayPal 或 Google）会在服务器端生成自己的 OTP 代码。你一提交自己的代码，服务器就会以密码学方式验证两者是否匹配。

授权发生在用户完成认证之后。广义上讲，它包括查看用户数据，并判断该用户是否拥有访问某资源所需的访问权限。典型的全栈应用既可以自己处理这些用户数据，也可以让用户在不提供用户数据的情况下登录。后一种方式有若干好处：处理和存储用户数据可能很麻烦，还会带来额外的责任（例如必须遵守更严格的隐私和数据保留法律），并且要求你的用户再创建一个账户。

假如你为用户提供通过授权提供方（authorization provider）使用现有账户登录的选项，那么你就消除了一个使用门槛，也不必再操心处理他们的数据。如果你确实需要用户数据（例如要向客户收费），可以使用 OAuth 工作流，并在必要时把从提供方收到的数据（比如用户的支付信息）保存到自己的数据库中。

### OAuth 的角色

每当一个 Web 应用让你通过 Facebook、GitHub 或 Google 这类第三方提供方登录时，它背后使用的都是 OAuth 授权码模式（authorization code flow）。OAuth 并不是认证，而是一种对你要使用的 Web 应用进行授权的方式，让它可以代表你执行操作或访问资源。常见的操作包括发布内容到你的 Facebook 时间线，以及访问你的姓名、头像或电子邮箱等数据。因此，每次使用基于 OAuth 的登录功能时，应用都会请求特定的权限，并且只能使用你授予它的那些权限。

要理解 OAuth，必须先理解它的术语。每个 OAuth 流程都使用一组 RESTful API，对客户端（client，即一个应用）进行授权，使其能够从资源提供方（resource provider，如 Facebook、GitHub 或 Google）那里获取资源（比如用户的个人资料信息），资源提供方拥有客户端想要访问的受保护资源。此外，我们把提供 OAuth API 端点的服务器称为授权服务器（authorization server），把拥有访问权限（因而有权授予应用访问某资源）的一方称为资源所有者（resource owner）。在大多数场景中，资源所有者就是应用的最终用户。

为了获得资源所有者的授权，客户端应用会把它的客户端凭证（client credentials）——即 ID 和密钥（secret）——以及用户凭证发送给授权服务器；授权服务器通常与资源提供方同属一个系统。授权服务器对资源所有者进行认证，并处理 OAuth 流程，最终向其颁发访问令牌，让用户能够访问资源提供方上的受保护资源。授权服务器和资源提供方实际上是同一系统上的两组 API。

客户端 ID（client ID）是客户端应用的公开标识符，可以公开，也可以存放在代码里。与客户端 ID 不同，客户端密钥（client secret）应当保密；它是应用专属的密码，绝不能存放在代码中，而应通过 Next.js 的环境文件或服务器的环境变量来管理。

### 授权类型（grant types）

OAuth 流程有若干变体。这些授权类型各自覆盖特定的用例，但最终都会生成一个访问令牌。OAuth 规定了四种授权类型：客户端凭证模式（client credentials flow）、隐式模式（implicit flow）、授权码模式（authorization code flow）和资源所有者密码凭证模式（resource owner password credentials flow）。

客户端凭证模式用于机器对机器（machine-to-machine）的通信；当不需要真正的最终用户授权时我们会使用它，比如连接 API 的自动化任务。此时，任务本身既是客户端又是资源所有者：它知道资源所有者的凭证（客户端 ID 和客户端密钥），并把它们传给授权服务器以换取访问令牌。

全栈 Web 开发中最常见的授权类型是授权码模式。在这种场景下，我们的 Web 应用是客户端，它会调用两个不同的 API 端点：第一次用于获取授权码（authorization grant code），第二次则用这个授权许可换取访问令牌。第 161 页的“授权码模式”一节将深入剖析这一过程。

后两种授权类型不应再使用。隐式模式与授权码模式类似，但客户端不再分别发起请求获取授权许可和访问令牌，而是直接收到访问令牌。这种流程跳过了授权步骤，不包含客户端认证，并已被弃用（deprecated）。资源所有者密码凭证模式也应避免使用，因为它要求最终用户把自己的用户凭证交给客户端，再由客户端把这些凭证发送给 OAuth 服务器去兑换访问令牌。这听起来虽然直接，但把真实的用户凭证发送到远程授权服务器是巨大的安全隐患。

### 持有者令牌

客户端应用发起 OAuth 流程后，会收到一个共享的访问令牌，最常见的是易于实现的持有者令牌。这种访问令牌取代了用户的凭证，因此任何持有令牌的人都能访问数据。为防止令牌被盗造成安全漏洞，持有者令牌通常有明确的保质期。过期后，只能使用有效的刷新令牌（refresh token）来刷新令牌。刷新令牌是长效令牌，我们用它来生成新的持有者令牌。

刷新令牌的操作可以隐式或显式完成，并且有多种策略可防止被盗的刷新令牌危及 OAuth 访问。例如，OAuth 提供方可以要求提供唯一 ID 或客户端密钥才颁发新令牌。提供方通常会在每次颁发新持有者令牌时轮换（rotate）刷新令牌，且每个刷新令牌只接受一次。从 OAuth 客户端的角度看，刷新令牌的细节并不重要，因为这个令牌由 OAuth 提供方负责处理。

包含用户会话和认证数据的持有者令牌是 JSON Web Token（JWT）。JWT 是一项以 JSON 对象安全传输数据的开放标准。由于 JSON 相当紧凑，JWT 可以作为 URL 参数、POST 数据的一部分，甚至放在 HTTP 头中发送，都不会影响应用的性能。

JWT 令牌既可以签名，也可以加密，这让应用无需再发起额外请求去验证令牌或获取额外数据。加密令牌会把其中的数据对其他各方隐藏起来；由于其额外开销，加密令牌在 OAuth 中并不常见，目前我们可以忽略它们。签名令牌保证其中数据的完整性，因为对令牌的任何修改都会改变其签名，应用因此可以信任其中存储的信息。

签名 JWT 最常用的加密算法是结合 SHA-256 哈希算法的基于哈希的消息认证码（hash-based message authentication code, HMAC）。HMAC 是消息认证码（message authentication code, MAC）的一种。MAC 的主要特性是可以通过从消息计算校验和（checksum）来验证消息的真实性。校验和用一个数学函数基于原始消息生成一个唯一且可复现的值或数据串；消息一旦改变，校验和也随之改变。这样我们就能快速验证数据的完整性。对 JWT 令牌而言，我们要做两项检查：真实性检查确认消息确实由真正的发送方发出，而数据完整性检查验证消息的内容没有被改动。

与其他类型的 MAC 不同，HMAC 使用加密哈希函数和一个密钥。加密哈希函数可以自由选择，但 HMAC 实现的强度取决于所选函数的密码学强度。JWT 常用 SHA-256 哈希函数，它是 SHA-2 家族中一种快速且抗碰撞（collision-resistant）的加密函数，也用于验证 Debian 软件包和比特币交易。在密码学中，碰撞（collision）指两个不同的输入产生相同的输出。碰撞的可能性越高，我们对哈希函数校验和的信任度就越低。如果碰撞很可能发生，消息就可能被替换成另一条，而哈希函数却显示它没有变化。因此，我们需要抗碰撞的加密函数。

## 授权码模式

为了理解前面提到的授权码模式下的 OAuth 交互是如何进行的，让我们回到虚构的天气服务。假设你想授予各气象站通过 API 向应用写入数据的能力，但每个气象站只能修改自己的 ZIP code（美国邮政编码）。你还希望应用能显示各气象站的位置及其他详细信息。此外，你既不想维护用户账户，也不想为每个气象站手动设置权限，因此使用 OAuth 是最佳选择。

假设每个气象站已经有一个用于发布天气动态的社交媒体账户，这些账户包含常规的用户信息和各站的 ZIP code。我们可以轻松地把该社交媒体提供方用作 OAuth 授权提供方来获取这些数据：各气象站使用社交媒体提供方登录天气应用，应用请求访问气象站的用户资料，然后我们把 OAuth 会话中存储的 ZIP code 与数据集中的进行比对，授予相应的写权限，并获取所需的任何其他数据。

实现这个授权码模式只需要几个步骤，图 9-1 是这些步骤的简化描述。开发者通常使用 SDK 或 Node.js 模块来实现这些步骤，只需提供少数几个属性，比如客户端 ID、客户端密钥和回调 URL（callback URL）。

*图 9-1：OAuth 授权许可流程的简化描述*

要把我们的应用注册为 OAuth 客户端，需要向 GitHub 提供一个指向我们应用的回调 URL，GitHub 会在授权请求完成后把用户重定向到该地址。应用上的这个端点负责接收授权许可（authorization grant）。较新的 OAuth 实现要求回调 URL 使用 HTTPS，以防令牌被截获。

我们的应用必须使用资源所有者的凭证，以及客户端凭证（一个 ID 和一个密钥）来与 GitHub 的授权服务器通信：ID 标识客户端，密钥用于认证客户端。随后，应用可以请求访问特定资源的授权，比如气象站的资料数据。为此，气象站用户需要登录 GitHub 的授权服务器，并会看到一个提示，其中汇总了所请求访问的资源，例如对资料（profile）或动态（stream）的读写权限。如果用户用自己的用户凭证批准了这些请求，OAuth 客户端就会在回调 URL 中以 GET 参数的形式收到授权许可；在流程的下一步，我们在应用中使用的 OAuth SDK 会到授权服务器把这个授权许可兑换成访问令牌。

在这一步，OAuth 客户端使用客户端凭证（即客户端 ID 和客户端密钥），结合先前收到的授权许可，向 OAuth 提供方的授权服务器请求访问令牌。授权服务器是 GitHub 基础设施的一部分；为了完成授权流程，它会认证身份，并验证该授权许可对此身份有效。最后，应用从这里收到持有者令牌，并将其存入用户会话。

有了从 OAuth 提供方收到的令牌和用户会话，我们的应用现在就能代表用户行动，访问其受保护资源，比如来自资源服务器（resource server）的资料数据。要代表用户行动，我们把持有者令牌加到 HTTP 请求的 Authorization 头中；OAuth 提供方用这个令牌检查我们获得的权限并验证我们的身份。要访问用户的数据，只需从会话数据中提取，然后在应用代码中使用即可。

对天气应用来说，我们可以用第二种方式从数据库查询特定位置的天气数据：从用户会话数据中读取 location 属性，把这个值作为提供给 API 端点的 ZIP code。此外，我们还可以访问其他属性，比如简介（description）、名称或头像，把它们显示在天气应用中每个气象站的状态页面上。

## 创建 JWT 令牌

大多数持有者令牌都是 JWT。虽然授权服务器会自动签发它们，但了解其中包含哪类信息是有益的。本节将带你完成为天气服务应用创建一个示例 OAuth JWT 的过程。JWT 是一个由三部分组成的字符串，各部分以句点（.）分隔：头部（header）、负载（payload）和签名（signature）。前两部分是经过 Base64 编码的 JSON 对象，签名则是前两部分的校验和。

### 头部

我们创建的第一个字符串是头部，它定义了基本元数据，比如令牌的类型以及所用的签名算法。代码清单 9-1 展示了如何用 JavaScript 创建一个包含最基本元数据的简单头部。

```javascript
const headerObject = {
    "typ": "JWT",
    "alg": "HS256"
}
```

*代码清单 9-1：OAuth2 天气服务的 JWT 头部*

我们把天气服务令牌的类型设为 JWT，并指定稍后使用 HMAC-SHA-256 算法计算签名。最后，把这个 JSON 对象存入一个常量以备后用。

### 负载

接下来我们创建第二个字符串——负载，它存储令牌的数据。负载的每个属性称为一个声明（claim）。在 OAuth 中，这些声明描述用户对象，通常还包括会话数据。JWT 规范包含三种类型的声明：注册声明（registered claim）、公开声明（public claim）和私有声明（private claim）。

#### 注册声明

注册声明共有七个，每个都是三个字母。虽然在一般的 JWT 中并非必需，但 iss、sub、auth 和 exp 这几个注册声明在 OAuth 的 JWT 中是必需的。

签发者声明（issuer claim）iss 包含签发该 JWT 的实体的唯一标识符。应用的 URL 就是一个合适的值，如 代码清单 9-2 所示。

```json
{
    "iss": "https://www.usemodernfullstack.dev/
}
```

*代码清单 9-2：注册的签发者声明*

主体声明（subject claim）sub 标识 JWT 所属的主体。在 OAuth 客户端认证流程中，主体声明必须是 OAuth 客户端的客户端 ID；而在 OAuth 授权许可中，主体应标识资源所有者，或以化名方式标识一个匿名用户。我们在 代码清单 9-3 中创建一个示例主体声明。

```json
{
    "sub": "THE_CLIENT_ID"
}
```

*代码清单 9-3：注册的主体声明*

受众声明（audience claim）aud 标识令牌的接收方。它的值可以是授权服务器上令牌端点（token endpoint）的 URL，或其他任何能标识接收方的东西，比如应用 ID。示例见 代码清单 9-4。

```json
{
    "aud": "api://endpoint"
}
```

*代码清单 9-4：注册的受众声明*

过期时间声明（expiration claim）exp 标识令牌有效的时间窗口。过了这段时间，授权服务器将拒绝该令牌，你需要请求新的令牌。过期时间声明的值是一个数字，日期以“自 Unix 纪元起的秒数”（seconds since the Unix Epoch）定义——这是时间戳的一种常见格式，通过统计自 1970 年 1 月 1 日以来经过的秒数得出。代码清单 9-5 给出了一个示例。

```json
{
    "exp":  1134156400
}
```

*代码清单 9-5：注册的过期时间声明*

签发时间声明（issued at claim）iat 是可选的，标识授权服务器签发令牌的时间。通过这个声明可以确定令牌已经存在多久，它同样以自 Unix 纪元起的秒数定义，如 代码清单 9-6 所示。

```json
{
    "iat": 1134156200
}
```

*代码清单 9-6：注册的签发时间声明*

生效时间声明（not before claim）nfb 是可选的，标识授权服务器应开始接受该令牌的时间。只要 nfb 声明位于未来，授权服务器就会拒绝该令牌。我们把它定义为以自 Unix 纪元起的秒数表示的数字，如 代码清单 9-7 所示。

```json
{
    "nfb": 1134156100
}
```

*代码清单 9-7：注册的生效时间声明*

JWT 声明（JWT claim）jti 是可选的，它为令牌设置一个唯一 ID（见 代码清单 9-8）。

```json
{
    "jti": "b5f8f86f-82ab-451e-b391-bf6a07041787"
}
```

*代码清单 9-8：注册的 JWT 声明*

授权服务器可能会保存一份近期令牌及其过期时间的列表，用来检查令牌是否在重放攻击（replay attack）中被重复使用；重放攻击指攻击者试图通过重用先前签发的令牌来访问数据。

#### 公开声明

令牌的签发者可以定义公开声明，用于添加应用专属的公开 API。与私有声明不同，公开声明是为公开访问而定义的自定义属性。签发者应在 JWT 声明注册表（JWT Claims registry）中注册这些声明，或者使用带自定义命名空间的抗碰撞名称——例如 UUID 或应用名称。此外，公开声明面向公开使用，因此绝不应包含私密或敏感信息。

对我们虚构的天气服务的 OAuth JWT 而言，公开声明可以包含 ZIP code，以便直接提供每个气象站的位置数据。把 ZIP code 做成公开声明后，我们就不必解析用户对象并手动提取 ZIP code 了。而且，位置本来就是社交媒体资料上公开的信息，并不敏感。

#### 私有声明

私有声明是既非注册声明也非公开声明的自定义声明。我们可以按需定义它们，它们可以专属于我们的应用或用例。即便不要求抗碰撞，也建议使用私有命名空间。与公开声明不同，私有声明包含应用专属的信息，仅供内部使用。公开声明存储姓名等通用信息，私有声明则包含应用内的用户 ID 和角色。例如，我们可以为虚构天气服务的 OAuth JWT 定义一个私有声明，用来指明所使用的服务类型。

理解了负载对象可能的属性之后，你就可以创建一个完整的负载，如 代码清单 9-9 所示，其中指定 GitHub 作为服务。

```javascript
const payloadObject = {
    "exp": 234133423,
    "weather_public_zip": "96815",
    "weather_private_type": "GitHub"
}
```

*代码清单 9-9：OAuth 天气服务的 JWT 负载*

同样，我们创建一个常量来存放这个对象。负载有三个声明，各属不同类型。包含哪些声明由 JWT 令牌的发布者决定；在本例中，我们把令牌限制为每种类型各含一个。注册声明 exp 设置过期日期和时间，zip 是公开声明，role 是私有声明，两者都使用自定义命名空间 weather，以尽量降低碰撞风险。

### 签名

有了头部和负载，我们就可以使用头部中指定的算法计算校验和，从而创建 JWT 签名。我们把头部和负载作为 Base64 编码的字符串，连同自定义密钥一起传给校验和函数。作为练习，我们将用 代码清单 9-10 中的代码以 TypeScript 创建签名。可以看到，为简单起见，这里的密钥是硬编码的；在生产代码中，这个密钥应存放在环境变量中。

把代码保存为 TypeScript 项目中的 index.ts，或者使用 npx ts-node index.ts 在本地运行。如果你愿意，也可以在 https://codesandbox.io 或 https://stackblitz.com 的 TypeScript 沙盒中运行。生成一个新密钥（https://www.usemodernfullstack.dev/generate-secret），用它替换代码清单中的密钥，看看令牌会如何变化。

```typescript
import {createHmac} from "crypto";

const base64UrlEncode = (data: string): string => {
    return Buffer.from(data, "utf-8").toString("base64");
};

const headerObject = {
    typ: "JWT",
    alg: "HS256"
};

const payloadObject = {
    exp: 234133423,
    weather_public_zip: "96815",
    weather_private_type: "GitHub"
};

const createJWT = () => {
    const base64Header = base64UrlEncode(JSON.stringify(headerObject));
    const base64Payload = base64UrlEncode(JSON.stringify(payloadObject));

    const secret = "59c4b48eac7e9ac37c046ba88964870d";

    const signature: string = createHmac("sha256", secret)
        .update(`${base64Header}.${base64Payload}`)
        .digest("hex");

        return [base64Header, base64Payload, signature].join(".");
};

console.log(createJWT());
```

*代码清单 9-10：为 OAuth2 天气服务计算 JWT 签名的 index.ts 文件*

我们使用 Node.js 标准的 crypto 模块，并创建了一个工具函数，通过缓冲区（Buffer）把 JSON 对象转换为 Base64 编码的字符串。接着，我们把字符串和密钥传给 crypto 模块的 createHmac 函数，以 sha256 作为哈希算法初始化 HMAC 对象；然后把以句点分隔的、Base64 编码的头部和负载字符串喂给 HMAC 对象；最后把结果转换为十六进制格式。

要得到 JWT 字符串，我们创建一个数组，其中包含来自头部和负载对象的 Base64 编码字符串，以及 Base64 编码的签名。然后以句点作为分隔符调用 Array.join，把数组转换成以句点分隔各部分的字符串，并返回得到的 JWT。

运行脚本即可生成 JWT。输出到控制台的最终 JWT 令牌应当与 代码清单 9-11 中的类似。

```json
eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJleHAiOjIzNDEzMzQyMywid2VhdGhlcl9wdWJsa
WNfemlwIjoiOTY4MTUiLCJ3ZWF0aGVyX3ByaXZhdGVfdHlwZSI6IkdpdEh1YiJ9.f667c81749886e
e01831376a38fbdba4d7f59a14c14f3a60e1bbee977c993ac9
```

*代码清单 9-11：OAuth2 天气服务最终的 JWT 令牌*

下一节，我们将运用新学的知识，实际走一遍 OAuth 流程。

## 练习 9：访问受保护资源

理解了 OAuth 的组件和授权码模式背后的理论之后，我们来做一个实际例子。我们将尝试访问 OAuth 服务器托管在 https://www.usemodernfullstack.dev/protected/resource 的受保护资源。请在终端中运行本练习的 cURL 命令，跟着一起操作。

首先，不携带访问令牌，尝试通过发送 GET 请求访问该受保护资源：

```bash
$ curl -i \
    -X GET 'https://www.usemodernfullstack.dev/protected/resource' \
    -H 'Accept: text/html'
--snip--
HTTP/2 401
Content-Type: text/html; charset=utf-8
--snip--
<h1>Unauthorized request: no authentication given</h1>
```

我们用 -i 标志输出响应头。在响应中查找 HTTP 状态码，看到的是 401，这表明我们无权访问该资源，必须先获取访问令牌。

为了拿到访问令牌，我们要先搭建一个 OAuth 客户端：创建一个用户账户并在提供方处注册，以获得客户端 ID 和客户端密钥。然后向 /oauth/authorize 端点发起请求，用该用户的凭证登录，并在我们的回调 URL 上接收授权许可。接着，在 /oauth/access_token 端点把授权码兑换成访问令牌。最后，再次发送同样的请求，并在请求头中提供访问令牌。

在这里，回调 URL 可以是任意 URL，因为我们不会真的向它发送数据。但在真实的授权许可流程中，它必须是你应用上的一个端点。这类端点通常由 OAuth SDK 提供，因为由它负责处理响应和令牌。

### 搭建客户端

在开始 OAuth 流程之前，我们需要创建一个用户并注册一个 OAuth 客户端。在浏览器中打开 https://www.usemodernfullstack.dev/register，在图 9-2 所示的表单中，用你选择的用户名和密码创建一个用户账户。

*图 9-2：在 OAuth 提供方处创建用户账户*

然后提供回调 URL，继续注册客户端（图 9-3）。这个回调 URL 指向我们应用上的 OAuth 回调端点。关于如何配置，SDK 或 OAuth 提供方通常会给出说明。

*图 9-3：在 OAuth 服务器上注册客户端应用以获取客户端凭证*

表单中预填了一个与典型 OAuth 回调结构相似的回调 URL，这类结构通常可以在 SDK 的文档里找到。不必担心你的应用上并不存在 http://localhost:3000/oauth/callback 这个 URL——本练习不会真的向它发送数据；稍后在走 API 调用的过程中你会看到，它只是请求与响应流程的一部分。点击按钮进入下一步，创建 OAuth 客户端。务必记下你的用户名、密码、客户端 ID 和客户端密钥，后续步骤都需要用到。然后点击 Register Your OAuth Client 完成流程。

### 登录以获取授权许可

现在，我们注册的用户必须用其凭证登录 OAuth 提供方，允许客户端应用访问其资源。我们调用 OAuth REST API 端点 /oauth/authorize，并以资源所有者的身份用用户凭证登录，这是流程的第一步。API 响应返回一个指向回调 URL 的重定向，其中 URL 参数 code 包含授权许可。

在真实应用中，资源所有者会点击某个“使用 OAuth 登录”按钮并输入凭证，API 调用在幕后自动完成。但为了本练习的目的，我们将手动执行所有 API 请求。通过原始的 API 调用，我们能看到通常被 SDK 抽象掉的操作。用下面的 cURL 命令直接调用 REST 端点：

```bash
$ curl -i \
    -X POST 'https://www.usemodernfullstack.dev/oauth/authenticate' \
    -H 'Accept: text/html' \
    -H 'Content-Type: application/x-www-form-urlencoded' \
    -d "response_type=code\
&client_id=<OAUTH_CLIENT_ID>\
&state=4nBjkh31\
&scope=read\
&redirect_uri=http://localhost:3000/oauth/callback\
&username=<OAUTH_USER>\
&password=<OAUTH_PASSWORD>"
--snip--
HTTP/2 302
Content-Type: text/html; charset=utf-8
location: http://localhost:3000/oauth/callback?code=<AUTHORIZATION_GRANT>&state=4nBjkh31
```

这个 POST 请求用于登录 OAuth 提供方。我们把 URL 设为 oauth/authenticate 端点，设置了 Accept 头，并为表单数据设置了相应的 Content-Type 头（application/x-www-form-urlencoded）。

我们用 -d 标志发送 POST 数据，表明我们想获取的是授权码。为了让 POST 数据分成易读的几段，需要用双引号（"）把它包裹起来，并用反斜杠（\）换行。我们加入了从 OAuth 提供方收到的客户端 ID，以及前面讨论过的回调 URL。scope（授权范围）参数指定我们请求的权限；state 参数包含一个唯一的随机字符串，用于缓解跨站请求伪造（cross-site request forgery, CSRF）攻击。OAuth 提供方应随授权码一起返回这个 state 参数，这样我们就能验证它的值没有变化，从而证明响应来自正确的 API 而非第三方。此外，我们还发送了之前注册的用户凭证。

响应头表明一切符合预期。OAuth API 返回状态码 302，并重定向到我们提供的回调 URL。如 location 头所示，重定向到回调 URL 的地址在 code 参数中包含授权许可，还有我们发送的 state 参数。与只是被原样回传的 state 不同，授权许可是唯一的，并且取决于请求数据。

### 用授权许可换取访问令牌

接下来，我们用授权许可向 OAuth 服务器请求访问令牌。复制上一步收到的 code，用它并携带客户端凭证，向 /oauth/access_token API 端点请求持有者访问令牌：

```bash
$ curl -i \
    -X POST 'https://www.usemodernfullstack.dev/oauth/access_token' \
    -H 'Accept: text/html, application/json' \
    -H 'Content-Type: application/x-www-form-urlencoded' \
    -d "code=<AUTHORIZATION_GRANT>\
&grant_type=authorization_code\
&redirect_uri=http://localhost:3000/oauth/callback\
&client_id=<OAUTH_CLIENT_ID>\
&client_secret=<OAUTH_CLIENT_SECRET>"
--snip--
HTTP/2 200 OK
Content-Type: application/json; charset=utf-8

{
    "access_token":"9bd55e2acf046128a54b76eada1ea6e0f909ca53",
    "token_type":"Bearer",
    "expires_in":3599,
    "refresh_token":"79a22d2b37c635a6095f5548ca08ea632deae573",
    "scope":"read"
}
```

这个发往 OAuth 服务器的 POST 请求用 Accept 头声明接受 JSON 响应，并把 Content-Type 头设为 POST 表单数据对应的值。表单数据本身通过 data-raw 标志发送，其中包含我们在 code 参数中收到的授权许可、一个告诉该 API 端点预期是授权许可流程的 grant_type（授权类型）参数，以及与之前相同的重定向 URL（redirect URL）。我们还传入了客户端 ID 和密钥。

响应的 HTTP 状态码为 200，表示请求成功。在响应体中，我们收到了访问令牌和其他详细信息。复制访问令牌的值，供下一步使用。

### 用访问令牌获取受保护资源

现在我们有了来自 OAuth 服务器的访问令牌，可以用它来获取本练习开头无法访问的那个受保护资源。使用与之前相同的 cURL 命令请求 https://www.usemodernfullstack.dev/protected/resource，并把 ACCESS_TOKEN 占位符替换为访问令牌：

```bash
$ curl -i \
    -X GET 'https://www.usemodernfullstack.dev/protected/resource' \
    -H 'Accept: text/html' \
    -H 'Authorization: Bearer <ACCESS_TOKEN>'
--snip--
HTTP/2 200 OK
Content-Type: text/html; charset=utf-8
--snip--
<h1>This page is secured.</h1>
--snip--
```

我们使用带 Bearer 关键字的 Authorization 头，并附上从授权许可流程中收到的访问令牌（即 access_token 属性中的值）。查找 HTTP 状态码，发现这次收到的不再是 401，而是 200。再仔细一看，响应体中也包含了受保护的内容。

我们手动走完了获得一个可用访问令牌所需的全部步骤。本练习仅适用于教学目的；正如本章前文所述，实际中我们通常使用 SDK 或 next-auth 这类库来实现 OAuth 流程。

## 总结

认证是指使用凭证来验证访问，而授权则定义并授予访问权限。本章介绍了如何用 OAuth2 协议实现授权。你熟悉了授权许可流程——全栈 Web 应用中最常用的 OAuth 流程——并学习了如何创建 JWT。随后，你练习了手动操作 OAuth：获取并使用 JWT 持有者令牌，并从全局视角了解了如何把 OAuth 流程应用到自己的应用中。

你可以在 https://oauth.net 找到更多资源、教程和规范。下一章将介绍 Docker——一个把你的开发环境与本地机器解耦的容器化平台。
