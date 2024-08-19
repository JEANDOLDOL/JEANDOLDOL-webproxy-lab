#include "csapp.h"
// argc = argument count, argv = argument vector
int main(int argc, char **argv)
{
    int clientfd; // 열린 소켓 식별자를 저장할 변수
    char *host, *port, buf[MAXLINE];
    // rio_t는 버퍼링된 입출력을 위해 사용되는 구조체. rio는 서버와의 통신에서 데이터를 읽고 쓰기 위한 상태를 저장한다.
    rio_t rio;

    if (argc != 3) // argument : './program','host.com','8080' 이 세개가 전부 있지 않다면.
    {
        // stderr: 표준 오류 출력 스트림.
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
    port = argv[2];

    // 클라이언트 소켓 생성.
    clientfd = Open_clientfd(host, port);
    // rio_t 구조체 초기화, clientfd 소켓을 통해 버퍼링된 입출력을 할 수 있도록 설정.
    Rio_readinitb(&rio, clientfd);

    // Fgets 함수는 표준 입력(stdin)에서 한 줄을 읽어 buf에 저장한다.
    // Fgets가 NULL을 반환할 때(즉, 입력이 종료될 때) 루프를 빠져나간다.
    while (Fgets(buf, MAXLINE, stdin) != NULL)
    {
        // Rio_writen <- 이건 버퍼가 있는데 왜 버퍼가 없는 입출력 rio로 나오지?
        Rio_writen(clientfd, buf, strlen(buf)); // Rio_writen 함수는 buf에 저장된 데이터를 clientfd 소켓을 통해 서버로 전송한다. strlen(buf)는 전송할 데이터의 길이를 계산한다.
        Rio_readlineb(&rio, buf, MAXLINE);      // Rio_readlineb 함수는 서버로부터 응답을 한 줄(\n으로 끝나는)을 읽어 buf에 저장한다.
        Fputs(buf, stdout);                     // Fputs 함수는 서버로부터 받은 응답을 표준 출력(stdout)에 출력한다.
    }
    Close(clientfd);
    exit(0);
}