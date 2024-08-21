/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include "csapp.h"

int main(void) {
    char *buf, *p;
    char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
    int n1=0, n2=0;

    // 환경 변수에서 쿼리 스트링을 읽어옵니다.
    // 쿼리 스트링이 있으면 이를 처리합니다.
    // 사용자가 폼을 제출하면 브라우저는 "/cgi-bin/adder?num1=값1&num2=값2" 형식의 URL을 생성
    if ((buf = getenv("QUERY_STRING")) != NULL) {
        // 쿼리 스트링에서 '&'를 찾아 두 인자를 분리합니다.
        p = strchr(buf, '&');
        *p = '\0';  // '&'를 널 문자로 바꿔 문자열을 분리합니다.
        sscanf(buf, "num1=%d", &n1);
        sscanf(p + 1, "num2=%d", &n2);
    }

    // 응답 본문을 만듭니다.
    // sprintf를 여러 번 사용하여 content 문자열을 점진적으로 구성합니다.
    // 이 부분은 if 문 밖에 있어야 합니다. 쿼리 스트링이 없어도 응답은 생성해야 하기 때문입니다.
    sprintf(content, "QUERY_STRING=%s<br>", buf);
    sprintf(content, "%sWelcome to add.com: ", content);
    sprintf(content, "%sTHE Internet addition portal.\r\n<p>", content);
    sprintf(content, "%sThe answer is: %d + %d = %d\r\n<p>", content, n1, n2, n1 + n2);
    sprintf(content, "%sThanks for visiting!\r\n", content);

    // HTTP 응답을 생성합니다.
    // 이 printf 문들은 HTTP 헤더와 본문을 직접 출력합니다.
    // 이 부분은 항상 실행되어야 하므로 if 문 밖에 있습니다.
    printf("Connection: close\r\n");
    printf("Content-length: %d\r\n", (int)strlen(content));
    printf("Content-type: text/html\r\n\r\n");  // 헤더의 끝을 나타내는 빈 줄
    printf("%s", content);  // 응답 본문 출력
    fflush(stdout);  // 출력 버퍼를 즉시 비웁니다.

    exit(0);
}