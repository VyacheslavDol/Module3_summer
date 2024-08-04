#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include "msg_func.h"
#include "./library/input_char.h"
#include "string.h"
#include <mqueue.h>


int mrcv(char name[], char mtext[])
{   
    unsigned int m_prio;
    mqd_t ds;
    struct mq_attr attr;
if ((ds = mq_open(name, O_CREAT | O_RDONLY, 0600, NULL)) == (mqd_t)-1){
        perror("Creating queue error");
        fprintf(stderr, "Error: %d\n", errno);
        return -1;
    }   

    mq_getattr(ds, &attr);
        
    if (mq_receive(ds, mtext, attr.mq_msgsize, &m_prio) == -1){
        perror("cannot receive");
        fprintf(stderr, "Error: %d\n", errno);
        return -1;
    }

    if (mq_close(ds) == -1){
        perror("Closing queue error");
        fprintf(stderr, "Error: %d\n", errno);}
    
    return 0;
}

int msnd(char name[], char mtext[], unsigned int prio, long size)
{   
    mqd_t ds;
    if ((ds = mq_open(name, O_CREAT | O_WRONLY, 0666, NULL)) == (mqd_t)-1)
  {
    perror("Creating queue error");
    fprintf(stderr, "Error: %d\n", errno);
    return -1;
  }

    if (mq_send(ds, mtext, size, prio) == -1)
  {
    perror("Sending message error");
    fprintf(stderr, "Error: %d\n", errno);
    return -1;
  }

    if (mq_close(ds) == -1)
        perror("Closing queue error");

    return 0;
}