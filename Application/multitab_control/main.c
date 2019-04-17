#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/kdev_t.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <time.h>
#include <dirent.h>
#include <pthread.h>

#include "queue.h"                           // 큐 자료구조

#define MDS2450_MULTITAB_CONTROL_MAJOR 71    // 디바이스 드라이버 메이저 넘버

#define TCP_PORT 5555                        // 소켓통신 포트
#define MAXLINE 1000                         // 소켓으로 한번에 받을 수 있는 데이터 길이
#define NAME_LEN 20                          // 소켓 클라이언트 이름 길이

char *EXIT_STRING = "exit";                  // 소켓 통신 종료 문자

int s;                                       // 소켓 파일 디스크립터

// 소켓 생성 및 서버 연결, 생성된 소켓리턴 함수
int tcp_connect(int af, char *servip, unsigned short port);
void errquit(char *mesg);

void *thread_function(void *arg);            // 쓰레드 함수

void Print_Queue(Queue *q);                  // 큐 내용 출력 함수

int main(int argc, char **argv)
{
   Queue q;                                  // 음악을 저장 할 큐

   DIR *dir;                                 // 디렉터리를 조회 할 디렉터리 구조체
   struct dirent *ent;                       //

   int dev_fd;                               // 디바이스 드라이버 파일 디스크립터
   char dev_path[32];                        // 디바이스 드라이보 파일 패스

   pthread_t s_thread;                       // aplay 명령을 실행 할 스레드

   char bufmsg[MAXLINE];                     // 수신 버퍼
   char sendbuf[MAXLINE];                    // 송신 버퍼
   int maxfdp1;                              // 최대 소켓 디스크립터

   fd_set read_fds;                          // 파일 디스크립터 집합
   char command[2];                          // LED 커맨드 저장 배열

   QueueInit(&q);                            // 큐 초기화

   if (argc != 3)                         
   {
      printf("how to execute => %s sever_ip name \n", argv[0]);
      exit(0);
   }

   // 소켓 초기화
   s = tcp_connect(AF_INET, argv[1], TCP_PORT);
   if (s == -1)
      errquit("tcp_connect fail");

   puts("Connected to Server");
   maxfdp1 = s + 1;
   FD_ZERO(&read_fds);                       // read_fds를 초기화

   ////////////////

   // 디바이스 드라이버 초기화
   sprintf(dev_path, "/dev/multitab_control");
   mknod(dev_path, (S_IRWXU | S_IRWXG | S_IFCHR),
         MKDEV(MDS2450_MULTITAB_CONTROL_MAJOR, 0));

   dev_fd = open(dev_path, O_RDWR);          // 디바이스 드라이버 파일 디스크립터 열기
   if (0 > dev_fd)
      printf("Open fail!!\n");

   while (1)
   {
      FD_SET(0, &read_fds);                  // read_fds에 0을 추가 
      FD_SET(s, &read_fds);                  // read_fds에 소켓 파일 디스크립터 값을 추가
      
      // 사용 가능한 파일 디스크립터 조회
      if (select(maxfdp1, &read_fds, NULL, NULL, NULL) < 0)
         errquit("select fail");
      
      if (FD_ISSET(s, &read_fds))            // 소켓이 열려 있다면
      {
         int nbyte;
         char command_index = -1;
         if ((nbyte = recv(s, bufmsg, MAXLINE, 0)) > 0)
         {
            bufmsg[nbyte] = 0;
            write(1, "\033[0G", 4);          //커서의 X좌표를 0으로 이동
            printf("%s\r\n", bufmsg);        //메시지 출력
            fprintf(stderr, "\033[1;32m");   //글자색을 녹색으로 변경
            fprintf(stderr, "%s>", argv[2]); //내 닉네임 출력

            if (strchr(bufmsg, 'C') != NULL)
            {
               command_index = strchr(bufmsg, 'C') - bufmsg;
               if (!command_index && strlen(bufmsg) == 4)
               {
                  command[0] = bufmsg[1];
                  command[1] = bufmsg[2];
                  write(dev_fd, command, 2);
               }
            }
            else if (!strcmp(bufmsg, "START\n"))
            {
               printf("APP STARTED!!\n");

               while (!IsEmpty(&q))
                  Dequeue(&q);

               printf("QUEUE CLEARED!!\n");

               dir = opendir("./Playlist");
               if (dir != NULL)
               {
                  while ((ent = readdir(dir)) != NULL)
                  {
                     if (strstr(ent->d_name, ".wav") - ent->d_name == strlen(ent->d_name) - 4)
                     {
                        Enqueue(&q, ent->d_name);
                        printf("Equeue : %s\n", ent->d_name);
                     }
                  }
                  closedir(dir);
               }
               else
               {
                  printf("%s\n", "Can't Open Directory!!");
               }
            }
            else if (!strcmp(bufmsg, "LIST\n"))
            {
               Print_Queue(&q);
            }
            else if (strstr(bufmsg, "PLAY") - bufmsg == 0 && strlen(bufmsg) == 6)
            {
               int index = 0;
               int music_index = bufmsg[4] - 0x30;
               char temp[BUFSIZ];

               for (; index < q.size; index++)
               {
                  if (index == music_index)
                  {
                     printf("PLAY : %s\n", Peek(&q));
                     sprintf(temp, "aplay Playlist/%s", Peek(&q));
                     printf("%s\n", temp);

                     pthread_create(&s_thread, NULL, thread_function, (void *)temp);
                  }
                  Enqueue(&q, Dequeue(&q));
               }
            }
         }
      }
      if (FD_ISSET(0, &read_fds))
      {
         if (fgets(bufmsg, MAXLINE, stdin))
         {
            fprintf(stderr, "\033[1;33m"); //글자색을 노란색으로 변경
            fprintf(stderr, "\033[1A");    //Y좌표를 현재 위치로부터 -1만큼 이동
            sprintf(sendbuf, "%s", bufmsg); //메시지에 현재시간 추가
            if (send(s, sendbuf, strlen(sendbuf), 0) < 0)
               puts("Error : Write error on socket.");
            if (strstr(bufmsg, EXIT_STRING) != NULL)
            {
               puts("Good bye.");
               close(s);
               exit(0);
            }
         }
      }
   }

   close(dev_fd);

   return 0;
}

int tcp_connect(int af, char *servip, unsigned short port)
{
   struct sockaddr_in servaddr;
   int s;
   // 소켓 생성
   if ((s = socket(af, SOCK_STREAM, 0)) < 0)
      return -1;
   // 채팅 서버의 소켓주소 구조체 servaddr 초기화
   bzero((char *)&servaddr, sizeof(servaddr));
   servaddr.sin_family = af;
   inet_pton(AF_INET, servip, &servaddr.sin_addr);
   servaddr.sin_port = htons(port);

   // 연결요청
   if (connect(s, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
      return -1;
   return s;
}

void errquit(char *mesg)
{
   perror(mesg);
   exit(1);
}

void *thread_function(void *arg)
{
   system((char *)arg);
}

void Print_Queue(Queue *q)
{
   int index = 0;
   char *list_buffer;

   printf("Music List : \n");
   for (; index < q->size; index++)
   {
      list_buffer = (char *)malloc(strlen(Peek(q)) + 1);
      sprintf(list_buffer, "%s\n", Peek(q));
      if (send(s, list_buffer, strlen(list_buffer), 0) < 0)
         puts("Error : Write error on socket.");

      printf("\t\t%s\n", Peek(q));
      Enqueue(q, Dequeue(q));
      free(list_buffer);
   }
}