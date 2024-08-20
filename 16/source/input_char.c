#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "input_char.h"


void input_char(char* buff, long size_buff){            //функция корректного и удобного ввода с помощью fgets с очисткой буффера
    int c;
    memset(buff, 0, size_buff);
    fgets(buff, size_buff, stdin);
    buff[strcspn(buff, "\n")] = '\0';
    if(strcmp(&buff[size_buff-2], "\0"))
    while ((c = fgetc(stdin)) != '\n' && c != EOF);
};

int get_ans_int(){                                              //функция получения ответа в виде чисел (от 0 до 99)
    int ansi;
    char ans[10];
    input_char(ans, sizeof(ans));
    ansi = atoi(ans);
    return ansi;
}

double get_double(){    //функция получения double
    double a;
    char buff[50];
    char* endptr;
    input_char(buff, sizeof(buff));
    a = strtod(buff, &endptr);
    if(endptr == buff)
        printf("Incorrect input\n"); 
    return a;
}