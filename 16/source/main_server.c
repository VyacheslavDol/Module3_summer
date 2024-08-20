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
#include "func_server.h"

#define h_addr h_addr_list[0]  /* для совместимости с предыдущими версиями */
#define REQ_DATA "Waiting data\n"
#define MENU "Выберите действие:\n1.Калькулятор\n2.Передача файлов\n0.Выход\n"

// функция обслуживания подключившихся пользователей
void client_req(int);

// функция обработки ошибок
void error(const char *msg) 
{
    perror(msg);
    exit(1);
}

// количество активных пользователей
int nclients = 0;
int sockfd, newsockfd; // дескрипторы сокетов

// печать количества активных пользователей
void printusers()
{
    if(nclients) 
    {
        printf("%d user online\n", nclients);
    } else 
    {
        printf("No User online\n");
    }
    return;
}

void counter_plus()     //SIGUSR1
{
    nclients++;
    printusers();
}

void counter_min()      //SIGUSR2
{
    nclients--;
    printusers();
}

int main(int argc, char *argv[])
{   
    char buff[1024]; // Буфер для различных нужд
    int portno; // номер порта
    int pid; // id номер потока
    int fd; //файловый дескриптор для передачи файла
    int ans;
    socklen_t clilen; // размер адреса клиента типа socklen_t
    struct sockaddr_in serv_addr, cli_addr; // структура сокета сервера и клиента
    
    signal(SIGUSR1, counter_plus);
    signal(SIGUSR2, counter_min);
    printf("TCP SERVER DEMO\n");

    // ошибка в случае если мы не указали порт
    if (argc < 2) 
    {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    // Шаг 1 - создание сокета
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    // Шаг 2 - связывание сокета с локальным адресом
    bzero((char*) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; // сервер принимает подключения на всех доступных IP, (сервер имеет множество IP адресов и один конкретный порт)
    serv_addr.sin_port = htons(portno); //порт

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    // Шаг 3 - ожидание подключений, размер очереди - 5
    listen(sockfd, 5);

    clilen = sizeof(cli_addr);

    // Шаг 4 - извлекаем сообщение из очереди (цикл извлечения запросов на подключение)
    while (1)
    {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen );
        if (newsockfd < 0) error("ERROR on accept");

        // вывод сведений о клиенте
        struct hostent *hst;
        hst = gethostbyaddr((char *)&cli_addr.sin_addr, 4, AF_INET);
        printf("+%s [%s] new connect!\n", (hst) ? hst->h_name : "Unknown host", inet_ntoa(cli_addr.sin_addr));
        printusers();
        pid = fork();
        if (pid < 0) error("ERROR on fork");

        if (pid == 0) //дочерний процесс
        {   
            kill(getppid(), SIGUSR1);   //счетчик
            close(sockfd);  //закрытие сокета приема пользователей
            client_req(newsockfd); //функция для работы с клиентом
            close(newsockfd); 
            exit(0);
        } else  //родительский процесс
            close(newsockfd);
    }
    return 0;
}

void client_req(int sock)   //функция работы с клиентом
{      
    char buff[1024];

    /* прием первого сообщения начала (start) */
    if(recv(sock, buff, sizeof(buff), 0) == -1)
    {
        close(sock);
        perror("recv first");
        exit(EXIT_FAILURE);
    };

    while(1)
    {
        /* отправка меню */
        memset(buff, 0, sizeof(buff));
        strcpy(buff, MENU);
        if(send(sock, buff, strlen(buff), 0) == -1)
        {
            close(sock);
            perror("send menu");
            exit(EXIT_FAILURE);
        }

         if(recv(sock, buff, sizeof(int), 0) == -1)
        {
            close(sock);
            perror("recv ans");
            exit(EXIT_FAILURE);
        }

        switch (atoi(buff)){
            case 1:

            /* CALCULATOR */
            calculator(sock);
                break;
   
            case 2:

            /* FILE CHANGER */
            file_changer(sock);
            break;
            
            case 0: 
                
                kill(getppid(), SIGUSR2);
                return; 

            default:
                break;
        } 
        
    };

}
