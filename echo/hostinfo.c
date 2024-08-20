#include "csapp.h"  // 필요한 헤더 파일 포함 (네트워크 함수들과 상수들 정의)

// getaddrinfo와 getnameinfo를 이용해서 도메인 이름과 연관된 IP 주소와 매필을 출력하는 프로그램
int main(int argc, char **argv){
    struct addrinfo *p, *listp, hints;  // DNS 조회 결과와 옵션을 저장할 구조체들
    char buf[MAXLINE];  // IP 주소 문자열을 저장할 버퍼
    int rc, flags;  // 반환 코드와 플래그 변수

    // 명령줄 인자 확인 (도메인 이름이 제공되었는지)
    if (argc != 2){
        fprintf(stderr, "usage: %s <domain name>\n",argv[0]);
        exit(0);
    }

    // addrinfo 구조체 초기화 (DNS 조회 옵션 설정)
    memset(&hints, 0, sizeof(struct addrinfo));  // 구조체를 0으로 초기화
    hints.ai_family = AF_INET;       // IPv4 주소만 요청
    hints.ai_socktype = SOCK_STREAM; // TCP 연결용 소켓 타입 지정

    // getaddrinfo 함수로 DNS 조회 수행
    if ((rc = getaddrinfo(argv[1], NULL, &hints, &listp)) != 0){
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rc));
        exit(1);
    }

    // IP 주소를 숫자 형식의 문자열로 변환하기 위한 플래그 설정
    flags = NI_NUMERICHOST;

    // DNS 조회 결과 리스트 순회
    for (p = listp; p; p = p->ai_next){
        // 각 소켓 주소를 문자열로 변환(점-십진수 주소 스트링으로 변환하도록 해서 addrinfo를 호출해서 이것을 조심스럽게 반환)
        Getnameinfo(p->ai_addr, p->ai_addrlen, buf, MAXLINE, NULL, 0, flags);
        printf("%s\n", buf);  // 변환된 IP 주소 출력
    }

    // DNS 조회 결과로 할당된 메모리 해제
    Freeaddrinfo(listp);

    exit(0);  // 프로그램 정상 종료
}