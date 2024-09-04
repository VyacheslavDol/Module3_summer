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
#define MENU "Bыберите действие:\n1.Калькулятор\n2.Передача файлов\n3.Выход\n"

typedef struct 
{
    double a1;  //первое число      8 байт
    double a2;  //второ число       8 байт
    char func[2];  //действие (+, - ..) 2 байта (дополняется до 8)
    double result; //результат
} math_msg;

void calculator_v2(int sock, char* buff); //калькулятор, buff = NULL => получение данных внутри функции, иначе buff - данные калькулятору

void send_menu(int sock);    /* отправка меню */

void calculator(int sock);  //калькулятор

void file_changer(int sock);    //функция передачи файла

#endif 