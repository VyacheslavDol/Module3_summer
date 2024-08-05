#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

#define FNAME "numbers"
#define SEM_NAME "/semaname08"

//установка значения в семафор
int setvalue_sem(sem_t* sem, int goal_value)  
{   
    int semvalue;
    if(sem_getvalue(sem, &semvalue) == -1) return -1;
    while(semvalue != goal_value)
    {
        if(semvalue < goal_value)   
            sem_post(sem);      //+1
        if(semvalue > goal_value)
            sem_wait(sem);      //-1
        if(sem_getvalue(sem, &semvalue) == -1) return -1;
    }
    return 0;
}

//получение значения семафора
int getval(sem_t* sem)
{
    int value;
    sem_getvalue(sem, &value);
    return value;
}

int main(int argc, char *argv[]){

    if(argc != 2) 
        if(argc != 2)
    {
        printf("Incorrect input\n");
        exit(EXIT_FAILURE);
    };

    int count = atoi(argv[1]);

    pid_t pid;
    int fd[2], number, fd_read, fd_write, read_number, value;
    sem_t *sem;
    srand(time(NULL));

    //создание канала
    if(pipe(fd) != 0){  
        printf("Error pipe");
        exit(1);
    };

    // Создание семафора
    sem = sem_open(SEM_NAME, O_CREAT, 0664, 1);         //создание семафора с начальным значением 1
    if(sem == SEM_FAILED)
    {
        perror("sem_open");
        exit(EXIT_FAILURE);
    };

    //создание подпроцесса
    switch (pid = fork())
    {
    case -1:
        perror("fork");
        exit(EXIT_FAILURE);
        break;

    case 0: // Child proccess

        close(fd[0]); //закрываем канал на чтение

    for(int i = 0; i <= count - 1; i++){

        number = (rand() % 1000);   //генерация случайных чисел

        if(write(fd[1], &number, sizeof(number)) == 0){     //запись случайных числе в канал
            printf("Ошибка записи в файл\n");
            close(fd[1]);
            exit(EXIT_FAILURE);
        };
        
        printf("Child: Я записал в канал %d\n", number);
        /* printf("Child: я жду пока значение семафора не станет 0\n"); */

        while(getval(sem) != 0)     //ожидание 0
        {
            /* waiting 0 */
        };

        /* printf("Child: значение семафора = %d\n", semctl(sem, 0, GETVAL, arg)); */
        
        fd_read = open(FNAME, O_RDONLY);        //открытие файла на чтение

        if (fd_read == -1) {
        perror("Ошибка открытия файла для чтения");
        exit(EXIT_FAILURE);
        }

        if(read(fd_read, &read_number, sizeof(read_number)) == -1){
            printf("Прочитать информацию из файла не удалось");
            exit(EXIT_FAILURE);
        };

        printf("Child: Я прочитал из файла %d\n", read_number);
        close(fd_read);

        if(sem_post(sem) == -1)     //увеличение семафора на 1 (закрытие доступа самому себе)
        {
            perror("sem_post");
            exit(EXIT_FAILURE);
        }

    };  
        close(fd[1]);
        exit(EXIT_SUCCESS);

    default: // Parent proccess

        close(fd[1]); //закрытие канала на запись

        for(int i = 0; i <= count - 1; i++){        
        sleep(1);
        if(read(fd[0], &number, sizeof(number)) == 0){      //чтение числа из канала
                printf("Ошибка чтения в файл\n");
                close(fd[0]);
                return 1;
        };
        printf("Parrent: Я прочитал из канала  %d\n", number);
        
        if(setvalue_sem(sem, 1) == -1)      //установка значения семафора в 1;
        {
            perror("setvalue_sem");
            exit(1);
        }

        /* printf("Parrent: semval = %d\n", value); */

        fd_write = open(FNAME, O_WRONLY | O_CREAT, 0644);       //открытие файла для записи
        if (fd_write == -1) {
        perror("Ошибка открытия файла");
        exit(EXIT_FAILURE);
        }

        printf("Parrent: Доступ закрыт\n");

        if(write(fd_write, &number, sizeof(number)) == 0){    //запись полученного числа в файл
                printf("Ошибка записи в файл\n");
                close(fd[1]);
                return 1;
            };

        printf("Parrent: Я записал в файл %d\n", number);
        close(fd_write);    //закрытие файла

        if(sem_wait(sem) == -1)         //открытие доступа дочернему процессу (перевод семафора в 0)
        {
            perror("Par: sem_wait");
            exit(1);
        }

        /* printf("Parrent: semval = %d\n", semctl(sem, 0, GETVAL, arg)); */

        printf("Parrent: Доступ открыт\n");
        sleep(2);
        };

        wait(NULL);

        if(sem_unlink(SEM_NAME) == -1)
        {
            perror("sem_unlink");
            exit(1);
        };
        printf("Семафор удален\n");
        exit(EXIT_SUCCESS);
    }
}