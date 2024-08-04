#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

#define FNAME "numbers"
#define CH_PR 5
#define NUM_ACCESS 2

union semun {
int val; /* значение для SETVAL */
struct semid_ds *buf; /* буферы для IPC_STAT, IPC_SET */
unsigned short *array; /* массивы для GETALL, SETALL */
/* часть, особенная для Linux: */
struct seminfo *__buf; /* буфер для IPC_INFO */
};

int parent_func(int sem, int* fd, int count){     //чтение случайных чисел из канала , отрпавка их в файл и регулировние доступом

    struct sembuf open_access = {1, -1, 0};    //уменьшение значения 1 семофора на -1
    struct sembuf close_access = {1, 1, 0};    //увеличение значения 1 семофора на +1
    struct sembuf open_count = {2, -1, 0};     //уменьшение значения 2 семаофра на -1

    close(fd[1]); //закрытие канала на запись
    int number, fd_write;
    union semun arg;
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
    arg.val = CH_PR + 1;
    semctl(sem, 2, SETVAL, arg);

    //открытие доступа  (1ый семафор значение = 0)
    semop(sem, &open_access, 1);
    printf("Parent Доступ открыт\n");

    //ожидание пока все доч процессы чтения не прочтут файлб (пока sem 2 не будет равен 1)
    while(semctl(sem, 2, GETVAL) != 1)
    {
        /* ожидание всех процессов */
    };

    //закрытие доступа (1ый в 1)
    semop(sem, &close_access, 1);
    printf("Parent Доступ закрыт\n");
    //перевод sem 2 в знаечние 0
    semop(sem, &open_count, 1);
}
    close(fd_write);    //закрытие файла
return 0;
}

int child_gen2(int sem, int* fd, int count)
{      
    int read_number;                        
    struct sembuf connect = {0, -1, 0};     //присоединеие к прочтению (захват возмоджности на чтение)
    struct sembuf disconnect = {0, 1, 0};   //отсоединение от чтения (освобождения возможности прочитать)
    struct sembuf wait_all = {2, 0, 0};     //ожидание пока прочтут все(и род. процесс пропустит дальше)
    struct sembuf wait_0 = {1, 0, 0};       //"ожидание нуля" для 1 семафора
    struct sembuf alrd_read = {2, -1, 0};   //отметка о прочитке

    //открытие файла на чтение
    int fd_read = open(FNAME, O_RDONLY | O_CREAT, 0644);        
    if (fd_read == -1) {
    perror("Ошибка открытия файла для чтения");
    return 1;
    }

    for(int i = 0; i < count; i++)
    {
    //ожидание открытия доступа на чтение
    semop(sem, &wait_0, 1);

    while(semctl(sem, 0, GETVAL) == 0)
    {
        /* ожидание свободного места на чтение файла */
    };

    //подключение к чтению файла
    semop(sem, &connect, 1);

    //чтение числа из файла
    if(read(fd_read, &read_number, sizeof(read_number)) == -1){
        printf("Прочитать информацию из файла не удалось");
        return 1;
    };
    printf("Child: Я прочитал из файла %d\n", read_number);
    
    //отключение от чтения
    semop(sem, &disconnect, 1);
    
    //"отметка" о прочтении
    semop(sem, &alrd_read, 1);

    //ожидание всех процессов чтения
    semop(sem, &wait_all, 1);
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
    key_t key;
    int sem, fd[2], number, fd_read, fd_write, read_number, value;
    union semun arg;
    
    srand(time(NULL));

    //создание канала
    if(pipe(fd) != 0){  
        printf("Error pipe");
        exit(1);
    };

    //Cоздание уникального ключа
    key = ftok("/home", 'w');       
    if(key == -1){
        printf("Ошибка создания ключа подключения\n");
        exit(1);}

    // Создание 3 семафоров
    sem = semget(key, 3, IPC_CREAT | 0666);         // 0 - кол-во процессов которые читают файл в данный момент времени; 1 - доступ (0/1); 2 - кол-во процессов прочитавших файл
    if (sem == -1)                             
    {
        perror("semget");
        exit(1);
    }

    //выставление начальных значений семафоров
    arg.val = 1;
    semctl(sem, 1, SETVAL, arg);        

    arg.val = CH_PR + 1;
    semctl(sem, 2, SETVAL, arg);

    arg.val = NUM_ACCESS;
    semctl(sem, 0, SETVAL, arg);

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
            child_gen2(sem, fd, atoi(argv[1]));
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
            parent_func(sem, fd, atoi(argv[1]));
            
            //ожидание закрытия всез доч процессов
            for(int i = CH_PR; i > 0; i--){
                wait(0);
            };
                    semctl(sem, 0, IPC_RMID);   //удаление семафора
        printf("Семафор удален\n");
        exit(EXIT_SUCCESS);
        }
    }
}
