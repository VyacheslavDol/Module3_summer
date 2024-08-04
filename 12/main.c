#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h> 
#include <time.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <signal.h>
#include <sys/wait.h>


#define RAND_NUM 5
#define MAX_VALUE 10000


int exit_flag = 1; //флаг выхода в случае SIGINT перевод в 0

union semun {
int val;                /* значение для SETVAL */
struct semid_ds *buf;   /* буферы для IPC_STAT, IPC_SET */
unsigned short *array;  /* массивы для GETALL, SETALL */
/* часть, особенная для Linux: */
struct seminfo *__buf;  /* буфер для IPC_INFO */
};

int find_max(int *array, int count)     //count - кол-во сгенерированных случайных чисел
{
    int max = array[0];     //array[0] здесь равен array[2] в main
    for(int i = 1; i < count; i++)
    {
        if(array[i] > max)
            max = array[i];
    }
    return max;
}

int find_min(int *array, int count)
{
    int min = array[0];     //array[0] здесь равен array[3] в main
    for(int i = 1; i < count; i++)
    {
        if(array[i] < min)
            min = array[i];
    }
    return min;
}

void react_sigint()
{
    exit_flag = 0;
}


int main()
{   

    int sem, value;
    union semun arg;
    
    /* операции с семафорами */
    struct sembuf wait_0 = {0, 0, 0};       //"ожидание нуля" сем0
    struct sembuf open_sem = {0, -1, 0};    //уменьшение значения семофора0 на -1
    struct sembuf close_sem = {0, 1, 0};    //увеличение значения семофора0 на +1
    struct sembuf wait_0_sem1 = {1, 0, 0};   //"ожидание нуля" сем1
    struct sembuf open_sem1 = {1, -1, 0};    //уменьшение значения семофора1 на -1 отметка о прочитке
    struct sembuf close_sem1 = {1, 1, 0};    //увеличение значения семофора1 на +1 
    
    pid_t pid;
    int rv;
    int count = 0;     //счетчик операций
    int *array;    // Указатель на разделяемую память 
    int shmid;     // IPC дескриптор для области разделяемой памяти
    char pathname[] = "main.c";
    key_t key, key_sem;     // IPC ключ

    /* изменение реакции на сигнал SIGINT */
    signal(SIGINT, react_sigint);

    if((key = ftok(pathname, 0)) < 0){
        printf("Can\'t generate key\n");
        exit(-1);
    }

    //создание области разделяемой памяти
    shmid = shmget(key, (3 + RAND_NUM)*sizeof(int), IPC_CREAT | IPC_EXCL | 0666);
    if (shmid == -1) {
        perror("shmget1");
        // Проверим, является ли ошибка "File exists"
        if (errno == EEXIST) {
           // Сегмент уже существует, вы можете получить его идентификатор
           shmid = shmget(key, 0, 0); // Получаем идентификатор существующего сегмента
            if (shmid == -1) {
               perror("shmget2");
               exit(1);
            }
       } else {
           exit(1);
    }
    }
    //создание семафоров
    key_sem = ftok("/home", 'q');       
    if(key == -1)
    {
        printf("Ошибка создания ключа подключения\n");
        exit(1);
    }

    sem = semget(key, 2, IPC_CREAT | 0666);
    if (sem == -1) 
    {
        perror("semget");
        exit(1);
    }

    /* установка начальных значений семафоров */
    if((value = semctl(sem, 0, GETVAL, arg)) != 1)
    {
        arg.val = 1;
        semctl(sem, 0, SETVAL, arg);
        semctl(sem, 1, SETVAL, arg);
    };

    //подлкючение к разд памяти
    if((array = (int *)shmat(shmid, NULL, 0)) == (int *)(-1))      //получение адрессного пространства разд памяти
    {
        printf("Can't attach shared memory3\n");
        exit(-1);
    }    

    printf("создаю доч процесс\n");
    switch(pid = fork())
    {   
        //Дочерний процесс
        case 0:

        while(1)
        {
        /* ожидание освобождения семафора */
        if(semop(sem, &wait_0, 1) == -1) {    //ожидание 0(разрешения на чтение из файла)
            perror("semop:wait_0");    
            exit(EXIT_FAILURE);
        };

        array[1] = find_max(&(array[3]), array[0]);     //запись максимального
        array[2] = find_min(&(array[3]), array[0]);     //запись минимального

        /* освобождение семафора */
        if(semop(sem, &close_sem, 1) == -1) {    //закрытие себе доступа к чтению       сем0 = 1
            perror("semop:close 0");    
            exit(EXIT_FAILURE);
        };

        if(semop(sem, &open_sem1, 1) == -1) {    //отметка о прочитке                   сем 1 = 0
            perror("semop:open 1");    
            exit(EXIT_FAILURE);
        };

        }

        break;
        
        
        //родительский процесс
        default:

        srand(time(0));

        while(exit_flag)
        {

        array[0] = (rand() % RAND_NUM) + 1; //количество сгенерируемых случайных чисел  от 1 до RAND_MAX

        for(int i = 0; i < array[0]; i++)
        {
            array[i+3] = rand() % (MAX_VALUE + 1);  //записываем случайные числа
        }

        /* освобождение семафора0 */
        semop(sem, &open_sem, 1);                                       //сем0 = 0

        /* ожидание пока дочерний процесс обработает информацию */
        semop(sem, &wait_0_sem1, 1);

        /* захват семафора1 */
        semop(sem, &close_sem1, 1);                                     //сем1 = 1

        printf("max = %d, min = %d\n", array[1], array[2]);     //вывод маскимального минимального

        count++;       //увеличение счетчика
        }

         if(shmdt(array) < 0)                        //отключение от разд памяти
        { 
            printf("Can't detach shared memory\n");
            exit(-1);                               
        }
        kill(pid, SIGKILL);  //закрываем дочерний процесс

        wait(&rv);

        printf("Данные обработаны %d раз\n", count);

        /* удаление семафора */
        semctl(sem, 0, IPC_RMID);

        // освобождение сегмента разделяемой памяти
        if (shmctl(shmid, IPC_RMID, NULL) < 0)
        {
            perror("shmctl");
            exit(1);
        }

        break;
    }

    exit(0);
}