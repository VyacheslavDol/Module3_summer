#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h> 
#include <time.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ARRAU_COUNT 5
#define MAX_VALUE 10000
#define SEM_0 "/sem_name_0"
#define SEM_1 "/sem_name_1"
#define SH_MEM "/shared_mem"

int exit_flag = 1; //флаг выхода в случае SIGINT перевод в 0

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

    int value;
    sem_t *sem_0;
    sem_t *sem_1;
    pid_t pid;
    int rv;
    int count = 0;       //счетчик операций
    int *array;          // Указатель на разделяемую память 
    int shmid;           // IPC дескриптор для области разделяемой памяти

    /* изменение реакции на сигнал SIGINT */
    signal(SIGINT, react_sigint);

    //создание области разделяемой памяти
    if((shmid = shm_open(SH_MEM, O_CREAT|O_RDWR, 0644)) == -1)
    {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    //создание семафоров с начальными знаечниями 1
    sem_0 = sem_open(SEM_0, O_CREAT, 0644, 1);
    sem_1 = sem_open(SEM_1, O_CREAT, 0664, 1);

    setvalue_sem(sem_1, 1);
    setvalue_sem(sem_0, 1);

    if(sem_0 == SEM_FAILED || sem_1 == SEM_FAILED)
    {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    // настраиваем размер разделяемой памяти
    if (ftruncate(shmid, ARRAU_COUNT * sizeof(int)) == -1) {
        perror("ftruncate");
        exit(1);
    }

    //отображение разделяемой памяти в адресное пространство
    if((array = (int*)mmap(0, ARRAU_COUNT * sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED, shmid, 0)) == MAP_FAILED)      //получение адрессного пространства разд памяти
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }    

    switch(pid = fork())
    {   
        //Дочерний процесс
        case 0:

        while(1)
        {
            
        while(getval(sem_0) != 0);      //ожидание 0
        {
            /* waiting 0 */
        };
        
        array[1] = find_max(&(array[3]), array[0]);     //запись максимального
        array[2] = find_min(&(array[3]), array[0]);     //запись минимального

        /* освобождение семафора */
        if(sem_post(sem_0) == -1)               //закрытие себе доступа к чтению       сем0 = 1
        {
            perror("sem_0:sem_post");
            exit(EXIT_FAILURE);
        }

        if(sem_wait(sem_1) == -1)                //отметка о прочитке                   сем 1 = 0
        {
            perror("sem_1:sem_wait");
            exit(EXIT_FAILURE);
        };

        }
        break;
        
        //родительский процесс
        default:

        srand(time(0));

        while(exit_flag)
    {

        array[0] = (rand() % ARRAU_COUNT) + 1; //количество сгенерируемых случайных чисел  от 1 до RAND_MAX

        for(int i = 0; i < array[0]; i++)
        {
            array[i+3] = rand() % (MAX_VALUE + 1);  //записываем случайные числа
        }

        /* освобождение семафора0 */
        
        if(sem_wait(sem_0) == -1)                //открытие доступа         сем0 = 0
        {
            perror("sem_0:sem_wait");
            exit(EXIT_FAILURE);
        };
        
        /* ожидание пока дочерний процесс обработает информацию */
        while(getval(sem_1) != 0)
        {
            /* waiting 0 sem_1 */
        }

        /* захват семафора1 */
        if(sem_post(sem_1) == -1)               //сброс отметки о прочитке          сeм1 = 1
        {
            perror("sem_1:sem_post");
            exit(EXIT_FAILURE);
        }

        printf("max = %d, min = %d\n", array[1], array[2]);     //вывод маскимального минимального

        count++;       //увеличение счетчика
    }
        
        kill(pid, SIGKILL);  //закрываем дочерний процесс

        wait(&rv);

        printf("Данные обработаны %d раз\n", count);

        /* удаление семафора */
        if(sem_unlink(SEM_0) == -1 || sem_unlink(SEM_1) == -1)
        {
            perror("sem_unlink");
            exit(EXIT_FAILURE);
        }

        // освобождение сегмента разделяемой памяти
        if(shm_unlink(SH_MEM) == -1)
        {
            perror("shm_unlink");
            exit(EXIT_FAILURE);
        }

        break;
    }

    exit(0);
}