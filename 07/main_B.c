#include <stdio.h>
#include <string.h>
#include <mqueue.h>
#include "./library/input_char.h"
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include "msg_func.h"
#include <signal.h>

int main(){

    mqd_t ds;
    char text1[SIZE];
    pid_t pid;
    unsigned int prio = 1;

switch(pid = fork()) {
    case -1:
        perror("fork");     // произошла ошибка
        exit(1); 

    case 0:                  //child process  чтение из очереди
    do{
        if(mrcv(QUEUE1, text1) == 0)
        printf("A: %s\n", text1);
    }while(strcmp("пока", text1) != 0);
    pid = getppid();
    kill(pid, SIGINT);
    break;

    default: //parent process   отправка сообщений
    prio = 1;  
    do{
    input_char(text1, sizeof(text1));
    msnd(QUEUE2, text1, prio, sizeof(text1));
    }while(strcmp("пока", text1) != 0);
    kill(pid, SIGINT);
    break;
  }

    if (mq_unlink(QUEUE2) == -1);    //удаление очереди
    //perror("Removing queue error");

    return 0;
}