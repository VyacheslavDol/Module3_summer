//персональный чат UDP, процессы общаются напрям

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/wait.h>
#include "input_char.h"

#define IP1 "127.0.0.1"
#define IP2 "127.0.0.5"
#define EXIT "exit"
#define WORD 
int exit_flag;

void exit_handler()
{
    exit_flag = 0;
}

int main(int argc, char* argv[])
{   
    if(argc != 2)
    {
        printf("./programm x, где x = 1 или 2 для разных собеседников\n");
        exit(EXIT_FAILURE);
    }
    int a = atoi(argv[1]);
    if(a == 0)
    {
        printf("Incorrect input\n");
        exit(EXIT_FAILURE);
    }
    int rv;
    int socketfd; /* дескрипторы сокетов чтения(принятия) и записи(отправки) */
    char sendline[1000], recvline[1000]; /* Массивы для отсылаемой и принятой строки */
    char exit_word[] = "Собеседник вышел из чата\0";
    struct sockaddr_in dest_addr, my_addr; /* Структуры для адресов назначения и своего */
    char my_ip[16], dest_ip[16];
    pid_t pid; /* PID процесса для функции fork() */

    int my_port = (a == 1) ? 50000 : 50002;    /* порт личный */
    int dest_port = (a == 1) ? 50002 : 50000;  /* порт собеседника */

    if(a == 1)          /* IP */
    {
        strcpy(my_ip, IP1);
        strcpy(dest_ip, IP2);
    }else
    {
        strcpy(my_ip, IP2);
        strcpy(dest_ip, IP1);
    }
    /* Реакция на сигнал*/
    signal(SIGUSR1, exit_handler);

    /* Создаем UDP сокет слушающий */
    if((socketfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror(NULL); /* Печатаем сообщение об ошибке */
        exit(EXIT_FAILURE);
    }

    /* Заполнение структуры своего адреса */
    bzero(&my_addr, sizeof(my_addr));   //заполнение 0
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(my_port);                                    //номер порта статическое задание
    if(inet_pton(AF_INET, my_ip, &my_addr.sin_addr) == 0)
    {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    };

    /* Заполнение структуры адреса назначения */
    bzero(&dest_addr, sizeof(dest_addr));                   //заполнение 0
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(dest_port);                    //номер порта статическое задание
    if(inet_pton(AF_INET, dest_ip, &dest_addr.sin_addr) == 0)      //IP адрес
    {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    };

    /* привязывание слушающего сокета к порту и IP адресу */
    if(bind(socketfd, (struct sockaddr*) &my_addr, sizeof(my_addr)) < 0)
    {
        perror(NULL);
        close(socketfd); /* По окончании работы закрываем дескриптор сокета */
        exit(EXIT_FAILURE);
    }

    /* Создание дочернего процесса для отправки сообщений */
    switch (pid = fork())
    {
        /* Дочерний процесс, отпраква сообщений */
        case 0:
            exit_flag = 1;
            while(exit_flag)
            {
                /* создание сообщения */
                input_char(sendline, sizeof(sendline));
                /* отправка сообщения */
                if(strcmp(EXIT, sendline) == 0)
                {   
                    strcpy(sendline, exit_word);
                    sendto(socketfd, sendline, strlen(sendline), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
                    kill(getppid(), SIGUSR1);
                    break;
                }
                if(sendto(socketfd, sendline, strlen(sendline), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0)
                {
                    perror(NULL);
                    close(socketfd);
                    exit(EXIT_FAILURE);
                };
            }
            close(socketfd);
            exit(EXIT_SUCCESS);
        break;
        /* Родительский процесс, принятие сообщений */
        default:

            /* TIMEOUT для сокета чтения */
            struct timeval tv;
            tv.tv_sec = 1;  // Таймаут 1 секунд
            tv.tv_usec = 0; // 0 микросекунд
            setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

            exit_flag = 1;
            while(exit_flag)
            {
                /* Получение сообщений */
                if(recvfrom(socketfd, recvline, sizeof(recvline), 0, (struct sockaddr *) NULL, NULL) < 0)
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                  // Таймаут, данных нет. Можно просто продолжить ожидание
           /* printf("Данных нет\n"); */
           continue;
        } else if(errno == EINTR)
        {
            break;
        } else {
           perror("recvfrom");
           break;
       }    
            recvline[strlen(recvline)] = '\0';
            printf("Other: %s\n", recvline);
            memset(recvline, 0 , sizeof(recvline));
            }
            close(socketfd);
            kill(pid, SIGUSR1);
            wait(&rv);
        break;
    }

    return 0;
}