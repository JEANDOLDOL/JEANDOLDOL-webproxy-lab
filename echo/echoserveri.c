#include "csapp.h"

void echo(int connfd); // 클라이언트와 통신을 처리하는 함수

int main(int argc, char **argv)
{
    int listenfd, connfd;               // 서버가 클라이언트의 연결을 대기할 소켓, 클라이언트와의 개별적인 연결을 나타내는 소켓.
    socklen_t clientlen;                // 클라이언트의 주소길이를 저장하는 변수.
    struct sockaddr_storage clientaddr; // accept로 보내지는 소켓 주소 구조체.
    char client_hostname[MAXLINE], client_port[MAXLINE];

    if (argc != 2) // 클라이언트가 포트 번호를 제대로 받았는지 확인.
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    listenfd = Open_listenfd(argv[1]); // argv[1] = host, 서버 소켓을 연다
    while (1)                          // 클라이언트의 연결을 계속해서 기다림.
    {
        clientlen = sizeof(struct sockaddr_storage);                                                  // 클라이언트 주소 구조체의 크기를 저장. accept함수 호출시 필요하다.
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);                                     // accept()는 클라이언트의 요청을 수락함. 성공하면 새로운 연결 소켓 식별자를 반환하고, 이를 connfd에 저장.
        Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0); // 클라이언트의 IP주소를 인간이 읽을 수 있는 형식으로 변환.
        printf("Connected to (%s, %s)\n", client_hostname, client_port);                              // 연결된 클라이언트의 호스트 이름과 포트번호를 출력함.
        echo(connfd);                                                                                 // connfd소켓을 통해 클라이언트로부터 데이터를 읽고,. 그 데이터를 그대로 클라이언트에게 돌려준다.
        Close(connfd);                                                                                // 통신 종료
    }
    exit(0); // 프로그램 종료코드. but, 코드는 무한루프안에 있기 떄문에 이부분에 도달하지 않음.
}