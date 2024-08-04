#ifndef MSG_FUNC_H_
#define MSG_FUNC_H_

struct msgbuf {
   long mtype;       /* тип сообщения, значение должно быть > 0 */
   char mtext[50];    /* данные сообщения */        
};

int msg_snd(int, int);

int msg_rcv(int, int);

#endif