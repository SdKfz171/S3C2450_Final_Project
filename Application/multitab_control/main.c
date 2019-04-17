// 라이브러리
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
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
#include <signal.h>


#include "queue.h"                                          // 큐 자료구조

// 매크로
#define MDS2450_MULTITAB_CONTROL_MAJOR 71                   // 디바이스 드라이버 메이저 넘버

#define TCP_PORT 5555                                       // 소켓통신 포트
#define MAXLINE 1000                                        // 소켓으로 한번에 받을 수 있는 데이터 길이
#define NAME_LEN 20                                         // 소켓 클라이언트 이름 길이

// 전역변수
char *EXIT_STRING = "exit";                                 // 소켓 통신 종료 문자

int s;                                                      // 소켓 파일 디스크립터

// 함수 프로토타입
int tcp_connect(int af, char *servip, unsigned short port); // 소켓 생성 및 서버 연결, 생성된 소켓리턴 함수
void errquit(char *mesg);

void *thread_function(void *arg);                           // 쓰레드 함수

void Print_Queue(Queue *q);                                 // 큐 내용 출력 함수

int getCmdLine(char *file, char *buf); 



// 메인 함수
int main(int argc, char **argv)
{
   Queue q;                                                 // 음악을 저장 할 큐

   DIR *dir;                                                // 디렉터리를 조회 할 디렉터리 구조체
   struct dirent *ent;                                      // 파일의 inode로 파일을 선택할 dirent 구조체

   int dev_fd;                                              // 핀 제어 디바이스 드라이버 파일 디스크립터
   char dev_path[32];                                       // 디바이스 드라이보 파일 패스

   pthread_t s_thread;                                      // aplay 명령을 실행 할 스레드

   char bufmsg[MAXLINE];                                    // 수신 버퍼
   char sendbuf[MAXLINE];                                   // 송신 버퍼
   int maxfdp1;                                             // 최대 소켓 디스크립터

   fd_set read_fds;                                         // 파일 디스크립터 집합
   char command[2];                                         // LED 커맨드 저장 배열

   bool music_state = false;                                // 음악 재생 상태 플래그
   int aplaypid;                                            // aplay 프로세스 아이디

   QueueInit(&q);                                           // 큐 초기화

   if (argc != 3)                         
   {
      printf("how to execute => %s sever_ip name \n", argv[0]);
      exit(0);
   }

   s = tcp_connect(AF_INET, argv[1], TCP_PORT);             // 소켓 생성
   if (s == -1)
      errquit("tcp_connect fail");

   puts("Connected to Server");
   maxfdp1 = s + 1;                                         // 소켓 디스크립터 카운트 추가
   FD_ZERO(&read_fds);                                      // read_fds를 초기화

   sprintf(dev_path, "/dev/multitab_control");              
   mknod(dev_path, (S_IRWXU | S_IRWXG | S_IFCHR),
         MKDEV(MDS2450_MULTITAB_CONTROL_MAJOR, 0));         // 디바이스 드라이버 초기화

   dev_fd = open(dev_path, O_RDWR);                         // 디바이스 드라이버 파일 디스크립터 열기
   if (0 > dev_fd)
      printf("Open fail!!\n");

   while (1)
   {
      FD_SET(0, &read_fds);                                 // read_fds에 0(stdin)을 추가 
      FD_SET(s, &read_fds);                                 // read_fds에 소켓 디스크립터 값을 추가
      
      if (select(maxfdp1, &read_fds, NULL, NULL, NULL) < 0) // 사용 가능한 파일 디스크립터 조회
         errquit("select fail");
      
      if (FD_ISSET(s, &read_fds))                           // 소켓이 열려 있다면
      {
         int nbyte;                                         // 받은 데이터 크기
         char command_index = -1;            

         if ((nbyte = recv(s, bufmsg, MAXLINE, 0)) > 0)     // 받은 데이터가 있다면
         {
            bufmsg[nbyte] = 0;                              // 마지막 문자를 NULL로 변경
            write(1, "\033[0G", 4);                         // 커서의 X좌표를 0으로 이동
            printf("%s\r\n", bufmsg);                       // 메시지 출력
            fprintf(stderr, "\033[1;32m");                  // 글자색을 녹색으로 변경
            fprintf(stderr, "%s>", argv[2]);                // 내 닉네임 출력

            if (strchr(bufmsg, 'C') != NULL)                // 수신 버퍼에 C가 있다면
            {
               command_index = strchr(bufmsg, 'C') - bufmsg;// C의 위치를 찾아서 저장
               if (!command_index && strlen(bufmsg) == 4)   // C가 수신 버퍼의 처음이면서 수신 버퍼의 크기가 4이면 ex) C01\n
               {
                  command[0] = bufmsg[1];                   // 커맨드만 분리해서 저장
                  command[1] = bufmsg[2];                   
                  write(dev_fd, command, 2);                // 핀 제어 디바이스 드라이버로 커맨드 전송
               }
            }
            else if (!strcmp(bufmsg, "START\n"))            // 수신 버퍼의 데이터가 START\n이면
            {
               printf("APP STARTED!!\n");                   

               while (!IsEmpty(&q))                         // 기존의 큐에 데이터가 있으면 
                  Dequeue(&q);                              // 기존의 데이터 전부 제거

               printf("QUEUE CLEARED!!\n");

               dir = opendir("./Playlist");                 // Playlist 디렉토리 불러오기
               if (dir != NULL)                             // 디렉토리가 있으면
               {
                  while ((ent = readdir(dir)) != NULL)      // 디렉토리의 파일이 없을 때 까지 읽기
                  {
                     // .wav로 끝나는 파일이면
                     if (strstr(ent->d_name, ".wav") - ent->d_name == strlen(ent->d_name) - 4)
                     {
                        Enqueue(&q, ent->d_name);           // 큐에 파일명 추가
                        printf("Equeue : %s\n", ent->d_name);
                     }
                  }
                  closedir(dir);                            // 디렉토리 닫기
               }
               else
               {
                  printf("%s\n", "Can't Open Directory!!");
               }
            }
            else if (!strcmp(bufmsg, "LIST\n"))             // 수신 버퍼의 데이터가 LIST\n이면
            {
               Print_Queue(&q);                             // 큐 데이터 출력 함수 호출
            }
            else if (strstr(bufmsg, "PLAY") - bufmsg == 0 && 
                     strlen(bufmsg) == 6)                   // 수신 버퍼의 데이터가 PLAY로 시작하면서 길이가 6이면 ex) PLAY0\n
            {
               int i = 0;
               int music_index = bufmsg[4] - 0x30;          // 선택한 음악의 큐에서의 인덱스 
               char music_command[BUFSIZ];                  // aplay를 실행하는 명령어 저장 배열

               for (; i < q.size; i++)                      // 큐의 사이즈만큼 반복
               {
                  if (i == music_index)                     // 선택한 음악의 인덱스이면
                  {
                     printf("PLAY : %s\n", Peek(&q));
                     sprintf(music_command, "aplay Playlist/%s", Peek(&q));
                     printf("%s\n", music_command);

                     // 쓰레드에서 aplay 실행 ==> 이렇게 하지 않으면 aplay 프로그램이 포그라운드로 실행되기 때문에 현 프로그램의 사용이 불가능 해진다. 
                     pthread_create(&s_thread, NULL, thread_function, (void *)music_command);
                  }
                  Enqueue(&q, Dequeue(&q));                 // <== 이 부분 때문에 LinkedList로 바꿀까 생각 중
               }
            }
            else if(!strcmp(bufmsg, "PAUSE\n"))             // 수신 버퍼의 데이터가 PAUSE\n이면
            {
               struct stat fileStat;                        // 파일의 정보를 담는 구조체

               int pid;                                     // 프로세스 아이디
               char cmdLine[256];
               char tempPath[256];
               printf("Processes Info\n");

               dir = opendir("/proc");                      // /proc 디렉토리 불러오기
               if (!music_state)                            // 음악이 정상 재생중이면
               {
                  while ((ent = readdir(dir)) != NULL)      // 디렉토리의 파일이 없을 때 까지 읽기
                  {                                         
                     lstat(ent->d_name, &fileStat);         // 파일의 상태 정보를 읽기

                     if (!S_ISDIR(fileStat.st_mode))        // 파일이 디렉토리가 아니라면
                        continue;                           // 스킵

                     pid = atoi(ent->d_name);               // 디렉토리명을 숫자로 반환
                     if (pid <= 0)
                        continue;                           // 숫자가 아니라면 다시 스킵

                     sprintf(tempPath, "/proc/%d/cmdline", pid); // cmdline : 프로세스 이름이 적힌파일
                     getCmdLine(tempPath, cmdLine);              // /proc/pid/cmdline에서 프로세스명을 가져오는 함수 호출 
                                                                 
                     if (strcmp(cmdLine, "aplay") == 0)     // 프로세스명에 aplay가 들어 있으면
                     {
                        kill(pid, SIGSTOP);                 // 프로세스 정지
                        music_state = true;                 // 음악이 일시정지됨
                        break;
                     }
                  }
                  aplaypid = pid;                           // aplaypid에 찾은 프로세스 아이디 적용
               }
               else
               {
                  kill(aplaypid, SIGCONT);                  // 프로세스 재개
                  music_state = false;                      // 음악이 다시 실행됨
               }
               closedir(dir);                               // 디렉토리 닫기
            }
         }
      }
      if (FD_ISSET(0, &read_fds))                           // stdin 파일 디스크립터가 열려있다면
      {
         if (fgets(sendbuf, MAXLINE, stdin))                 // 문자열을 입력 받았다면
         {
            fprintf(stderr, "\033[1;33m");                  // 글자색을 노란색으로 변경
            fprintf(stderr, "\033[1A");                     // Y좌표를 현재 위치로부터 -1만큼 이동
            
            if (send(s, sendbuf, strlen(sendbuf), 0) < 0)   // 소켓으로 전송
               puts("Error : Write error on socket.");
            if (strstr(bufmsg, EXIT_STRING) != NULL)        // 끝내는 문자열을 입력받으면 종료
            {
               puts("Good bye.");
               close(s);                                    // 소켓 디스크립터 닫기
               exit(0);
            }
         }
      }
   }

   close(dev_fd);                                           // 핀 제어 디바이스 드라이버 파일 디스크립터 닫기

   return 0;
}

// 함수
int tcp_connect(int af, char *servip, unsigned short port)
{
   struct sockaddr_in servaddr;                             // 소켓 주소 구조체
   int s;                                                   // 함수 내부에서 사용 할 소켓
   
   if ((s = socket(af, SOCK_STREAM, 0)) < 0)                // 소켓 생성
      return -1;
   
   bzero((char *)&servaddr, sizeof(servaddr));              // 채팅 서버의 소켓 주소 구조체 servaddr 초기화
   servaddr.sin_family = af;                                // af값을 소켓 주소의 어드레스 패밀리값에 반영
   inet_pton(AF_INET, servip, &servaddr.sin_addr);          // IPv4형식 주소를 바이너리 형태로 변경
   servaddr.sin_port = htons(port);                         // port값을 네트워크 byte order로 변경 후 소켓 주소의 포트값에 반영

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
   system((char *)arg);                                     // 인자로 넘겨받은 문자열을 쉘에서 실행
}

void Print_Queue(Queue *q)
{
   int i = 0;
   char *list_buffer;                                       // 파일명을 임시로 저장할 문자열

   printf("Music List : \n");
   for (; i < q->size; i++)                                 // 큐의 크기 만큼 반복
   {
      list_buffer = (char *)malloc(strlen(Peek(q)) + 1);    // 큐에서 PEEK한 값의 크기 + 1 만큼 동적 할당 
      sprintf(list_buffer, "%s\n", Peek(q));                // PEEK한 값에 개행 문자를 하나 붙여서  
      if (send(s, list_buffer, strlen(list_buffer), 0) < 0) // 소켓으로 전송
         puts("Error : Write error on socket.");

      printf("\t\t%s\n", Peek(q));
      Enqueue(q, Dequeue(q));
      free(list_buffer);                                    // 동적 할당 한 버퍼 메모리 해제
   }
}


int getCmdLine(char *file, char *buf) { // 이거 
    FILE *srcFp;                         // file descripter
    int i;
    srcFp = fopen(file, "r");            //  전달받은 경로 file open (/proc/pid/cmdline)

    memset(buf, 0, sizeof(buf));         //  배열buf 0으로 memset
    fgets(buf, 256, srcFp);              //  프로세스명을 buf에 씌움.
    fclose(srcFp);                       //  파일 디스크립터 닫기
}
