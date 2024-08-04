#ifndef MSG_FUNC_H_
#define MSG_FUNC_H_
#define QUEUE1 "/q1"
#define QUEUE2 "/q2"
#define SIZE 50

int mrcv(char name[], char mtext[]);
int msnd(char name[], char mtext[], unsigned int prio, long size);

#endif