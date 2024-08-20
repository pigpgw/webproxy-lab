#include "csapp.h" // 네트워크 함수들과 상수들이 정의된 헤더 파일 포함

int main(int argc, char **argv)
{
    int clientfd; // 클라이언트 소켓 파일 디스크립터
    char *host, *port, buf[MAXLINE]; // 호스트명, 포트, 데이터 버퍼
    rio_t rio; // Robust I/O 구조체 (RIO)

    // 사용법을 확인하고 인자가 정확히 3개가 아니면 에러 메시지 출력
    if (argc != 3)
    {
        fprintf(stderr, "usage: %s <host> <port> \n", argv[0]); // 사용법 안내
        exit(0); // 프로그램 종료
    }

    // 명령행 인수로부터 호스트명과 포트 번호를 저장
    host = argv[1];
    port = argv[2];

    // 서버와 연결하기 위한 클라이언트 소켓을 생성하고 연결 시도
    clientfd = Open_clientfd(host, port);

    // Robust I/O (RIO) 구조체 초기화, 클라이언트 소켓으로 데이터 입출력 준비
    Rio_readinitb(&rio, clientfd);

    // 표준 입력(stdin)으로부터 입력받은 데이터를 서버로 전송하고, 서버로부터 응답을 받는 루프
    while (Fgets(buf, MAXLINE, stdin) != NULL) // 표준 입력으로부터 데이터를 읽음
    {
        // 클라이언트 소켓을 통해 서버에 데이터를 전송
        Rio_writen(clientfd, buf, strlen(buf));

        // 서버로부터 데이터를 한 줄씩 읽음
        Rio_readlineb(&rio, buf, MAXLINE);

        // 서버로부터 받은 데이터를 표준 출력(stdout)에 출력
        Fputs(buf, stdout);
    }

    // 모든 작업이 끝난 후, 클라이언트 소켓을 닫음
    Close(clientfd);

    // 프로그램 종료
    exit(0);
}
