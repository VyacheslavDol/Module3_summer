#include "func_server.h"


void send_menu(int sock)    /* отправка меню */
{   
    char buff[sizeof(MENU)];     
    memset(buff, 0, sizeof(buff));
    strcpy(buff, MENU);
    if(send(sock, buff, strlen(buff), 0) == -1)
    {
        close(sock);
        perror("send menu");
        return;
    }
    return;
}


void calculator_v2(int sock, char* buff)
{
    math_msg* msgp = (math_msg*)buff;

    int flag;
    double (*operation)(double, double); //function pointer

    if(buff == NULL){
        if(recv(sock, msgp, sizeof(math_msg), 0) == -1)
        {
            close(sock);
            perror("recv msg");
            return;
        };
    }
    
    printf("a1 = %lf, a2 = %lf, d = %s\n", msgp->a1, msgp->a2, msgp->func);
    flag = 1;
    /* обработка полученных данных */
    switch ((int)msgp->func[0])
    {
    case '+':
        operation = mysum;
        break;

    case '-':
        operation = mysubstr;
        break;

    case '/':
        if(msgp->a2 == 0)
        {   
            flag = -1;
            msgp->result = 0;
            break;
        }
        operation = mydiv;
        break;

    case '*':
        operation = mymult;
        break;

    default:
    msgp->result = 0;
    flag = -1;
        break;
    }

    if(flag == 1)
    msgp->result = operation(msgp->a1, msgp->a2); //вычисление действия

    //отправка ответа с резульатотм действия
    if(send(sock, msgp, sizeof(math_msg), 0) == -1)
    {
        close(sock);
        perror("send reply");
        return;
    }
    return;
}

void calculator(int sock)
{   
    math_msg msg;
    int flag;
    double (*operation)(double, double); //function pointer

    if(recv(sock, &msg, sizeof(msg), 0) == -1)
    {
        close(sock);
        perror("recv msg");
        return;
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
        return;
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
        return;
    }

    ssize_t a, b;
    buff[BUFF_SIZE] = '\0';
    while ((a = read(fd, buff, BUFF_SIZE)) > 0){ 
    if((b = send(sock, &buff, BUFF_SIZE, 0)) == -1)
    {
        close(sock);
        perror("send");
        return;
    };

    memset(buff, 0, BUFF_SIZE);
    };

    if(send(sock, END_FILE, sizeof(END_FILE), 0) == -1)
    {
        close(sock);
        perror("send");
        return;
    };
    printf("Файл отправлен\n");
    close(fd);
    return;
}