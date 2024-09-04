#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <strings.h>
#include <fcntl.h>
#include "input_char.h"
#include <sys/time.h>
#include <errno.h>

#define h_addr  h_addr_list[0]  /* для совместимости с предыдущими версиями */
#define FILENAME "file_client"
#define END_FILE "end of file"
#define BUFF_FILE 64
#define EXIT_MSG "3"
#define SIZE_BUFF 100

int exit_flag = 1;

void exit_func()
{
    exit_flag = 0;
}

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

void file_chang(int sock)
{
    int fd;
    char buff[BUFF_FILE + 1]; //буффер приема
    buff[BUFF_FILE] = '\n';
    fd = open(FILENAME, O_CREAT | O_WRONLY, 0771); //создание файла в который буде записиываться полученная информация
    if(fd == -1)
    {
        perror("open file");
        close(sock);
        exit(EXIT_FAILURE);
    }
    memset(buff, 0, sizeof(buff));
    while((recv(sock, &buff, BUFF_FILE, 0)) != 0) {
        if(strcmp(buff, END_FILE) == 0) 
        {   
            close(fd);
            break;
        }
        write(fd, buff, strlen(buff));  //запись в файл
        memset(buff, 0, sizeof(buff));
    };
    printf("Файл получен\n");
    return;
}

void calc(int my_sock)
{   
    math_msg snd_buff, rcv_buff;

    printf("Действие (+ - / *): "); input_char(snd_buff.func, sizeof(snd_buff.func));
    printf("Первое число: ");   snd_buff.a1 = get_double();
    printf("Второе число: ");   snd_buff.a2 = get_double();
    bzero(&(snd_buff.result), sizeof(snd_buff.result));

    /* отправка данных */
    if(send(my_sock, (void*)&snd_buff, sizeof(snd_buff), 0) == -1)
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
    
    char* buff = (char*)&rcv_buff;
    char buff_err[SIZE_BUFF];
    if(buff[0] == 'B')    //пришло меню из-за некорректного ввода в функции
    {
        printf("Incorrect input\n");
        recv(my_sock, buff_err, SIZE_BUFF, 0);
        if(send(my_sock, "start", 5, 0) == -1)
        {
            close(my_sock);
            perror("send");
            exit(EXIT_FAILURE);
        };
        return;
    }
    /* вывод полученных результатов */
    printf("S> %.3lf %c %.3lf = %.3lf\n", 
                    rcv_buff.a1, rcv_buff.func[0], rcv_buff.a2, rcv_buff.result);
    return;
}

int main(int argc, char *argv[])
{
    int my_sock, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buff[1024];
    math_msg snd_buff, rcv_buff;

    signal(SIGINT, exit_func);

    printf("print <start> to start\n");
    
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

    while(exit_flag)
    {       /* МЕНЮ */
        if(recv(my_sock, buff, sizeof(buff), 0) == -1)
        {
            close(my_sock);
            perror("recv");
            exit(EXIT_FAILURE);
        };
        printf("S: %s", buff);
        
        input_char(buff, sizeof(buff));

        if(send(my_sock, buff, strlen(buff), 0) == -1)
        {
            close(my_sock);
            perror("send start");
            exit(EXIT_FAILURE);
        }


        switch (atoi(buff))
        {
        case 1:
            /*CALCULATOR */
            
            calc(my_sock);
        
            break;
        
        case 2:
            /* FILE download */

            file_chang(my_sock);

            break;

        case 3:
            printf("закрытие сессии\n");
            close(my_sock);
            exit(EXIT_SUCCESS);            
            
        default:
            break;
        }
    }
    
    if(send(my_sock, EXIT_MSG, strlen(EXIT_MSG), 0) == -1)
    {
        close(my_sock);
        perror("send start");
        exit(EXIT_FAILURE);
    }

    printf("закрытие сессии\n");
    close(my_sock);
    exit(EXIT_SUCCESS);
}