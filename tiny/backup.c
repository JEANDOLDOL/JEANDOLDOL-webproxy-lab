/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 *
 * Updated 11/2019 droh
 *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
 */
#include "csapp.h"

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg);

// doit -> 한 개의 트랜젝션을 처리한다.
void doit(int fd)
{
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;

    /* read요청 코드와 헤더 */
    Rio_readinitb(&rio, fd);
    Rio_readlineb(&rio, buf, MAXLINE);
    printf("request headers:\n");
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);
    if (strcasecmp(method, "GET"))
    {
        clienterror(fd, method, "501", "Not implemented", "Tiny does not implement this method");
        return;
    }
    // 다른 요청 헤더파일들 무시.
    read_requesthdrs(&rio);

    /* GET 요청으로 부터 받은 URI를 분석 */
    is_static = parse_uri(uri, filename, cgiargs); // 정적 콘텐츠니?
    if (stat(filename, &sbuf) < 0)
    {
        clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file");
        return;
    }

    /* 정적 콘텐츠를 전송 */
    if (is_static)
    {
        // 읽기 권한을 가지고 있는지 검증함.
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
        {
            clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");
            return;
        }
        // 읽기 권한이 있다면 정적 콘텐츠 제공.
        serve_static(fd, filename, sbuf.st_size);
    }

    /* 동적 콘텐츠를 전송 */
    else
    {
        // 실행가능한 파일인지 검증
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode))
        {
            clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the CGI program");

            return;
        }
        // 그렇다면 동적 콘텐츠 제공.
        serve_dynamic(fd, filename, cgiargs);
    }

    return;
}

// Tiny clienterror 에러 메시지를 클라이언트에게 보낸다.
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
    char buf[MAXLINE], body[MAXBUF];

    /* HTTP 응답 body 만들기 */
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor="
                  "ffffff"
                  " >\r\n ",
            body);
    sprintf(body, " %s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

    /* HTTP 응답을 출력 */
    sprintf(buf, "HTTP/1.0 %s %s \r\n", errnum, shortmsg); // 예시:HTTP/1.0 404 Not Found
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}

// Tiny 요청 헤더를 읽고 무시한다. -> GET 요청만 처리하게 때문? -> 그렇지 않다.
void read_requesthdrs(rio_t *rp)
{
    char buf[MAXLINE];

    // 클라이언트의 요청으로부터 한 줄을 읽어 버퍼에 저장한다.
    // 이 함수는 파일 디스크립터를 통해 데이터를 읽어들여 버퍼에 저장하고, 읽은 바이트 수를 반환함.
    rio_readlineb(rp, buf, MAXLINE);
    // 현재 읽은 줄(buf)가 빈 줄이 아닐때 계속 반복
    while (strcmp(buf, "\r\n"))
    {
        rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
    return;
}

// HTTP URI를 분석한다.
// 요청된 uri가 정적 콘텐츠인지 동적콘텐츠인지 구분함.
int parse_uri(char *uri, char *filename, char *cgiargs)
{
    char *ptr;
    if (!strstr(uri, "cgi-bin")) // 정적 콘텐츠인 경우
    {
        strcpy(cgiargs, "");
        strcpy(filename, ".");           // 현재 디렉토리로 시작하는 파일 경로
        strcat(filename, uri);           // URI를 파일 경로로 추가
        if (uri[strlen(uri) - 1] == '/') // URI가 /로 끝나면
        {
            strcat(filename, "home.html"); // 기본 파일을 설정 (예: home.html)
        }
        return 1; // 정적 콘텐츠임을 나타냄
    }
    else // 동적 콘텐츠인 경우
    {
        ptr = index(uri, '?'); // ?로 쿼리 문자열을 분리
        if (ptr)
        {
            strcpy(cgiargs, ptr + 1); // ? 이후의 문자열을 cgiargs에 복사
            *ptr = '\0';              // URI의 ? 이전 부분만 남기기
        }
        else
        {
            strcpy(cgiargs, ""); // 쿼리 문자열이 없으면 빈 문자열 할당
        }
        strcpy(filename, "."); // 현재 디렉토리로 시작하는 파일 경로
        strcat(filename, uri); // URI를 파일 경로로 추가
        return 0;              // 동적 콘텐츠임을 나타냄
    }
}

// 정적 콘텐츠를 클라이언트에게 서비스한다
void serve_static(int fd, char *filename, int filesize)
{
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];
    char header[MAXLINE] = ""; // cat head

    // 응답 헤더를 클라이언트에게 전송
    get_filetype(filename, filetype);
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    strcat(header, buf); // happy cat
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    strcat(header, buf); // happy cat
    sprintf(buf, "%sConnection: close \r\n", buf);
    strcat(header, buf); // happy cat
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    strcat(header, buf); // happy cat
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    strcat(header, buf); // happy cat
    Rio_writen(fd, buf, strlen(buf));
    printf("Response headers:\n");
    printf("%s", buf);

    // 응답 바디를 클라이언트에게 전송
    srcfd = Open(filename, O_RDONLY, 0);
    // srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    srcp = (char *)malloc(filesize);
    rio_readn(srcfd, srcp, filesize);
    Rio_writen(fd, srcp, filesize);
    free(srcp);
    Close(srcfd);
}

// filename로부터 이끌어냄
void get_filetype(char *filename, char *filetype)
{
    if (strstr(filename, ".html"))
    {
        strcpy(filetype, "text/html");
    }
    else if (strstr(filename, ".gif"))
    {
        strcpy(filetype, "image/gif");
    }
    else if (strstr(filename, ".png"))
    {
        strcpy(filetype, "image/png");
    }
    else if (strstr(filename, ".jpg"))
    {
        strcpy(filetype, "image/jpg");
    }
    else if (strstr(filename, ".mp4"))
    {
        strcpy(filetype, "video/mp4");
    }
    else
    {
        strcpy(filetype, "text/plain");
    }
}

// 동적 콘텐츠를 클라이언트에 제공한다.
void serve_dynamic(int fd, char *filename, char *cgiargs)
{
    char buf[MAXLINE], *emptylist[] = {NULL};

    /* Return first part of HTTP response */
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));

    if (Fork() == 0) // 자식
    {
        // 실제 서버는 모든 CGI vars를 이곳에 설치할 것임
        setenv("QUERY_STRING", cgiargs, 1);
        Dup2(fd, STDOUT_FILENO);              // stdout을 클라이언트에게 향하도록 조정
        Execve(filename, emptylist, environ); // CGI 프로그램 실행
    }
    Wait(NULL); // 부모는 자식이 끝나길 기다린당
}

int main(int argc, char **argv)
{
    int listenfd, connfd; // 소켓 파일 디스크립터 두 개
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen; // 소켓 주소 구조체의 크기
    struct sockaddr_storage clientaddr;

    /* Check command line args */
    // 똑같이 파일이름과 포트 번호인가? -> 네
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    listenfd = Open_listenfd(argv[1]); // 주어진 포트번호로 소켓 열기
    // 무한 반복문으로 계속 들어
    while (1)
    {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr,
                        &clientlen); // line:netp:tiny:accept 연결 요청 수락
        Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE,
                    0); // 소켓 파일 디스크립터 반환
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        doit(connfd);  // 트랜젝션 실행
        Close(connfd); // line:netp:tiny:close
    }
}
