#include "test_common.h"

#include <string.h>

#include "http_protocol.h"

void test_http_protocol(void) {
    HttpRequest request;
    const char *raw_request =
        "GET /hello HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    TEST_ASSERT(http_parse_request(raw_request, strlen(raw_request), &request));
    TEST_ASSERT(strcmp(request.method, "GET") == 0);
    TEST_ASSERT(strcmp(request.target, "/hello") == 0);
    TEST_ASSERT(strcmp(request.version, "HTTP/1.1") == 0);

    const char *invalid_requests[] = {
        "GET /hello HTTP/1.0\r\n\r\n",
        "GET /hello\r\n\r\n",
        "GET hello HTTP/1.1\r\n\r\n",
        "GET /hello HTTP/1.1 extra\r\n\r\n",
        "GET  HTTP/1.1\r\n\r\n",
        "GET /hello HTTP/1.1",
    };
    for (size_t i = 0; i < sizeof invalid_requests / sizeof invalid_requests[0];
         i++) {
        TEST_ASSERT(!http_parse_request(invalid_requests[i],
                                        strlen(invalid_requests[i]), &request));
    }
    TEST_ASSERT(!http_parse_request(NULL, 0, &request));
    TEST_ASSERT(!http_parse_request(raw_request, strlen(raw_request), NULL));

    TEST_ASSERT(strcmp(http_reason_phrase(200), "OK") == 0);
    TEST_ASSERT(strcmp(http_reason_phrase(404), "Not Found") == 0);
    TEST_ASSERT(http_reason_phrase(418) == NULL);

    char response[512];
    size_t written = 0;
    TEST_ASSERT(http_build_response(200, "text/plain", "hello", response,
                                    sizeof response, &written));
    TEST_ASSERT(written == strlen(response));
    TEST_ASSERT(strstr(response, "HTTP/1.1 200 OK\r\n") == response);
    TEST_ASSERT(strstr(response, "Content-Length: 5\r\n") != NULL);
    TEST_ASSERT(strstr(response, "\r\n\r\nhello") != NULL);

    TEST_ASSERT(!http_build_response(418, "text/plain", "x", response,
                                     sizeof response, &written));
    TEST_ASSERT(!http_build_response(200, "text/plain", "long body", response,
                                     8, &written));
    TEST_ASSERT(!http_build_response(200, NULL, "x", response, sizeof response,
                                     &written));
    TEST_ASSERT(!http_build_response(200, "text/plain", "x", NULL, 0,
                                     &written));
}
