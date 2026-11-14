# 用 C 写一个简易 HTTP/1.1 协议

HTTP 看起来是网络数据，但最核心的部分只是约定好的文本格式：客户端发送请求行和请求头，服务器返回状态行、响应头和响应体。

本示例只实现教学所需的最小子集：

- 只解析 `HTTP/1.1` 请求行；
- 服务器只处理 `GET /` 和 `GET /hello`；
- 不读取文件、不拼接用户路径，避免把示例变成目录穿越漏洞；
- 其他方法返回 `405`，未知路径返回 `404`，格式错误返回 `400`；
- 服务器只监听 `127.0.0.1`，连接处理完立即关闭。

**注意：本页下面的三个 C 代码块就是源代码。** 单元测试不会维护另一份实现，而是先从本页提取源码，再进行编译和测试。

## 1. HTTP 报文长什么样

客户端请求：

```text
GET /hello HTTP/1.1\r\n
Host: 127.0.0.1\r\n
Connection: close\r\n
\r\n
```

服务器响应：

```text
HTTP/1.1 200 OK\r\n
Content-Type: text/plain; charset=utf-8\r\n
Content-Length: 13\r\n
Connection: close\r\n
\r\n
Hello, HTTP!\n
```

每一行以 `\r\n` 结束，空行表示请求头结束。`Content-Length` 使用**字节数**，不是字符数。

## 2. 协议层：解析请求并构造响应

协议层不直接操作 socket，只负责解析请求行、选择状态码和拼接响应。这样可以先用单元测试验证协议本身。

### `http_protocol.h`

```c
/* file: http_protocol.h */
#ifndef C_DOC_TEST_HTTP_PROTOCOL_H
#define C_DOC_TEST_HTTP_PROTOCOL_H

#include <stddef.h>

#define HTTP_METHOD_CAPACITY 16U
#define HTTP_TARGET_CAPACITY 256U
#define HTTP_VERSION_CAPACITY 16U

typedef struct {
    char method[HTTP_METHOD_CAPACITY];
    char target[HTTP_TARGET_CAPACITY];
    char version[HTTP_VERSION_CAPACITY];
} HttpRequest;

/* 解析 HTTP/1.1 请求行；成功返回 1，格式错误返回 0。 */
int http_parse_request(const char *data, size_t length, HttpRequest *request);

const char *http_reason_phrase(int status_code);

/* 构造带 Content-Length 的 HTTP/1.1 响应；成功返回 1。 */
int http_build_response(int status_code, const char *content_type,
                        const char *body, char *output, size_t capacity,
                        size_t *written);

#endif
```

### `http_protocol.c`

```c
/* file: http_protocol.c */
#include "http_protocol.h"

#include <stdio.h>
#include <string.h>

static int copy_token(char *destination, size_t capacity,
                      const char *source, size_t length) {
    if (length == 0 || length >= capacity) {
        return 0;
    }
    memcpy(destination, source, length);
    destination[length] = '\0';
    return 1;
}

int http_parse_request(const char *data, size_t length, HttpRequest *request) {
    if (data == NULL || request == NULL) {
        return 0;
    }

    const char *line_end = NULL;
    for (size_t i = 0; i + 1 < length; i++) {
        if (data[i] == '\r' && data[i + 1] == '\n') {
            line_end = data + i;
            break;
        }
    }
    if (line_end == NULL) {
        return 0;
    }

    const size_t line_length = (size_t)(line_end - data);
    const char *first_space = memchr(data, ' ', line_length);
    if (first_space == NULL || first_space == data) {
        return 0;
    }
    const size_t method_length = (size_t)(first_space - data);

    const char *target_start = first_space + 1;
    const size_t remaining_after_method = line_length - method_length - 1U;
    const char *second_space = memchr(target_start, ' ', remaining_after_method);
    if (second_space == NULL || second_space == target_start) {
        return 0;
    }
    const size_t target_length = (size_t)(second_space - target_start);
    const char *version_start = second_space + 1;
    const size_t version_length = (size_t)(line_end - version_start);

    if (memchr(version_start, ' ', version_length) != NULL ||
        target_start[0] != '/') {
        return 0;
    }
    if (!copy_token(request->method, sizeof request->method,
                    data, method_length) ||
        !copy_token(request->target, sizeof request->target,
                    target_start, target_length) ||
        !copy_token(request->version, sizeof request->version,
                    version_start, version_length)) {
        return 0;
    }
    return strcmp(request->version, "HTTP/1.1") == 0;
}

const char *http_reason_phrase(int status_code) {
    switch (status_code) {
    case 200:
        return "OK";
    case 400:
        return "Bad Request";
    case 404:
        return "Not Found";
    case 405:
        return "Method Not Allowed";
    case 500:
        return "Internal Server Error";
    default:
        return NULL;
    }
}

int http_build_response(int status_code, const char *content_type,
                        const char *body, char *output, size_t capacity,
                        size_t *written) {
    if (content_type == NULL || body == NULL || output == NULL ||
        written == NULL || capacity == 0) {
        return 0;
    }

    const char *reason = http_reason_phrase(status_code);
    if (reason == NULL) {
        return 0;
    }

    const size_t body_length = strlen(body);
    const int header_length = snprintf(
        output, capacity,
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n",
        status_code, reason, content_type, body_length);
    if (header_length < 0 || (size_t)header_length >= capacity) {
        return 0;
    }

    const size_t header_size = (size_t)header_length;
    if (body_length > capacity - header_size - 1U) {
        return 0;
    }
    memcpy(output + header_size, body, body_length);
    output[header_size + body_length] = '\0';
    *written = header_size + body_length;
    return 1;
}
```

## 3. socket 层：启动本地服务器

socket 层只负责接收字节、调用协议层、发送响应。下面的代码使用 POSIX socket，因此只能在 Linux/macOS 等 POSIX 环境验证。

### `http_server.c`

```c
/* file: http_server.c */
#include "http_protocol.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define HTTP_SERVER_REQUEST_CAPACITY 8192U
#define HTTP_SERVER_RESPONSE_CAPACITY 4096U

static int parse_port(const char *text, unsigned short *port) {
    char *end = NULL;
    unsigned long value = strtoul(text, &end, 10);
    if (text[0] == '\0' || end == text || *end != '\0' || value > 65535UL) {
        return 0;
    }
    *port = (unsigned short)value;
    return 1;
}

static int send_all(int client, const char *data, size_t length) {
    size_t sent = 0;
    while (sent < length) {
        ssize_t result = send(client, data + sent, length - sent, 0);
        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }
            return 0;
        }
        if (result == 0) {
            return 0;
        }
        sent += (size_t)result;
    }
    return 1;
}

static size_t find_header_end(const char *data, size_t length) {
    if (length < 4U) {
        return 0;
    }
    for (size_t i = 0; i <= length - 4U; i++) {
        if (memcmp(data + i, "\r\n\r\n", 4U) == 0) {
            return i + 4U;
        }
    }
    return 0;
}

static void handle_client(int client) {
    char request_data[HTTP_SERVER_REQUEST_CAPACITY];
    size_t received = 0;
    size_t header_end = 0;

    while (received < sizeof request_data) {
        ssize_t result = recv(client, request_data + received,
                              sizeof request_data - received, 0);
        if (result < 0 && errno == EINTR) {
            continue;
        }
        if (result <= 0) {
            break;
        }
        received += (size_t)result;
        header_end = find_header_end(request_data, received);
        if (header_end != 0) {
            break;
        }
    }

    int status = 400;
    const char *body = "Bad Request\n";
    if (header_end != 0) {
        HttpRequest request;
        if (http_parse_request(request_data, header_end, &request)) {
            if (strcmp(request.method, "GET") != 0) {
                status = 405;
                body = "Method Not Allowed\n";
            } else if (strcmp(request.target, "/") == 0) {
                status = 200;
                body = "Hello from C HTTP server\n";
            } else if (strcmp(request.target, "/hello") == 0) {
                status = 200;
                body = "Hello, HTTP!\n";
            } else {
                status = 404;
                body = "Not Found\n";
            }
        }
    }

    char response[HTTP_SERVER_RESPONSE_CAPACITY];
    size_t response_length = 0;
    if (http_build_response(status, "text/plain; charset=utf-8", body,
                            response, sizeof response, &response_length)) {
        (void)send_all(client, response, response_length);
    }
}

int main(int argc, char **argv) {
    unsigned short port = 8080;
    int once = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--once") == 0) {
            once = 1;
        } else if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            if (!parse_port(argv[++i], &port)) {
                fprintf(stderr, "invalid port\n");
                return 2;
            }
        } else {
            fprintf(stderr, "usage: %s [--port N] [--once]\n", argv[0]);
            return 2;
        }
    }

    (void)signal(SIGPIPE, SIG_IGN);
    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) {
        perror("socket");
        return 1;
    }

    int reuse = 1;
    (void)setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof reuse);
    struct sockaddr_in address;
    memset(&address, 0, sizeof address);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(port);
    if (bind(server, (struct sockaddr *)&address, sizeof address) < 0 ||
        listen(server, 8) < 0) {
        perror("bind/listen");
        close(server);
        return 1;
    }

    socklen_t address_length = sizeof address;
    if (getsockname(server, (struct sockaddr *)&address, &address_length) < 0) {
        perror("getsockname");
        close(server);
        return 1;
    }
    printf("PORT %u\n", (unsigned int)ntohs(address.sin_port));
    fflush(stdout);

    do {
        int client = accept(server, NULL, NULL);
        if (client < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("accept");
            close(server);
            return 1;
        }
        handle_client(client);
        close(client);
    } while (!once);

    close(server);
    return 0;
}
```

## 4. 从笔记源码构建并运行

源码提取脚本会把本页标记为 `file:` 的代码块写入临时目录：

```bash
cd MarkDown笔记/C笔记/c_doc_test
python3 tools/extract_http_sources.py --output-dir /tmp/c-http
cc -std=c11 -Wall -Wextra -Wpedantic -Werror \
  -I/tmp/c-http /tmp/c-http/http_server.c \
  /tmp/c-http/http_protocol.c -o /tmp/c-http/http_server
/tmp/c-http/http_server --port 8080
```

另开终端访问：

```bash
curl -i http://127.0.0.1:8080/
curl -i http://127.0.0.1:8080/hello
curl -i http://127.0.0.1:8080/missing
```

测试程序支持 `--once`，处理一个连接后退出，便于自动化验证：

```bash
/tmp/c-http/http_server --port 8080 --once
```

## 5. 验证重点

```bash
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
python3 tools/check_platform_examples.py --check
```

测试会验证：

| 请求 | 预期状态 | 预期响应体 |
|---|---:|---|
| `GET /` | 200 | `Hello from C HTTP server` |
| `GET /hello` | 200 | `Hello, HTTP!` |
| `GET /missing` | 404 | `Not Found` |
| `POST /` | 405 | `Method Not Allowed` |
| 格式错误请求 | 400 | `Bad Request` |

这个示例适合学习 HTTP 报文和 socket 流程；它没有实现 keep-alive、分块传输、TLS、并发、完整请求头解析和文件服务，因此不能直接用于生产环境。
