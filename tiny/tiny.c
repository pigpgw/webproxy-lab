/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 *
 * Updated 11/2019 droh
 *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
 */
#include "csapp.h"

// 함수 프로토타입 선언
void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

int main(int argc, char **argv) {
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    /* 명령줄 인자 확인 */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\\n", argv[0]);
        exit(1);
    }

    // 지정된 포트에서 리스닝 소켓 생성
    listenfd = Open_listenfd(argv[1]);

    while (1) {
        clientlen = sizeof(clientaddr);
        
        // 클라이언트의 연결 요청을 수락
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        
        // 클라이언트 정보 출력
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, 
                    port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        
        // 클라이언트 요청 처리
        doit(connfd);
        
        // 연결 종료
        Close(connfd);

        // 이 지점에서 다음 클라이언트 처리를 위해 루프의 시작으로 돌아감
    }
}

// 버퍼를 포함하고 있는 더 큰 개념
// typedef struct {
//     int rio_fd;                /* 이 버퍼와 연관된 파일 디스크립터 */
//     int rio_cnt;               /* 내부 버퍼에 남아있는 읽지 않은 바이트 수 */
//     char *rio_bufptr;          /* 내부 버퍼에서 다음에 읽을 바이트의 위치 */
//     char rio_buf[RIO_BUFSIZE]; /* 실제 데이터를 저장하는 내부 버퍼 */
// } rio_t;



// doit은 안 개의 HTTP 트랜잭션을 처리한다. 먼저 요청 라인을 읽고 분석한다.
// fd에 넘어오는 것은 일반적으로 네트워크 소켓의 식별자 accept를 통해 새로운 소켓이 생성되고 이 소켓의 파일 디스크립터를 반환함
// 이 새로 생성된 소켓의 파일 디스크립터가 doit() 함수에 전달됨
// 이 fd는 특정 클라이언트와의 연결을 나타냄, 서버는 이 fd를 사용하여 해당 클라이언트와 통신함
void doit(int fd)
{
    int is_static;  // 요청된 컨텐츠가 정적인지 동적인지를 나타내는 플래그
    struct stat sbuf;  // 파일 상태 정보를 저장하는 구조체
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];  // HTTP 요청을 파싱하기 위한 버퍼들
    char filename[MAXLINE], cgiargs[MAXLINE];  // 요청된 파일 이름과 CGI 인자를 저장할 버퍼
    rio_t rio;  // Robust I/O (RIO) 패키지를 위한 구조체
    printf("-------------%d---------------",fd);
    /* 요청 라인과 헤더 읽기 */
    Rio_readinitb(&rio, fd);  // rio 구조체 초기화. fd와 연결
    //한 개의 HTTP 트랜잭션을 처리함, 먼저 요청 라인을 읽음
    Rio_readlineb(&rio, buf, MAXLINE);  // 클라이언트로부터 한 줄(요청 라인) 읽기
    printf("Request headers:\n");
    printf("67876%s678678", buf);  // 요청 라인 출력
    // sscanf를 사용하여 buf에서 구조화된 데이터를 추출
    sscanf(buf, "%s %s %s", method, uri, version);  // 요청 라인을 method, uri, version으로 파싱

    // GET 메소드가 아닌 경우 에러 처리
    // strcasecmp 함수는 문자열을 비교하는 C 라이브러리 함수
    if (strcasecmp(method, "GET")) {
        clienterror(fd, method, "501", "Not implemented",
                    "Tiny does not implement this method");
        return;
    }
    read_requesthdrs(&rio);  // 나머지 요청 헤더들을 읽고 무시 (이 서버에서는 사용하지 않음)

    /* GET 요청에서 URI 파싱 */
    is_static = parse_uri(uri, filename, cgiargs);  // URI를 파일 이름과 CGI 인자로 파싱
    
    // 요청된 파일의 상태 확인
    if (stat(filename, &sbuf) < 0) {  // 파일이 존재하지 않으면
        clienterror(fd, filename, "404", "Not found",
                    "Tiny couldn't find this file");
        return;
    }

    if (is_static) { /* 정적 콘텐츠 제공 */
        // 일반 파일이 아니거나 읽기 권한이 없는 경우
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
            clienterror(fd, filename, "403", "Forbidden",
                        "Tiny couldn't read the file");
            return;
        }
        serve_static(fd, filename, sbuf.st_size);  // 정적 콘텐츠 제공
    }
    else { /* 동적 콘텐츠 제공 */
        // 일반 파일이 아니거나 실행 권한이 없는 경우
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
            clienterror(fd, filename, "403", "Forbidden",
                        "Tiny couldn't run the CGI program");
            return;
        }
        serve_dynamic(fd, filename, cgiargs);  // 동적 콘텐츠 제공
    }
}

void serve_static(int fd, char *filename, int filesize) {
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];

    /* 클라이언트에 응답 헤더 보내기 */
    // 파일 타입을 결정합니다.
    get_filetype(filename, filetype);
    // HTTP 응답 헤더를 생성합니다.
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%s서버: Tiny 웹 서버\r\n", buf);
    sprintf(buf, "%s연결: close\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    // 응답 헤더를 클라이언트에 보냅니다.
    Rio_writen(fd, buf, strlen(buf));
    printf("응답 헤더:\n");

    /* 클라이언트에 응답 본문 보내기 */
    // 요청된 파일을 엽니다.
    srcfd = Open(filename, O_RDONLY, 0);
    // 파일을 메모리에 매핑합니다.
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    // 파일 디스크립터를 닫습니다 (더 이상 필요 없음).
    Close(srcfd);
    // 파일 내용을 클라이언트에 전송합니다.
    Rio_writen(fd, srcp, filesize);
    // 매핑된 메모리 영역을 해제합니다.
    Munmap(srcp, filesize);
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) 
{
    char buf[MAXLINE], body[MAXBUF];

    /* Build the HTTP response body */
    sprintf(body, "<html><title>Tiny Error</title>");  // HTML 문서의 시작과 제목 설정
    sprintf(body, "%s<body bgcolor=\"ffffff\">\r\n", body);  // 바디 시작, 배경색 설정
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);  // 에러 번호와 짧은 메시지 추가
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);  // 긴 메시지와 원인 추가
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);  // 구분선과 서버 정보 추가

    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);  // HTTP 응답 상태 라인 생성
    Rio_writen(fd, buf, strlen(buf));  // 상태 라인을 클라이언트에게 전송
    
    sprintf(buf, "Content-type: text/html\r\n");  // Content-type 헤더 생성
    Rio_writen(fd, buf, strlen(buf));  // Content-type 헤더를 클라이언트에게 전송
    
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));  // Content-length 헤더 생성
    Rio_writen(fd, buf, strlen(buf));  // Content-length 헤더를 클라이언트에게 전송
    
    Rio_writen(fd, body, strlen(body));  // 응답 본문(HTML)을 클라이언트에게 전송
}

void read_requesthdrs(rio_t *rp) 
{
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);  // 첫 번째 헤더 라인을 읽음
    // 빈 라인(\r\n) 헤어듸 끝을 의미함
    while(strcmp(buf, "\r\n")) {  // 빈 라인(\r\n)을 만날 때까지 반복
        Rio_readlineb(rp, buf, MAXLINE);  // 다음 헤더 라인을 읽음
        printf("%s", buf);  // 읽은 헤더 라인을 출력 (디버깅 목적)
    }
    return;
}

int parse_uri(char *uri, char *filename, char *cgiargs) 
{
    char *ptr;

    // URI에 "cgi-bin"이 포함되어 있지 않으면 정적 컨텐츠로 간주
    printf("%s",uri);
    if (!strstr(uri, "cgi-bin")) { /* Static content */
        strcpy(cgiargs, "");                     // CGI 인자 문자열을 빈 문자열로 초기화
        strcpy(filename, ".");                   // filename을 현재 디렉토리(".")로 시작
        strcat(filename, uri);                   // filename에 URI 추가
        if (uri[strlen(uri)-1] == '/')           // URI가 '/'로 끝나면
            strcat(filename, "home.html");       // 기본 파일 "home.html" 추가
        return 1;                                // 정적 컨텐츠임을 나타내는 1 반환
    }
    else {  /* Dynamic content */
        ptr = index(uri, '?');                   // URI에서 '?' 문자 찾기
        if (ptr) {                               // '?'가 있으면 (CGI 인자가 있음)
            strcpy(cgiargs, ptr+1);              // '?' 다음 문자부터 CGI 인자로 복사
            *ptr = '\0';                         // URI에서 '?'를 널 문자로 대체하여 잘라냄
        }
        else
            strcpy(cgiargs, "");                 // CGI 인자가 없으면 빈 문자열로 설정
        strcpy(filename, ".");                   // filename을 현재 디렉토리(".")로 시작
        strcat(filename, uri);                   // filename에 URI 추가
        return 0;                                // 동적 컨텐츠임을 나타내는 0 반환
    }
}

void serve_dynamic(int fd, char *filename, char *cgiargs) {
    char buf[MAXLINE], *emptylist[] = { NULL };

    /* HTTP 응답의 첫 부분을 반환합니다 */
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "서버: Tiny 웹 서버\r\n");
    Rio_writen(fd, buf, strlen(buf));

    if (Fork() == 0) { /* 자식 프로세스 */
        /* 실제 서버라면 여기서 모든 CGI 변수를 설정할 것입니다 */
        // QUERY_STRING 환경 변수를 설정합니다.
        setenv("QUERY_STRING", cgiargs, 1);
        // 자식의 표준 출력을 클라이언트 소켓으로 재지정합니다.
        Dup2(fd, STDOUT_FILENO);
        // CGI 프로그램을 실행합니다.
        Execve(filename, emptylist, environ);
    }
    // 부모 프로세스는 자식이 종료될 때까지 기다리고 정리합니다.
    Wait(NULL);
}

/* get_filetype - Derive file type from filename */
void get_filetype(char *filename, char *filetype) {
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
        strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else if (strstr(filename, ".mpg") || strstr(filename, ".mpeg"))
        strcpy(filetype, "video/mpeg");
    else if (strstr(filename, ".mov"))
        strcpy(filetype, "video/mov");
    else if (strstr(filename, ".mp4"))
        strcpy(filetype, "video/mp4");
    else
        strcpy(filetype, "text/plain");
}