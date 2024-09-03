//простой UDP сниффер

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/wait.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SIZE_BUFF 1080 //1000 - размер буфера отслеживаемого приложения, 60 - MAX Ip hdr, 20 - udp hdr
#define FNAME "packets"
int portno, exit_flag = 1;
char addr_dest[16]; //адресс в октетах

void exit_func()
{
    exit_flag = 0;
}

void pr_an(char *buffer, FILE* fd)  //функция вывода в файл анализа заголовков
{   
    unsigned char* buff = buffer;
    int nb = 0;
    fprintf(fd, "\nВерсия и длина заголовка в 32 битных словах: %x\n", buff[nb++]);
    fprintf(fd, "Тип сервиса: %x\n", buff[nb++]);
    fprintf(fd, "Полная длина: %x %x\n", buff[nb], buff[nb+1]); nb+=2;
    fprintf(fd, "Идентификатор: %x %x\n", buff[nb], buff[nb+1]); nb+=2;
    fprintf(fd, "Флаги и указатель фрагмента: %x %x\n", buff[nb], buff[nb+1]); nb+=2;
    fprintf(fd, "Время жизни: %x\n", buff[nb++]);
    fprintf(fd, "Протокол: %x\n", buff[nb++]);
    fprintf(fd, "Контрольная сумма: %x %x\n", buff[nb], buff[nb+1]); nb+=2;
    fprintf(fd, "IP отправителя: "); 
        for(int i = 0; i < 4; i++){
            fprintf(fd,"%x ", buff[nb++]);}
    fprintf(fd, "\nIP получателя: "); 
        for(int i = 0; i < 4; i++){
            fprintf(fd,"%x ", buff[nb++]);}
    fprintf(fd, "\nПорт отправителя: %x %x\n", buff[nb], buff[nb+1]); nb+=2;
    fprintf(fd, "Порт отправителя: %x %x\n", buff[nb], buff[nb+1]); nb+=2;
    fprintf(fd, "Длина пакета: %x %x\n", buff[nb], buff[nb+1]); nb+=2;
    fprintf(fd, "Контрольгая сумма: %x %x\n", buff[nb], buff[nb+1]); nb+=2;
    
}

void process_packet(unsigned char *buffer, int size)
{
    struct iphdr *ip_header = (struct iphdr *)buffer;
    struct udphdr *udp_header = (struct udphdr *)(buffer + ip_header->ihl * 4);
    int cmp = strcmp(addr_dest, inet_ntoa(*(struct in_addr *)&ip_header->daddr));
    
    FILE* fd = fopen(FNAME, "a+");  //открытие файла для записи
    if((cmp == 0 && (portno == ntohs(udp_header->dest))) || addr_dest[0] == '0')  //фильтрация если пакет подходит то выводится инфа, ip = 0 - все пакеты данного протокола
    {
    // Вывод информации о пакете на экран и в файл
    printf("Received UDP Packet:\n"); fprintf(fd, "Received UDP Packet:\n");
    printf("Source IP: %s\n", inet_ntoa(*(struct in_addr *)&ip_header->saddr)); fprintf(fd, "Source IP: %s\n", inet_ntoa(*(struct in_addr *)&ip_header->saddr)); 
    printf("Destination IP: %s\n", inet_ntoa(*(struct in_addr *)&ip_header->daddr)); fprintf(fd, "Destination IP: %s\n", inet_ntoa(*(struct in_addr *)&ip_header->daddr));
    printf("Source Port: %d\n", ntohs(udp_header->source)); fprintf(fd, "Source Port: %d\n", ntohs(udp_header->source));
    printf("Destination Port: %d\n", ntohs(udp_header->dest)); fprintf(fd, "Destination Port: %d\n", ntohs(udp_header->dest));
    printf("Payload Size: %ld bytes\n", size - (ip_header->ihl * 4) - sizeof(struct udphdr)); fprintf(fd, "Payload Size: %ld bytes\n", size - (ip_header->ihl * 4) - sizeof(struct udphdr));
    int offset = (ip_header->ihl * 4) + sizeof(struct udphdr); //смещение до DATA
    buffer[size] = '\0';
    printf("Data: %s\n", &buffer[offset]); fprintf(fd, "Data: %s\n", &buffer[offset]);      //Вывод Data по ASCII
    printf("Data (Bytes): ");  fprintf(fd, "Data: ");
    for(int i = 0; i < (size - (ip_header->ihl * 4) - sizeof(struct udphdr)); i++ )
    {
        printf("%x ", buffer[offset + i]); fprintf(fd, "%x ", buffer[offset + i]);     //Вывод по байтно
    }
    pr_an(buffer, fd); //вывод анализа заголовка по байтам (только udp, ip заголовок 20 байт)
    fprintf(fd, "\nBytes all: ");
        for(int i = 0; i < size; i++)
    {
        fprintf(fd, "%x ", buffer[i]);     //Вывод по байтно
    }
    
    printf("\n\n"); fprintf(fd, "\n\n");
    }
    fclose(fd);
    return;
}

int main(int argc, char* argv[])    // sudo ./snif 127.0.0.1 50002  (./snif <адрес назначения> <порт назначения>)  
{      
    printf("argc = %d\n", argc);
    if(argc > 3 && argc < 2)
    {
        printf("Error, use sudo ./shif <адрес назначения> <порт назначения>\n");
        exit(EXIT_FAILURE);
    }

    int raw_sock; //дескриптор сокета
    pid_t pid; //для дочернег процесса
    char* buff;     //буффер raw socket
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    size_t recv_size;

    signal(SIGINT, exit_func);  //настройка выхода из приложения, реакция CTRL+C

    if(argc == 2)    //номер порта
        portno = 0;
    else
        portno = atoi(argv[2]);

    strcpy(addr_dest, argv[1]);     //адрес

    printf("Адрес и порты для отлова добычи %s, %d\n", addr_dest, portno);

    buff = malloc(SIZE_BUFF + 1);
    if(buff == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    raw_sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP); //создание сырого сокета
    if(raw_sock < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
        free(buff);
    }
    
    while(exit_flag)
    {
        if((recv_size = recvfrom(raw_sock, buff, SIZE_BUFF, 0, (struct sockaddr*)&addr, &addr_len)) == -1) //получение копии пакета
        {
            perror("recvfrom");
            close(raw_sock);
            free(buff);
            exit(EXIT_FAILURE);
        };

        // Обработка полученного пакета
        process_packet(buff, recv_size);

    }

    close(raw_sock);
    free(buff);
    printf("Закрытие программы...\n");
    exit(EXIT_SUCCESS);
}