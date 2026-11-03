# 第 15 章 添加 OAuth

*Chapter 15: Adding OAuth*

在本章中，你将为 Food Finder 应用添加 OAuth 认证（authentication），让用户有机会使用自己的 GitHub 账号登录（log in）。你还将实现心愿单（wish list）页面，已认证的用户可以在其中添加和移除地点（location），并实现完成这一功能所需的按钮组件。最后，你将学习如何保护 GraphQL 变更（mutation）免受未认证用户的访问。

## 使用 next-auth 添加 OAuth

开发者通常会使用第三方库或 SDK 来实现 OAuth。对于 Food Finder 应用，我们将使用来自 Auth.js 的 next-auth 包，它附带了大量预配置的模板，让我们能够轻松连接到 OAuth 服务。这些模板被称为提供方（provider），我们将使用其中之一：GitHub provider，它会为我们的应用添加一个“Log In with GitHub”（使用 GitHub 登录）按钮。想要重温 OAuth 认证流程，可以回到第 9 章。

### 创建 GitHub OAuth 应用

首先，我们需要使用 GitHub 创建一个 OAuth 应用。这应该会为我们提供 Food Finder 应用连接 GitHub 所需的客户端 ID（client ID）和客户端密钥（client secret）。如果你还没有 GitHub 账号，请现在在 https://github.com 上创建一个，然后登录。前往 https://github.com/settings/developers，在 OAuth Apps 部分创建一个新的 OAuth 应用。在随之出现的表单中填入 Food Finder 应用的详细信息，该表单应当与图 15-1 类似。

*图 15-1：GitHub 中用于添加新 OAuth 应用的用户界面*

名称填写 Food Finder，主页 URL（home page URL）设置为 http://localhost:3000/，授权回调 URL（authorization callback URL）设置为 http://localhost:3000/api/auth/callback/github。注册应用后，GitHub 应该会显示客户端 ID，并允许我们生成客户端密钥。

### 添加客户端凭证

现在，把这些凭证（credentials）以 GITHUB_CLIENT_ID 和 GITHUB_CLIENT_SECRET 的形式复制到应用代码根目录下的 env.local 文件中。该文件如下所示：

```bash
MONGO_URI=mongodb://backend:27017/foodfinder
GITHUB_CLIENT_ID=ADD_YOUR_CLIENT_ID_HERE
GITHUB_CLIENT_SECRET=ADD_YOUR_CLIENT_SECRET_HERE
```

用你自己的凭证填入这些占位符。

### 安装 next-auth

要把 Auth.js 的 OAuth SDK next-auth 添加到 Food Finder 应用，并将其配置为连接提供方，请运行以下命令：

```bash
$ docker exec -it foodfinder_application npm install next-auth
```

默认情况下，该 SDK 使用加密的 JWT（JSON Web Token）来存储会话（session）信息，并把会话信息附加到 API 请求上。只要我们为它提供一个密钥（secret），该库就会自动处理加密和解密。要添加这样的密钥，请打开 env.local 文件，并在末尾添加下面这一行：

```bash
NEXTAUTH_SECRET=78f6cc4bf633b1102f4ca4d72602c60f
```

你可以使用任何自己喜欢的密钥。这里使用的字符串是通过 OpenSSL 在 https://www.usemodernfullstack.dev/api/v1/generate-secret 上随机生成的，你应该为每个应用使用一个新的密钥。

### 创建认证回调

现在，我们来开发 api/auth 路由，它对应注册 OAuth 应用时提供给 GitHub 的授权回调 URL。在 pages/api 目录下创建 auth 文件夹，并在其中创建文件 [...nextauth].ts。文件名中的 ... 告诉 Next.js 路由器这是一个通配路由（catch-all），也就是说，它会处理所有发往 /auth 之下端点的 API 调用；例如 auth/signin 或 auth/callback/github。把清单 15-1 中的代码添加到该文件中。

```typescript
import GithubProvider from "next-auth/providers/github";
import {NextApiRequest, NextApiResponse} from "next";
import NextAuth from "next-auth";
import {createHash} from "crypto";

const createUserId = (base: string): string => {
    return createHash("sha256").update(base).digest("hex");
};

export default async function auth(req: NextApiRequest, res: NextApiResponse) {
    return await NextAuth(req, res, {
        providers: [
            GithubProvider({
                clientId: process.env.GITHUB_CLIENT_ID || " ",
                clientSecret: process.env.GITHUB_CLIENT_SECRET || " ",
            }),

        ],
        callbacks: {
            async jwt({token}) {
                if (token?.email && !token.fdlst_private_userId) {
                    token.fdlst_private_userId = createUserId(token.email);
                }
                return token;
            },
            async session({session}) {
                if (
                    session?.user?.email &&
                    !session?.user.fdlst_private_userId
                ) {
                    session.user.fdlst_private_userId = createUserId(
                        session?.user?.email
                    );
                }
                return session;
            },
        },
    });
}
```

*清单 15-1：pages/api/auth/[...nextauth].ts 文件*

我们导入所需的依赖，包括内置的 GithubProvider 和默认的 crypto 模块。然后，我们创建一个简单的 createUserId 函数，它接受一个字符串作为参数，并调用 crypto 模块的 createHash 函数，从这个字符串返回经过哈希的用户 ID。

接下来，我们创建并导出默认的异步 auth 函数。为此，我们初始化 NextAuth 模块，并把 GithubProvider 添加到 providers 数组中。我们将其配置为使用存储在环境变量中的 clientId 和 clientSecret。

由于我们希望让应用尽可能简单，我们将保持无状态（stateless）；因此，我们使用 jwt 和 session 回调，next-auth 每次在内部创建新会话或 JWT 时都会用到它们。在回调中，我们使用自己的 createId 函数从用户的电子邮件计算出哈希后的用户 ID（如果当前的 token 或 session 对象中还没有该 ID）。最后，我们把它存储在一个私有声明（private claim）中。

我们在 next-auth 会话的 user 对象上刚刚创建了一个新属性 fdlst_private_userId。不出所料，TSC 会警告我们该属性在 Session 类型上不存在。我们需要通过调整应用根目录下的 customs.d.ts 文件来扩充（augment）该类型的接口，使其与清单 15-2 一致。

```typescript
import mongoose from "mongoose";
import {DefaultSession} from "next-auth";

declare global {
    var mongoose: mongoose;
}

declare module "next-auth" {
    interface Session {
        user: {
            fdlst_private_userId: string;
        } & DefaultSession["user"];
    }
}
```

*清单 15-2：更新后的 customs.d.ts 文件，扩充了 Session 接口*

在更新后的代码中，我们导入了 next-auth 包的 DefaultSession（它定义了默认的会话对象），然后创建并重新声明 Session 接口的 user 对象，加入新的 fdlst_private_userId 属性。由于 TypeScript 会覆盖已有的 user 对象，我们显式地从 DefaultSession 对象中把它加回来。换句话说，我们把新的 fdlst_private_userId 属性添加到了 Session 接口中。

### 跨页面与组件共享会话

回调 URL 设置好之后，我们需要确保用户的会话在所有 Next.js 页面和 React 组件之间共享。我们可以使用第 4 章讨论过的 useContext 钩子（hook），next-auth 为我们提供了它。在 pages/_app.tsx 文件中，把应用包裹在 SessionProvider 中，如清单 15-3 所示。

```tsx
import "../styles/globals.css";
import "../styles/layout.css";
import type {AppProps} from "next/app";
import Layout from "components/layout";
import {SessionProvider} from "next-auth/react";

export default function App({
    Component, pageProps: {session, ...pageProps}}: AppProps) {
    return (
        <SessionProvider session={session}>
            <Layout>
                <Component {...pageProps} />
            </Layout>
        </SessionProvider>
    );
}
```

*清单 15-3：修改后的 pages/_app.tsx 文件*

我们从 next-auth 包导入 SessionProvider，并用 session 对象增强 pageProps。我们把当前会话存储在 provider 的 session 属性中，使其在整个 Next.js 应用中都可用。

在我们能够在前端和中间件（middleware）中访问会话之前，需要添加带有 Sign In（登录）按钮的认证元素（auth-element），让用户能够登录。

## 通用按钮组件

是时候实现我们之前提到的通用按钮组件了。从技术上讲，这个组件将是一个通用的 div 元素，我们会通过样式让它看起来像一个按钮，并带有几种变体。它将用作认证元素中的 Sign In/Sign Out（登录/登出）按钮，以及地点详情组件中的 Add To/Remove From（添加到/移出）按钮。在 components 文件夹中新建一个 button 文件夹，在其中添加一个包含清单 15-4 代码的 index.module.css 文件，以及一个 index.tsx 文件。

```css
.root {
    align-items: center;
    border-radius: 5px;
    color: #1d1f21;
    cursor: pointer;
    display: inline-flex;
    font-weight: 500;
    height: 35px;
    letter-spacing: 0;
    margin: 0;
    overflow: hidden;
    place-content: flex-start;
    position: relative;
    white-space: nowrap;
}

.root > a,
.root > span {
    padding: 0 1rem;
    white-space: nowrap;
}

.root {
    transition: border-color 0.25s ease-in, background-color 0.25s ease-in,
        color 0.25s ease-in;
    will-change: border-color, background-color, color;
}

.root.default,
.root.default:link,
.root.default:visited {
    background-color: transparent;
    border: 1px solid transparent;
    color: #1d1f21;
}

.root.default:hover,
.root.default:active {
    background-color: transparent;
    border: 1px solid #dbd8e3;
    color: #1d1f21;
}

.root.blue,
.root.blue:link,
.root.blue:visited {
    background-color: rgba(0, 118, 255, 0.9);
    border: 1px solid rgba(0, 118, 255, 0.9);
    color: #fff;
    text-decoration: none;
}

.root.blue:hover,
.root.blue:active {
    background-color: transparent;
    border: 1px solid #1d1f21;
    color: #1d1f21;
    text-decoration: none;
}

.root.outline,
.root.outline:link,
.root.outline:visited {
    background-color: transparent;
    border: 1px solid #dbd8e3;
    color: #1d1f21;
    text-decoration: none;
}

.root.outline:hover,
.root.outline:active {
    background-color: transparent;
    border: 1px solid rgba(0, 118, 255, 0.9);
    color: rgba(0, 118, 255, 0.9);
    text-decoration: none;
}

.root.disabled,
.root.disabled:link,
.root.disabled:visited {
    background-color: transparent;
    border: 1px solid #dbd8e3;
    color: #dbd8e3;
    text-decoration: none;
}

.root.disabled:hover,
.root.disabled:active {
    background-color: transparent;
    border: 1px solid #dbd8e3;
    color: #dbd8e3;
    text-decoration: none;
}
```

*清单 15-4：components/button/index.module.css 文件*

我们为想要创建的每种按钮变体添加样式。所有按钮的高度都是 35 像素，并且带有圆角。我们定义了一个默认样式、一个蓝色背景白色文字的变体，以及一个背景为白色的描边（outline）版本。此外，我们还为停用（disabled）状态的按钮定义了样式。

样式就绪后，我们就可以编写组件的代码了。把清单 15-5 的内容复制到组件的 index.tsx 文件中。

```tsx
import React from "react";
import styles from "./index.module.css";

interface PropsInterface {
    disabled?: boolean;
    children?: React.ReactNode;
    variant?: "blue" | "outline";
    clickHandler?: () => any;
}

const Button = (props: PropsInterface): JSX.Element => {
    const {children, variant, disabled, clickHandler} = props;

    const renderContent = (children: React.ReactNode) => {
        if (disabled) {
            return (
                <span className={styles.span}>
                    {children}
                </span>
            );
        } else {
            return (
                <span className={styles.span} onClick={clickHandler}>
                    {children}
                </span>
            );
        }
    };

    return (
        <div
            className={[
                styles.root,
                disabled ? styles.disabled : " ",
                styles[variant || "default"],
            ].join(" ")}
        >
            {renderContent(children)}
        </div>
    );
};

export default Button;
```

*清单 15-5：components/button/index.tsx 文件*

导入依赖之后，我们为组件的 prop 参数定义接口。接着，我们把 Button 组件定义为一个返回 JSX 元素的函数，然后使用对象解构（object-destructuring）语法把 props 对象拆分成代表该对象键值对的常量。我们定义了内部的 renderContent 函数，它接受一个类型为 ReactNode 的参数 children，并将其包裹在 span 元素中渲染。根据 disabled 属性的状态，我们还会添加来自 props 对象的点击处理器（click handler）。组件本身返回一个我们通过样式使其看起来像按钮的 div。

## AuthElement 组件

尽管我们已经把 next-auth 包添加到了项目中、创建了 OAuth API 路由并配置好了 OAuth 提供方，我们仍然无法访问会话信息，因为还没有登录按钮。让我们创建这个 AuthElement 组件，然后把它添加到页头中。该组件使用我们的默认按钮组件，并且一旦用户登录，它就会显示用户的全名，以及指向其心愿单的链接。

在 components/header 目录下创建 auth-element 文件夹，然后添加包含清单 15-6 代码的 index.module.css 文件。

```css
.root {
    align-items: center;
    display: flex;
    justify-content: space-between;
    margin: 0;
    padding: 1rem 0;
    width: auto;
}

.root > * {
    margin: 0 0 0 2rem;
}

.name {
    margin: 1rem 0 0 0;
}

@media (min-width: 600px) {
    .name {
        margin: 0 0 0 1rem;
    }
}
```

*清单 15-6：components/header/auth-element/index.module.css 文件*

我们为该组件定义了一组基础样式，使用 flexbox 和外边距让它们垂直对齐，并针对较小的屏幕更改了它们的布局。

要编写组件本身，请在 auth-element 文件夹中添加一个 index.tsx 文件，并在其中输入清单 15-7 中的代码。

```tsx
import Link from "next/link";
import {signIn, signOut, useSession} from "next-auth/react";
import Button from "components/button";
import styles from "./index.module.css";

const AuthElement = (): JSX.Element => {
    const {data: session, status} = useSession();

    return (
        <>
            {status === "authenticated" (
                <span className={styles.name}>
                    Hi <b>{session?.user?.name}</b>
                </span>
            )}

            <nav className={styles.root}>
                {status === "authenticated" && (
                    <>
                        <Button variant="outline">
                            <Link
                                href={`/list/${session?.user.fdlst_private_userId}`}
                            >
                                Your wish list
                            </Link>
                        </Button>

                        <Button variant="blue" clickHandler={() => signOut()}>
                            Sign out
                        </Button>
                    </>
                )}
                {status == "unauthenticated" && (
                    <>
                        <Button variant="blue" clickHandler={() => signIn()}>
                            Sign in
                        </Button>
                    </>
                )}
            </nav>
        </>
    );
};
export default AuthElement;
```

*清单 15-7：components/header/auth-element/index.tsx 文件*

最值得注意的导入是 next-auth 中的 signIn 和 signOut 函数，以及 useSession 钩子。后者让我们能够轻松访问会话信息，而那两个函数分别用于触发登录流程和终止会话。

接着，我们定义 AuthElement 组件，并从 useSession 钩子获取会话数据和会话状态。我们需要这两者来构造从组件返回的 JSX 元素。在客户端（client side），我们可以直接通过 useSession 钩子访问会话信息；而在服务器端（server side），我们需要通过 JWT 来访问它，因为会话信息是 API 请求的 HTTP cookie 的一部分。

当会话的状态为 authenticated（已认证）时，我们根据会话数据渲染用户的名称，并把 Your Wish List（你的心愿单）和 Sign Out（登出）按钮添加到导航的 nav 元素中。否则，我们添加 Sign In 按钮来启动 OAuth 流程。对于所有这些按钮，我们都使用通用按钮组件以及从 next-auth 模块导入的 signIn 和 signOut 函数，它们都会自动处理 OAuth 流程。

我们使用 next/link 元素链接到用户的心愿单。（这是我们稍后会实现的另一个 Next.js 页面。）心愿单位于动态路由 /list/:userId，它使用我们通过对用户电子邮件地址进行哈希计算得到、并存储在 fdlst_private_userId 中的用户 ID。

## 把 AuthElement 组件添加到页头

现在，我们必须把新组件添加到页头中。打开 components/header 目录下的 index.tsx 文件，调整它使其与清单 15-8 一致。

```tsx
import styles from "./index.module.css";
import Logo from "components/header/logo";
import AuthElement from "components/header/auth-element";
const Header = (): JSX.Element => {
    return (
        <header className={styles.root}>
            <div className="layout-grid">
                <Logo />
                <AuthElement />
            </div>
        </header>
    );
};

export default Header;
```

*清单 15-8：修改后的 components/header/index.tsx 文件*

这个更新很简单：我们导入 AuthElement 组件，并把它添加到页头中 Logo 的旁边。

测试一下 OAuth 工作流，看看我们的会话管理在实践中的效果。打开 http://localhost:3000 时，页头中应该会出现 Sign In 按钮，如图 15-2 所示。

*图 15-2：未登录状态下的页头，带有 Sign In 按钮*

让我们用 OAuth 登录。点击 Sign In 按钮，next-auth 应该会把你重定向到登录界面，在那里你可以选择使用已配置的某个 OAuth 提供方登录（图 15-3）。

*图 15-3：OAuth 要求我们选择一个提供方*

点击按钮进行登录。OAuth 应该会把你重定向回应用，AuthElement 会根据会话信息渲染出你的名字和新的按钮。此时屏幕应当与图 15-4 类似。

*图 15-4：已登录状态下的页头，带有会话信息*

页头元素已根据会话的状态发生了变化。我们显示了从 OAuth 提供方获得的当前用户的名称、指向其公开心愿单的链接，以及 Sign Out 按钮。

## 心愿单 Next.js 页面

页头中的心愿单按钮应当链接到位于动态 URL list/:userId 的心愿单页面。这个常规的 Next.js 页面应当显示所有 on_wishlist 属性包含动态 URL 中指定用户 ID 的地点。它看起来会与起始页十分相似，我们可以用现有的组件来构建它。

要创建该页面的路由，请在 pages 目录下创建 list 文件夹，并在其中创建 [userId].tsx 文件。然后把清单 15-9 中的代码添加到这个 .tsx 文件中。

```tsx
import type {
    GetServerSideProps,
    GetServerSidePropsContext,
    NextPage,
    PreviewData,
    InferGetServerSidePropsType,
} from "next";
import Head from "next/head";
import {ParsedUrlQuery} from "querystring";

import dbConnect from "middleware/db-connect";
import {onUserWishlist} from "mongoose/locations/services";
import {LocationType} from "mongoose/locations/schema";
import LocationsList from "components/locations-list";

import {useSession} from "next-auth/react";

const List: NextPage = (
    props: InferGetServerSidePropsType<typeof getServerSideProps>
) => {
    const locations: LocationType[] = JSON.parse(props.data?.locations);
    const userId: string | undefined = props.data?.userId;
    const {data: session} = useSession();
    let title = `The Food Finder- A personal wish list`;
    let isCurrentUsers =
        userId && session?.user.fdlst_private_userId === userId;
    return (
        <div>
            <Head>
                <title>{title}</title>
                content={`The Food Finder. A personal wish list.`}
            </Head>
            <h1>
                {isCurrentUsers ? " Your " : " A "}
                wish list!
            </h1>
            {isCurrentUsers && locations?.length === 0 && (
                <>
                    <h2>Your list is currently empty! :(</h2>
                    <p>Start adding locations to your wish list!</p>
                </>
            )}
            <LocationsList locations={locations} />
        </div>
    );
};

export const getServerSideProps: GetServerSideProps = async (
    context: GetServerSidePropsContext<ParsedUrlQuery, PreviewData>
) => {
    let {userId} = context.query;
    let locations: LocationType[] | [] = [];
    try {
        await dbConnect();
        locations = await onUserWishlist(userId as string);
    } catch (err: any) {}
    return {
        // 这些 props 将由页面组件接收
        props: {
            data: {locations: JSON.stringify(locations), userId: userId},
        },
    };
};
export default List;
```

*清单 15-9：pages/list/[userId].tsx 文件*

尽管我们希望心愿单页面看起来与起始页相似，我们还是像地点详情页那样，使用 SSR（服务器端渲染）配合 getServerSideProps。心愿单页面的动态性很强；因此，我们需要在每次请求时重新生成 HTML。

另一种方法是使用客户端渲染（client-side rendering），然后在 useEffect 钩子中通过 GraphQL API 请求用户的地点。然而，这会导致用户每次打开心愿单页面时都看到加载画面。使用 SSR 可以完全避免这种较差的用户体验。

在页面代码的服务器端部分，我们首先从 context 的 query 对象中提取 URL 参数 userId。我们使用这个用户 ID 和 onUsersWishlist 服务来获取用户心愿单上的所有地点。如果出现错误，我们只是继续执行，而不重定向到 404 错误页面，此时会渲染一个空列表。

然后，我们把 locations 数组和用户 ID 传递给 Next.js 页面，在页面中我们像往常一样提取 locations 以及 userId。我们把来自 URL 的用户 ID 与当前会话中的用户 ID 进行比较。如果两者匹配，我们就知道当前登录的用户访问的是自己的心愿单，并相应地调整用户界面。

## 把按钮添加到地点详情组件

现在我们可以访问心愿单页面了，但它将始终是空的。我们还没有为用户提供向其中添加条目的途径。为了改变这一点，我们将在地点详情组件中放置一个按钮，让用户可以添加或移除某个特定地点。我们将使用通用按钮组件和会话信息。打开 components/location-details.tsx 目录下的 index.ts 文件，修改代码使其与清单 15-10 一致。

```tsx
import {LocationType} from "mongoose/locations/schema";
import styles from "./index.module.css";

import {useSession} from "next-auth/react";
import {useEffect, useState} from "react";
import Button from "components/button";

interface PropsInterface {
    location: LocationType;
}

interface WishlistInterface {
    locationId: string;
    userId: string;
}

const LocationDetail = (props: PropsInterface): JSX.Element => {
    let location: LocationType = props.location;

    const {data: session} = useSession();
    const [onWishlist, setOnWishlist] = useState<Boolean>(false);
    const [loading, setLoading] = useState<Boolean>(false);

    useEffect(() => {
        let userId = session?.user.fdlst_private_userId;
        setOnWishlist(
            userId && location.on_wishlist.includes(userId) ? true : false
        );
    }, [session]);

    const wishlistAction = (props: WishlistInterface) => {

        const {locationId, userId} = props;

        if (loading) {return false;}
        setLoading(true);

        let action = !onWishlist ? "addWishlist" : "removeWishlist";

        fetch("/api/graphql", {
            method: "POST",
            headers: {
                "Content-Type": "application/json",
            },
            body: JSON.stringify({
                query: `mutation wishlist {
                    ${action}(
                        location_id: "${locationId}",
                        user_id: "${userId}"
                    ) {
                        on_wishlist
                    }
                }`,
            }),
        })
        .then((result) => {
            if (result.status === 200) {
                setOnWishlist(action === "addWishlist" ? true : false);
            }
        })
        .finally(() => {
            setLoading(false);
        });
    };

    return (
        <div>
            {location && (
                <ul className={styles.root}>
                    <li>
                        <b>Address: </b>
                        {location.address}
                    </li>
                    <li>
                        <b>Zipcode: </b>
                        {location.zipcode}
                    </li>
                    <li>
                        <b>Borough: </b>
                        {location.borough}
                    </li>
                    <li>
                        <b>Cuisine: </b>
                        {location.cuisine}
                    </li>
                    <li>
                        <b>Grade: </b>
                        {location.grade}
                    </li>
                </ul>
            )}

            {session?.user.fdlst_private_userId && (
                <Button
                    variant={!onWishlist ? "outline" : "blue"}
                    disabled={loading ? true : false}
                    clickHandler={() =>
                        wishlistAction({
                            locationId: session?.user.fdlst_private_userId,
                            userId: session?.user?.userId,
                        })
                    }
                >
                    {onWishlist && <>Remove from your Wishlist</>}
                    {!onWishlist && <>Add to your Wishlist</>}
                </Button>
            )}

        </div>
    );
};
export default LocationDetail;
```

*清单 15-10：修改后的 components/location-details/index.tsx 文件*

首先，我们从 next-auth 导入 useSession，从 React 导入 useEffect 和 useState，以及通用 Button 组件。然后，我们定义 WishlistInterface，也就是稍后要实现的 wishlistAction 函数的接口。

在组件内部，我们从 useSession 钩子获取会话，然后用 useState 创建布尔（Boolean）类型的状态变量 onWishlist 和 loading。我们用第一个状态变量来指定某个地点当前是否在用户的心愿单上，并据此更新用户界面。我们在 useEffect 钩子中根据地点的 on_wishlist 属性计算初始状态。一旦我们成功地把地点添加到心愿单或从心愿单移除，就更新该状态变量和按钮的文本。

我们实现 wishlistAction 函数来更新 on_wishlist 属性。首先解构参数对象，然后检查 loading 状态，看当前是否有请求正在运行。如果有，就退出函数；否则，把 loading 状态设为 true 以阻塞用户界面，计算出 GraphQL 变更所需的动作（action），并用它调用 wishlist 变更。在成功修改数据库中的文档之后，我们更新 onWishlist 状态并解除用户界面的阻塞。

我们检查当前会话，看用户是否已登录。如果是，就渲染 Button 组件，并根据 loading 状态设置 disabled 和类名（class name）属性，以及一个点击事件。按钮每被点击一次，我们就以当前地点 ID 和用户 ID 作为参数调用 wishlistAction 函数。最后，我们根据 onWishlist 状态设置按钮的文本：把当前地点添加到心愿单，或者把它从心愿单移除。

在继续往下之前，试着向心愿单添加几个地点，再移除几个。检查按钮文本是否会相应地变化，以及心愿单页面上是否会出现一个与起始页上类似的地点列表。

## 保护 GraphQL 变更

要完成这个应用，我们还有最后一件事要做：保护 GraphQL API。查询（query）应当公开可用，而变更应当只有已登录的用户才能访问，并且这些用户只能针对 on_wishlist 属性添加或移除自己的用户 ID。

但如果你用 curl 命令测试这个 API，就会看到目前任何人都可以访问它。注意，-d 标志所带的值必须写在同一行上，否则服务器可能会返回错误：

```bash
$ curl -v \
    -X POST \
    -H "Accept: application/json" \
    -H "Content-Type: application/json" \
    -d '{"query":"mutation wishlist {removeWishlist(location_id: \"12340\",
 user_id: \"exampleid\") {on_wishlist}}"}' \
    http://localhost:3000/api/graphql

< HTTP/1.1 200 OK
<
{"data":{"removeWishlist":{"on_wishlist":[]}}}
```

作为测试，我们发送一个简单的变更，把 ID 为 12340 的地点从一个不存在的用户的心愿单中移除。（这个变更不会成功，不过没关系；我们只是想验证这个 API 是否对公众开放。）该命令收到了 200 响应和预期的 JSON，证明这些变更是公开的。

让我们实现一个 authGuard 来保护我们的变更。守卫（guard）是一种先检查某个条件、如果条件不满足就抛出错误的模式，而认证守卫（auth guard）用于保护路由或 API 免受未授权访问。

我们首先在 middleware 文件夹中创建文件 auth-guards.ts，并添加清单 15-11 中的代码。

```typescript
import {GraphQLError} from "graphql/error";
import {JWT} from "next-auth/jwt";

interface paramInterface {
    user_id: string;
    location_id: string;
}
interface contextInterface {
    token: JWT;
}

export const authGuard = (
    param: paramInterface,
    context: contextInterface
): boolean | Error => {

  if (!context || !context.token || !context.token.fdlst_private_userId) { // 1
        return new GraphQLError("User is not authenticated", {
            extensions: {
                http: {status: 500},
                code: "UNAUTHENTICATED",
            },
        });
    }

  if (context?.token?.fdlst_private_userId !== param.user_id) { // 2
        return new GraphQLError("User is not authorized", {
            extensions: {
                http: {status: 500},
                code: "UNAUTHORIZED",
            },
        });
    }
    return true;
};
```

*清单 15-11：middleware/auth-guards.ts 文件*

我们还从 next-auth 导入了 JWT，并从 graphql 导入了 GraphQLError 构造函数。后者将用于在认证失败时创建返回给用户的错误对象。接下来，我们为 authGuard 函数的参数定义接口，并导出该函数本身。

我们会从变更的解析器（resolver）中调用认证守卫，传入两个参数：一个包含用户 ID 和地点 ID 的对象（为此我们定义了 paramInterface），以及包含令牌（token）的 context 对象（contextInterface）。认证守卫要么返回一个表示认证成功的布尔值，要么返回一个错误。在 authGuard 函数中，我们验证对变更的每次访问都带有包含私有声明的令牌（标记 1），并且私有声明中的用户 ID 与我们传给变更的用户 ID 一致（标记 2）。换句话说，我们验证发起 API 请求的用户已登录，并且正在修改的是他们自己的心愿单。

如果检查失败，我们就创建一个带有消息和代码的错误。此外，我们把 HTTP 状态码设置为 500。请记住，与依赖一长串精确的 HTTP 状态码来与调用方通信的 REST API 不同，GraphQL API 通常只用 200 或 500 作为错误的状态码。大致来说，当 GraphQL 完全无法执行查询时，我们发送 500 状态码；当查询可以执行时，则发送 200。在这两种情况下，GraphQL API 都应当包含关于所发生错误的精确信息。

现在，我们必须把用户的 OAuth 令牌传给解析器，解析器再把它传给认证守卫。为此，我们将使用在 startServerAndCreateNextHandler 函数中实现的 context 函数，该函数位于 pages/api/graphql.ts 文件中。打开该文件，将其调整为与清单 15-12 中的代码一致。

```typescript
import {ApolloServer, BaseContext} from "@apollo/server";
import {startServerAndCreateNextHandler} from "@as-integrations/next";

import {resolvers} from "graphql/resolvers";
import {typeDefs} from "graphql/schema";
import dbConnect from "middleware/db-connect";

import {NextApiHandler, NextApiRequest, NextApiResponse} from "next";

import {getToken} from "next-auth/jwt";

const server = new ApolloServer<BaseContext>({
    resolvers,
    typeDefs,
});

const handler = startServerAndCreateNextHandler(server, {
    context: async (req: NextApiRequest) => {
        const token = await getToken({req});
        return {token};
    },
});

const allowCors =
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

const connectDB =
    (fn: NextApiHandler) =>
    async (req: NextApiRequest, res: NextApiResponse) => {
        await dbConnect();
        return await fn(req, res);
    };

export default connectDB(allowCors(handler));
```

*清单 15-12：修改后的 pages/api/graphql.ts 文件，带有 JWT 令牌*

在客户端，我们可以直接通过 useSession 钩子访问会话信息；与之不同，在服务器端我们需要通过 JWT 来访问它。这是因为在服务器上，会话信息是 API 请求的 HTTP cookie 的一部分，而不是 SessionProvider 的共享会话状态，我们需要从请求中把它提取出来。为此，我们从 next-auth 的 jwt 模块导入 getToken 函数。然后，把从 context 函数接收到的请求对象传给 getToken 调用，并等待解码后的 JWT。接下来，我们从 context 函数返回该令牌，这样就可以在解析器函数中访问它了。

最后，让我们使用该令牌把 authGuard 添加到解析器中，保护它们免受未认证和未授权的访问。打开 graphql/locations/mutations.ts 文件，用清单 15-13 中的代码更新它。

```typescript
import {updateWishlist} from "mongoose/locations/services";
import {authGuard} from "middleware/auth-guard";
import {JWT} from "next-auth/jwt";

interface UpdateWishlistInterface {
    user_id: string;
    location_id: string;
}

interface contextInterface {
    token: JWT;
}

export const locationMutations = {
    removeWishlist: async (
        _: any,
        param: UpdateWishlistInterface,
        context: contextInterface
    ) => {

        const guard = authGuard(param, context);
        if (guard !== true) {return guard;}

        return await updateWishlist(param.location_id, param.user_id, "remove");
    },

    addWishlist: async (
        _: any,
        param: UpdateWishlistInterface,
        context: contextInterface
    ) => {

        const guard = authGuard(param, context);
        if (guard !== true) {return guard;}

        return await updateWishlist(param.location_id, param.user_id, "add");
    },
};
```

*清单 15-13：添加了 authGuard 的 graphql/locations/mutations.ts 文件*

我们为 context 定义了一个新接口，并更新 context 参数使其包含 JWT。接着，我们把 authGuard 函数添加到变更中，遵循守卫模式：立即返回错误，而不继续执行后面的代码。

要测试 authGuard 的功能，请再次运行 curl。命令行输出应当与清单 15-14 类似。

```bash
$ curl -v \
    -X POST \
    -H "Accept: application/json" \
    -H "Content-Type: application/json" \
    -d '{"query":"mutation wishlist {removeWishlist(location_id: \"12340\",
  user_id: \"exampleid\") {on_wishlist}}"}' \
  http://localhost:3000/api/graphql

< HTTP/1.1 500 Internal Server Error
<
{
    "errors":[
        {
            "message":"User is not authenticated",
            "locations": [{"line":1,"column":20}],
            "path": ["removeWishlist"],
            "extensions": {"code":"UNAUTHENTICATED","data":null}
        }
    ]
}
```

*清单 15-14：用于测试我们 API 的 curl 命令*

与之前的 curl 调用不同，GraphQL API 现在以 HTTP/1.1 500 Internal Server Error 和一条详细的错误消息作为响应，这条消息是我们在 auth-guards.ts 文件中创建 GraphQLError 时定义的。

## 总结

我们已成功地为 Food Finder 应用添加了 OAuth 认证流程。现在，用户可以使用自己的 GitHub 账号登录。登录之后，他们可以维护自己的个人公开心愿单。此外，我们还保护了 GraphQL 变更，这意味着它们不再对所有人开放；相反，只有已登录的用户才能访问它们。在最后一章中，我们将添加自动化测试，使用 Jest 来评估这个应用。
