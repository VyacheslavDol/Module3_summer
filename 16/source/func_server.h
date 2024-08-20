#ifndef FUNC_SERVER_H_
#define FUNC_SERVER_H_

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include "input_char.h"
#include "mymaths.h"
#include <fcntl.h>

#define FILENAME "file_server"
#define END_FILE "end of file"
#define BUFF_SIZE 64
typedef struct 
{
    double a1;  //первое число
    double a2;  //второ число
    char func[2];  //действие (+, - ..)
    double result; //результат
} math_msg;

void calculator(int sock);  //калькулятор

void file_changer(int sock);    //функция передачи файла

#endif 