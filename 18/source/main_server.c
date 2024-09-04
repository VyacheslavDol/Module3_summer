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
#include <sys/select.h>
#include "input_char.h"
#include "mymaths.h"
#include "func_server.h"

#define h_addr h_addr_list[0]  /* для совместимости с предыдущими версиями */
#define REQ_DATA "Waiting data\n"
//#define MENU "Выберите действие:\n1.Калькулятор\n2.Передача файлов\n0.Выход\n"

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
int sockfd, newsockfd, sock; // дескрипторы сокетов

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

    fd_set master; //набор всех имеющих дескрипторов
    fd_set read_fds; //дескрипторы на чтение(что-то куда-то пришло) 
    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    FD_SET(sockfd, &master); //добавление слушающего сокета в набор master
    int FD_MAX = sockfd;    //максимальное значение дескриптора
    int a;
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    //char buff[1024];

    while(1)
    {   
        read_fds = master;       
        if((a = select(FD_MAX + 1, &read_fds, NULL, NULL, NULL)) == -1)
        {
            perror("select");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        for(int i = 0; i <= FD_MAX; i++)
        {
            if(FD_ISSET(i, &read_fds))
            {
                printf("Работа с сокетом %d\n", i);
                if(i == sockfd)     /* Присоединение нового клиента */
                {
                    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen );
                    if (newsockfd < 0) error("ERROR on accept");
                    printf("Добавлен клиент с сокетом %d\n", newsockfd);
                    // вывод сведений о клиенте
                    struct hostent *hst;
                    hst = gethostbyaddr((char *)&cli_addr.sin_addr, 4, AF_INET);
                    printf("+%s [%s] new connect!\n", (hst) ? hst->h_name : "Unknown host", inet_ntoa(cli_addr.sin_addr));
                    nclients++;
                    printusers();

                    FD_SET(newsockfd, &master); //добавление нового дескриптора в общий набор
                    if(newsockfd > FD_MAX) FD_MAX = newsockfd;

                }
                else    //изменение на клиентском сокете
                {
                    /* Получение сообщения */
                    memset(buff, 0, sizeof(buff));
                    sock = i;                    
                    /* прием сообщения в зависиомти от содержимого которого зависят следующие действия */
                    if(recv(sock, (void*)buff, sizeof(buff), 0) == -1)
                    {
                        close(sock);
                        perror("recv");
                        exit(EXIT_FAILURE);
                    };
                    /*
                        Все возможные сообщения от клиентов:
                            - "start" - начало работы с сокетом - отправка клиенту меню
                            - "1" - запрос на калькулятор - ничего не отправлять
                            - "2" - запрос на передачу файла - отправка файла
                            - 16-ый байт =  '+' '-' '*' '/' - данные для калькулятора - отправка результата
                            - "3" - отсоединение клиента
                    */
                    if(strcmp(buff, "start") == 0)
                    {
                        /* отправка меню */
                        send_menu(sock);
                    }
                    else if((atoi(buff)) == 1)
                    {
                        /* клиент запросил калькулятор (ничего не нужно делать) */
                    }
                    else if((atoi(buff)) == 2)
                    {
                        /* отправка файла */
                        file_changer(sock);
                        send_menu(sock);
                    }
                    else if(buff[16] == '+' || buff[16] == '-' || buff[16] == '/' || buff[16] == '*')
                    {
                        /* расчет и отправка результата */
                        printf("Данные получены\n");
                        calculator_v2(sock, buff);
                        send_menu(sock);
                    }
                    else if(atoi(buff) == 3)
                    {
                        /* отсоединение клиента */
                        FD_CLR(sock, &master);
                        close(sock);
                        nclients--;
                        printusers();
                    }
                    else
                    {   
                        send_menu(sock);
                    }
                }
            }
        }
    }
    return 0;
}