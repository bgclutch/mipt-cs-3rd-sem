/*
  Программа: Совместное вычитание числа (Ping-Pong через Семафоры)

  Компиляция: gcc subtractor.c -o sub

  Запуск:
  Терминал 1: ./sub 1   (Создает ресурсы, вводит число, начинает первым)
  Терминал 2: ./sub 2   (Подключается, работает вторым)
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>

#define SHM_SIZE 1024
/* Имя файла для генерации ключа (должен существовать) */
#define FTOK_FILE "subtractor.c"

/* Структура для инициализации семафора (требуется для semctl) */
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int main(int argc, char *argv[])
{
    /* Проверка аргументов */
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <1 or 2>\nExample: ./sub 1\n", argv[0]);
        exit(1);
    }

    int proc_num = atoi(argv[1]);
    if (proc_num != 1 && proc_num != 2) {
        fprintf(stderr, "Please specify process number: 1 or 2\n");
        exit(1);
    }

    key_t key;
    int shm_id, sem_id;
    int *shm_ptr;

    /* 1. Генерация ключа */
    if ((key = ftok(FTOK_FILE, 1)) == -1) {
        perror("ftok error (make sure file exists)");
        exit(1);
    }

    /* 2. Подготовка ресурсов (Различается для Процесса 1 и 2) */

    if (proc_num == 1) {
        /* --- ПРОЦЕСС 1: Инициализатор --- */
        printf("[Proc 1] Creating shared resources...\n");

        /* Создаем память */
        shm_id = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
        if (shm_id < 0) { perror("shmget"); exit(1); }

        /* Создаем набор из 2 семафоров */
        sem_id = semget(key, 2, IPC_CREAT | 0666);
        if (sem_id < 0) { perror("semget"); exit(1); }

        /* Присоединяем память */
        shm_ptr = (int *)shmat(shm_id, NULL, 0);
        if (shm_ptr == (void *)-1) { perror("shmat"); exit(1); }

        /* Ввод начального числа */
        printf("[Proc 1] Enter initial number: ");
        int input;
        scanf("%d", &input);
        *shm_ptr = input;

        /* Инициализация семафоров */
        union semun arg;

        /* Sem[0] = 1 (Разрешено Процессу 1) */
        arg.val = 1;
        semctl(sem_id, 0, SETVAL, arg);

        /* Sem[1] = 0 (Запрещено Процессу 2) */
        arg.val = 0;
        semctl(sem_id, 1, SETVAL, arg);

        printf("[Proc 1] Ready. Waiting for cycle...\n");

    } else {
        /* --- ПРОЦЕСС 2: Присоединяющийся --- */
        printf("[Proc 2] Connecting to resources...\n");

        /* Пытаемся найти память и семафоры. Цикл нужен, если Proc 1 еще не запустился. */
        while ((shm_id = shmget(key, SHM_SIZE, 0666)) < 0) {
            printf("[Proc 2] Waiting for Proc 1 to start...\n");
            sleep(1);
        }

        sem_id = semget(key, 2, 0666);
        if (sem_id < 0) { perror("semget"); exit(1); }

        shm_ptr = (int *)shmat(shm_id, NULL, 0);
        if (shm_ptr == (void *)-1) { perror("shmat"); exit(1); }
    }

    /* 3. Определение операций для семафоров */

    struct sembuf wait_my_turn;
    struct sembuf signal_next_turn;

    if (proc_num == 1) {
        /* Процесс 1 ждет Sem[0], открывает Sem[1] */
        wait_my_turn.sem_num = 0;     // Индекс 0
        wait_my_turn.sem_op  = -1;    // Wait (P)
        wait_my_turn.sem_flg = 0;

        signal_next_turn.sem_num = 1; // Индекс 1
        signal_next_turn.sem_op  = 1; // Signal (V)
        signal_next_turn.sem_flg = 0;
    } else {
        /* Процесс 2 ждет Sem[1], открывает Sem[0] */
        wait_my_turn.sem_num = 1;     // Индекс 1
        wait_my_turn.sem_op  = -1;    // Wait (P)
        wait_my_turn.sem_flg = 0;

        signal_next_turn.sem_num = 0; // Индекс 0
        signal_next_turn.sem_op  = 1; // Signal (V)
        signal_next_turn.sem_flg = 0;
    }

    /* 4. Основной цикл работы */
    while (1) {
        /* Ждем своей очереди */
        if (semop(sem_id, &wait_my_turn, 1) < 0) {
            perror("semop wait");
            break;
        }

        /* Читаем число */
        int current_val = *shm_ptr;

        /* Условие выхода: если число <= 0 */
        if (current_val <= 0) {
            /* Передаем ход другому, чтобы он тоже мог выйти из цикла корректно,
               или просто завершаем работу. */
            semop(sem_id, &signal_next_turn, 1);
            break;
        }

        /* Основная работа */
        printf("Process %d: %d -> %d\n", proc_num, current_val, current_val - 1);
        *shm_ptr = current_val - 1;

        /* Задержка для наглядности */
        sleep(1);

        /* Передаем ход следующему процессу */
        if (semop(sem_id, &signal_next_turn, 1) < 0) {
            perror("semop signal");
            break;
        }
    }

    /* 5. Завершение работы */
    shmdt(shm_ptr);

    if (proc_num == 1) {
        /* Процесс 1 удаляет ресурсы */
        printf("[Proc 1] Deleting shared memory and semaphores...\n");
        shmctl(shm_id, IPC_RMID, NULL);
        semctl(sem_id, 0, IPC_RMID);
    } else {
        printf("[Proc 2] Exiting...\n");
    }

    return 0;
}