/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include "csapp.h"

// CGI 프로그램의 시작점
int main(void)
{
  char *buf, *p;
  char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
  int n1 = 0, n2 = 0;

  /*
   * QUERY_STRING 환경 변수에서 두 인자를 추출함.
   * CGI 프로그램은 웹 서버로부터 전달받은 쿼리 문자열을
   * QUERY_STRING 환경 변수로 통해 접근할 수 있음.
   */
  if ((buf = getenv("QUERY_STRING")) != NULL)
  {
    // 쿼리 문자열에서 '&' 문자를 찾아 두 인자를 분리함
    p = strchr(buf, '&');
    *p = '\0'; // '&'를 null 문자로 대체하여 문자열을 두 부분으로 나눔
    sscanf(buf, "arg1=%d", &n1);
    sscanf(p + 1, "arg2=%d", &n2);

    // // 인자들을 정수형으로 변환
    // n1 = atoi(arg1); // arg1을 정수형으로 변환하여 n1에 저장
    // n2 = atoi(arg2); // arg2를 정수형으로 변환하여 n2에 저장
  }

  /*
   * HTTP 응답의 콘텐츠를 생성함.
   * sprintf를 사용해 content에 응답 내용을 저장.
   */
  // sprintf(content, "QUERY_STRING=%s", buf);
  // sprintf(content, "Welcome to add.com: ");
  // sprintf(content, "%sTHE Internet addition portal. \r\n<p>", content);
  // sprintf(content, "%sThe answer is: %d + %d = %d\r\n<p>", content, n1, n2, n1 + n2);
  // sprintf(content, "%sThanks for visiting!\r\n", content);
  sprintf(content, "QUERY_STRING=%s", buf); // 초기화하면서 첫 번째 문자열 작성
  sprintf(content + strlen(content), "Welcome to add.com: ");
  sprintf(content + strlen(content), "%sTHE Internet addition portal. \r\n<p>", content);
  sprintf(content + strlen(content), "The answer is: %d + %d = %d\r\n<p>", n1, n2, n1 + n2);
  sprintf(content + strlen(content), "Thanks for visiting!\r\n");

  /*
   * HTTP 응답 헤더를 출력함.
   * 클라이언트에게 응답이 완료되었음을 알리는 HTTP 헤더를 전송.
   */
  printf("Connection: close\r\n");
  printf("Content-length: %d\r\n", (int)strlen(content)); // 콘텐츠 길이 출력
  printf("Content-type: text/html\r\n\r\n");              // 콘텐츠 타입 지정
  printf("%s", content);                                  // 콘텐츠 본문을 출력하여 클라이언트에게 전송
  fflush(stdout);                                         // 출력 버퍼를 비워서 클라이언트로 전송을 확실히 함

  exit(0); // 프로그램 종료
}
/* $end adder */
