#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include "msg_func.h"
#include "./library/input_char.h"
#include "string.h"

int msg_snd(int qid, int mtype)
{
    struct msgbuf msg;
    msg.mtype = mtype;

    input_char(msg.mtext, sizeof(msg.mtext));
    
    if(msgsnd(qid, &msg, sizeof(msg.mtext), 0) == -1)                   //отправка сообщения в очередь
       {
           printf("Ошибка отправки\n");;
           exit(1);
       }
    if(strcmp(msg.mtext, "пока") == 0) return -1;
    return 0;
}

int msg_rcv(int qid, int mtype)
{
    struct msgbuf msg;
    
    if(msgrcv(qid, &msg, sizeof(msg.mtext), mtype, 0) == -1)     
        {
            printf("Прочитать сообщение не удалось\n");
            exit(1);
        };
        
    printf("Another: %s\n", msg.mtext);
    if(strcmp(msg.mtext, "пока") == 0) return -1;
    return 0;
}