#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <pthread.h>

#define FNAME "numbers.txt"
#define CH_PR 5
#define NUM_ACCESS 2
#define SEM0_NAME "/somename0"
#define SEM1_NAME "/somename1"
#define SEM2_NAME "/somename2"

//устаановка значения в семафор
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

int parent_func(sem_t *sem1, sem_t *sem2, int* fd, int count){     //чтение случайных чисел из канала , отрпавка их в файл и регулировние доступом

    close(fd[1]); //закрытие канала на запись
    int number, fd_write, value = 0;
    
    fd_write = open(FNAME, O_WRONLY | O_CREAT, 0644);       //открытие файла для записи
        if (fd_write == -1) {
        perror("Ошибка открытия файла");
        return 1;
    };

    for(int i = 0; i < count; i++){       

    if(read(fd[0], &number, sizeof(number)) == 0){      //чтение числа из канала
            printf("Ошибка чтения в файл\n");
            close(fd[0]);
            return 1;
    };
    printf("Parrent: Я прочитал из канала  %d\n", number);

    if(write(fd_write, &number, sizeof(number)) == 0){    //запись полученного числа в файл
        printf("Ошибка записи в файл\n");
        close(fd_write);
        return 1;
    };

    printf("Parrent: Я записал в файл %d\n", number);

    //восстановление 2го семафора до макс значения
    if(setvalue_sem(sem2, CH_PR + 1) == -1) return 1;

    //открытие доступа  (1ый семафор значение = 0)
    if(sem_wait(sem1) == -1) return 1;
    printf("Parent: Доступ открыт sem1 = %d\n", getval(sem1));

    //ожидание пока все доч процессы чтения не прочтут файл (пока sem 2 не будет равен 1)
    while(getval(sem2) != 1)
    {
        //ожидание
    };

    //закрытие доступа (1ый в 1)
    if(sem_post(sem1) == -1) return 1;
    printf("Parent: Доступ закрыт sem1 = %d\n", getval(sem1));

    //перевод sem 2 в знаечние 0
    if(sem_wait(sem2) == -1) return 1;
    //printf("Parr: sem2 = %d\n", getval(sem2));
}
    close(fd_write);    //закрытие файла
    close(fd[0]);
return 0;
}

int child_gen2(sem_t *sem0, sem_t *sem1, sem_t *sem2, int* fd, int count)
{      
    int read_number;
    int reading_proc, fd_read;

    //открытие файла на чтение
    fd_read = open(FNAME, O_RDONLY | O_CREAT, 0666);        
    if (fd_read == -1) {
    perror("Ошибка открытия файла для чтения");
    return 1;
    };

    for(int i = 0; i < count; i++)
    {
    //ожидание открытия доступа на чтение
    while(getval(sem1) != 0)
    {
        //ожидание
    }

    //подключение к чтению файла
    if(sem_wait(sem0) == -1) return 1;

    //чтение числа из файла
    if(read(fd_read, &read_number, sizeof(read_number)) == -1){
        printf("Прочитать информацию из файла не удалось\n");
        return 1;
    };
    //отключение от чтения
    if(sem_post(sem0) == -1) return 1;

    printf("Child %d: Я прочитал из файла %d\n", getpid(), read_number);
    
    //"отметка" о прочтении
    if(sem_wait(sem2) == -1) return 1;

    //ожидание всех процессов чтения
    while(getval(sem2) != 0)
    {
        //ожидание
    }
    };
    close(fd_read);
    return 0;
}

int child_gen1(int count, int* fd) //генерация случайных чисел и отправка их в канал
{
    close(fd[0]); //закрываем канал на чтение
    int number;
    for(int i = 0; i <= count - 1; i++){
        number = (rand() % 1000);   //генерация случайных чисел

        if(write(fd[1], &number, sizeof(number)) == 0){     //запись случайных числе в канал
            printf("Ошибка записи в файл\n");
            close(fd[1]);
            exit(EXIT_FAILURE);
        };
        printf("Child: Я записал в канал %d\n", number);
}
    close(fd[1]);
    return 0;
}

int main(int argc, char* argv[])
{
    if(argc != 2) 
        if(argc != 2)
    {
        printf("Incorrect input\n");
        exit(EXIT_FAILURE);
    };

    int count = atoi(argv[1]);

    pid_t pid1, pid2;
    int fd[2], number, fd_read, fd_write, read_number, value;    
    sem_t *sem0;
    sem_t *sem1;
    sem_t *sem2;
    srand(time(NULL));

    //создание канала
    if(pipe(fd) != 0){  
        printf("Error pipe");
        exit(1);
    };

    // Создание 3 семафоров
    sem0 = sem_open(SEM0_NAME, O_CREAT, 0664, NUM_ACCESS);      // 0 - кол-во процессов которые читают файл в данный момент времени
    sem1 = sem_open(SEM1_NAME, O_CREAT, 0664, 1);               // 1 - доступ к чтению (0/1)
    sem2 = sem_open(SEM2_NAME, O_CREAT, 0664, CH_PR + 1);       //2 - кол-во процессов поричтавших файл
    if(sem0 == SEM_FAILED || sem1 == SEM_FAILED || sem1 == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    //выставление начальных значений семафоров
    if(setvalue_sem(sem1, 1) == -1) exit(EXIT_FAILURE);        

    if(setvalue_sem(sem2, CH_PR + 1) == -1) exit(EXIT_FAILURE);   

    if(setvalue_sem(sem0, NUM_ACCESS) == -1) exit(EXIT_FAILURE);

    switch (pid1 = fork())
    {
    case -1:
        perror("fork");
        exit(EXIT_FAILURE);
        break;

    case 0: // Child proccess

        child_gen1(atoi(argv[1]), fd);    //дочерний процесс, посылающий случайные числа в канал

        exit(EXIT_SUCCESS);

    default: // Parent proccess

        //порождение 5 дочерних процессов
        for (int i = 0; i < CH_PR; i++) {
        pid2 = fork();
        
        if (pid2 == 0) {
            //дочерние процесс читающий инфорамацию из файла
            if(child_gen2(sem0, sem1, sem2, fd, atoi(argv[1])) == 1)
            {
                perror("ch_error");
                exit(EXIT_FAILURE);
            };
            sem_close(sem0); sem_close(sem1); sem_close(sem2); 
            exit(EXIT_SUCCESS);
        } else if (pid2 > 0) {
            // Родительский процесс продолжает выполнение цикла
        } else {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }
        if(pid2 > 0)        //родительский процесс, прием чисел из канала, запись чисел в файл, регулировка семафорами
        {
            if(parent_func(sem1, sem2, fd, atoi(argv[1])) == 1)
            {
                perror("par_error");
                exit(EXIT_FAILURE);
            };
            sem_close(sem0); sem_close(sem1); sem_close(sem2);
            //ожидание закрытия всез доч процессов
            for(int i = CH_PR; i > 0; i--){
                wait(0);
            };
            sem_unlink(SEM0_NAME); sem_unlink(SEM1_NAME); sem_unlink(SEM2_NAME); //удаление семафоров
        printf("Семафоры удалены\n");
        exit(EXIT_SUCCESS);
        }
    }
}