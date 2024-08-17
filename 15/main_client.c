#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include "input_char.h"

#define h_addr  h_addr_list[0]  /* для совместимости с предыдущими версиями */
#define EXIT_WORD "exit"

void error(const char *msg) {
perror(msg);
exit(0);
}

typedef struct  {
    double a1;  //первое число
    double a2;  //второ число
    char func[2];  //действие (+, - ..)
    double result; //результат
} math_msg;

int main(int argc, char *argv[])
{
    int my_sock, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buff[1024];
    math_msg snd_buff, rcv_buff;

    printf("TCP DEMO CLIENT\n");
    
    if (argc < 3) 
    { 
        fprintf(stderr, "usage %s hostname port\n",
        argv[0]);
        exit(0);
    }

    // извлечение порта
    portno = atoi(argv[2]);

    // Шаг 1 - создание сокета
    my_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (my_sock < 0) error("ERROR opening socket");
    
    // извлечение хоста
    server = gethostbyname(argv[1]);
    if (server == NULL) 
    {
        perror("server: no such host\n");
        exit(0);
    }
    
    // заполнение структуры serv_addr
    bzero((char*) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char*)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
    server->h_length);
    
    // установка порта
    serv_addr.sin_port = htons(portno);

    // Шаг 2 - установка соединения
    if (connect(my_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");
    
    // Шаг 3 - чтение и передача сообщений

    do{
    input_char(buff, sizeof(buff));     /* ожидание слова start для начала передачи */
    } while(strcmp(buff, "start"));

    if(send(my_sock, buff, strlen(buff) + 1, 0) == -1)
    {
        close(my_sock);
        perror("send start");
        exit(EXIT_FAILURE);
    }

    if(recv(my_sock, buff, sizeof(buff), 0) == -1)
    {
        close(my_sock);
        perror("recv first");
        exit(EXIT_FAILURE);
    };

    /* вывод первого полученного сообщения */
    printf("S: %s", buff);

    while(1)
    {
        /* заполение msg_buf */

        printf("Действие (+ - / *): ");
        input_char(snd_buff.func, sizeof(snd_buff.func));

        if(!(strcmp(snd_buff.func, "q"))) /* Разрыв соединения при "quit" */
        {   
            printf("закрытие сессии\n");
            strcpy(snd_buff.func, "q");
            send(my_sock, &snd_buff, sizeof(snd_buff), 0);
            close(my_sock);
            exit(EXIT_SUCCESS);
        };

        printf("Первое число: ");   snd_buff.a1 = get_double();
        printf("Второе число: ");   snd_buff.a2 = get_double();
        bzero(&(snd_buff.result), sizeof(snd_buff.result));
        
        /* отправка данных */
        if(send(my_sock, &snd_buff, sizeof(snd_buff), 0) == -1)
        {
            close(my_sock);
            perror("send");
            exit(EXIT_FAILURE); 
        };

        if(recv(my_sock, &rcv_buff, sizeof(rcv_buff), 0) == -1)
        {
            close(my_sock);
            perror("recv");
            exit(EXIT_FAILURE);
        };

        /* вывод полученных результатов */
        printf("S> %.3lf %c %.3lf = %.3lf\n", rcv_buff.a1, rcv_buff.func[0], rcv_buff.a2, rcv_buff.result);
        
    }
}