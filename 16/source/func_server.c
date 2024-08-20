#include "func_server.h"


void calculator(int sock)
{   
    math_msg msg;
    int flag;
    double (*operation)(double, double); //function pointer

    if(recv(sock, &msg, sizeof(msg), 0) == -1)
    {
        close(sock);
        perror("recv msg");
        exit(EXIT_FAILURE);
    };

    printf("a1 = %lf, a2 = %lf, d = %s\n", msg.a1, msg.a2, msg.func);
    flag = 1;
    /* обработка полученных данных */
    switch ((int)msg.func[0])
    {
    case '+':
        operation = mysum;
        break;

    case '-':
        operation = mysubstr;
        break;

    case '/':
        if(msg.a2 == 0)
        {   
            flag = -1;
            msg.result = 0;
            break;
        }
        operation = mydiv;
        break;

    case '*':
        operation = mymult;
        break;

    default:
    msg.result = 0;
    flag = -1;
        break;
    }

    if(flag == 1)
    msg.result = operation(msg.a1, msg.a2); //вычисление действия

    //отправка ответа с резульатотм действия
    if(send(sock, &msg, sizeof(msg), 0) == -1)
    {
        close(sock);
        perror("send reply");
        exit(EXIT_FAILURE);
    }
}

void file_changer(int sock)
{
    int fd;
    char buff[BUFF_SIZE]; //буффер приема
    char rcv_buff[2];
    memset(buff, 0, sizeof(buff));
    fd = open(FILENAME, O_RDONLY); //отрытие файла для отправки
    if(fd == -1)
    {
        perror("open file");
        close(sock);
        exit(EXIT_FAILURE);
    }

    ssize_t a, b;
    buff[BUFF_SIZE] = '\0';
    printf("buff[64] = %x", buff[64]);
    while ((a = read(fd, buff, BUFF_SIZE)) > 0){    //работает но криво при sizeof(buff) просто
    printf("size read a = %ld \n", a);
    printf("buff[64] = %x, buff[65] = %x, buff[63] = %x %c, buff[0] = %c\n", buff[64], buff[65], buff[63], buff[63], buff[0]);
    if((b = send(sock, &buff, BUFF_SIZE, 0)) == -1)
    {
        close(sock);
        perror("send");
        exit(EXIT_FAILURE);
    };

    printf("Buff = %s\n", buff);
    printf("strlen = %ld\n", strlen(buff));
    memset(buff, 0, BUFF_SIZE);
    printf("Buff[0] = %c, buff = %s\n", buff[0], buff);
    };

    if(send(sock, END_FILE, sizeof(END_FILE), 0) == -1)
    {
        close(sock);
        perror("send");
        exit(EXIT_FAILURE);
    };

    close(fd);
    return;
}