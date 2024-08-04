#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <unistd.h>
#include "./library/input_char.h"
#include "msg_func.h"
#include <signal.h>

#define MTYPE_READ 2
#define MTYPE_WRITE 1

int main(){

    int msgid, number;
    struct msgbuf message;
    key_t key;
    pid_t pid;

    key = ftok("/home", 'q');       //создание уникального ключа
    if(key == -1){
        printf("Ошибка создания ключа подключения\n");
        exit(1);
    }
    
    msgid = msgget(key, IPC_CREAT | 0666);      //создание очереди
    if(msgid == -1) {
    printf("Ошибка создания очереди\n");
    exit(1);
    };

    switch(pid=fork()) {
    case -1:
        perror("fork"); /* произошла ошибка */
        exit(1); /*выход из родительского процесса*/
    case 0:  //child process  Чтение из очереди
         
        message.mtype = MTYPE_READ;
        
        while(msg_rcv(msgid, message.mtype) != -1)
        {/* чтение сообщений*/   };
        pid = getppid();
        kill(pid, SIGINT);
    break;
    default: //parent process   отправка сообщений
        message.mtype = MTYPE_READ;
        while(msg_snd(msgid, message.mtype) != -1)
        {/* отправка сообщений */};
        kill(pid, SIGINT);
        if(msgctl(msgid, IPC_RMID, 0) == -1) //удаление очереди после завершения работы программы
        {
            printf("Удалить очередь не удалось\n");
            exit(1);
        };
    break;
  }

    exit(0);
}