#include "csapp.h"

// 클라이언트로부터 받은 데이터를 그대로 돌려주는 함수.
void echo(int connfd);

int main(int argc, char **argv)
{
    // 리슨 소켓과 클라이언트와 연결된 소켓의 파일 디스크립터 선언.
    int listenfd, connfd;
    // 클라이언트 주소의 크기를 저장할 변수.
    socklen_t clientlen;
    // 클라이언트의 주소 정보를 저장할 구조체.
    struct sockaddr_storage clientaddr;
    // 클라이언트의 호스트명과 포트 번호를 저장할 버퍼.
    char client_hostname[MAXLINE], client_port[MAXLINE];

    // 프로그램 실행 시 인자로 포트 번호가 정확히 하나 전달되었는지 확인.
    if (argc != 2)
    {
        // 포트 번호가 전달되지 않은 경우, 사용법을 출력하고 프로그램 종료.
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    // 사용자로부터 입력받은 포트 번호로 바인딩된 리슨 소켓을 열음.
    listenfd = Open_listenfd(argv[1]);

    // 무한 루프를 통해 클라이언트의 연결 요청을 지속적으로 처리.
    while (1)
    {
        // Accept 함수에 전달할 클라이언트 주소 구조체의 크기를 설정.
        clientlen = sizeof(struct sockaddr_storage);
        // 클라이언트의 연결을 수락하고 연결된 소켓의 파일 디스크립터를 얻음.
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

        // 연결된 소켓의 정보를 바탕으로 클라이언트의 호스트명과 포트 번호를 얻고 출력.
        Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);

        // echo 함수를 호출하여 클라이언트와의 통신을 처리.
        echo(connfd);

        // 통신이 끝나면 연결된 소켓을 닫음.
        Close(connfd);
    }

    // 프로그램을 정상 종료 (무한 루프이므로 실제로는 도달하지 않음).
    exit(0);
}

// echo 함수: 클라이언트로부터 데이터를 읽고 그대로 다시 돌려줌.
void echo(int connfd)
{
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    // 연결된 소켓에서 읽기 위해 RIO 버퍼 초기화.
    rio_readinitb(&rio, connfd);

    // 클라이언트로부터 데이터를 한 줄씩 읽고 처리하는 반복문.
    // 클라이언트로부터 한 줄의 데이터를 읽어서 buf에 저장.
    // Rio_readlineb()는 읽은 바이트 수를 반환하며, 0이면 연결이 종료된 것.
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0)
    {
        // 서버가 수신한 바이트 수를 출력.
        printf("server received %d bytes\n", (int)n);
        // 수신한 데이터를 클라이언트에게 다시 보냄 (에코).
        Rio_writen(connfd, buf, n);
    }
}
