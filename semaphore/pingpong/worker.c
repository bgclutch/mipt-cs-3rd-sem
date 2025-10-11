/* worker.c - Процесс 2: Подключается и продолжает вычитание */
#include "common.h"

int main()
{
    key_t key;
    int shm_id, sem_id;
    int *shm_ptr;

    struct sembuf wait_my_turn;
    struct sembuf signal_next_turn;

    /* 1. Генерация того же ключа */
    if ((key = ftok(FTOK_FILE, PROJECT_ID)) == -1) {
        perror("ftok (Файла 'keyfile' нет?)");
        exit(1);
    }

    /* 2. Поиск ресурсов (Без IPC_CREAT) */
    /* Пытаемся подключиться, пока Init не создаст память */
    printf("[Proc 2] Waiting for resources...\n");
    while ((shm_id = shmget(key, SHM_SIZE, 0666)) < 0) {
        sleep(1);
    }

    if ((sem_id = semget(key, 2, 0666)) < 0) {
        perror("semget failed"); exit(1);
    }

    /* 3. Подключение памяти */
    if ((shm_ptr = (int *)shmat(shm_id, NULL, 0)) == (void *)-1) {
        perror("shmat"); exit(1);
    }

    printf("[Proc 2] Connected! Ready to work.\n");

    /* Настройка операций (зеркально относительно init) */
    /* Ждать разрешения для себя (Sem 1) */
    wait_my_turn.sem_num = 1; wait_my_turn.sem_op = -1; wait_my_turn.sem_flg = 0;
    /* Дать разрешение другому (Sem 0) */
    signal_next_turn.sem_num = 0; signal_next_turn.sem_op = 1; signal_next_turn.sem_flg = 0;

    /* 4. Основной цикл */
    while (1) {
        /* Ждем своей очереди (P-операция на Sem 1) */
        /* Если semop вернет ошибку (например, init удалил семафоры), выходим */
        if (semop(sem_id, &wait_my_turn, 1) < 0) {
            printf("[Proc 2] Semaphore removed. Exiting.\n");
            break;
        }

        /* Проверяем условие выхода ДО работы */
        if (*shm_ptr <= 0) {
            /* Возвращаем ход (на всякий случай) и выходим */
            semop(sem_id, &signal_next_turn, 1);
            break;
        }

        int val = *shm_ptr;
        printf("[Proc 2] Current: %d -> New: %d\n", val, val - 1);
        *shm_ptr = val - 1;

        sleep(1);

        /* Передаем ход (V-операция на Sem 0) */
        semop(sem_id, &signal_next_turn, 1);
    }

    /* Отключаемся (удалять память НЕ надо, это делает init) */
    shmdt(shm_ptr);
    printf("[Proc 2] Finished.\n");

    return 0;
}