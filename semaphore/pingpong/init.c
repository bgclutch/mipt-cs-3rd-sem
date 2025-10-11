/* init.c - Процесс 1: Создает ресурсы и начинает вычитание */
#include "common.h"

int main()
{
    key_t key;
    int shm_id, sem_id;
    int *shm_ptr;
    union semun arg;

    /* Структуры для операций с семафорами */
    struct sembuf wait_my_turn;
    struct sembuf signal_next_turn;

    /* 1. Генерация ключа */
    if ((key = ftok(FTOK_FILE, PROJECT_ID)) == -1) {
        perror("ftok (Проверьте, существует ли файл 'keyfile'!)");
        exit(1);
    }

    printf("[Proc 1] IPC Key generated: %d\n", key);

    /* 2. Создание ресурсов */
    /* Память */
    if ((shm_id = shmget(key, SHM_SIZE, IPC_CREAT | 0666)) < 0) {
        perror("shmget"); exit(1);
    }
    /* Семафоры (массив из 2 штук) */
    if ((sem_id = semget(key, 2, IPC_CREAT | 0666)) < 0) {
        perror("semget"); exit(1);
    }

    /* 3. Подключение памяти */
    if ((shm_ptr = (int *)shmat(shm_id, NULL, 0)) == (void *)-1) {
        perror("shmat"); exit(1);
    }

    /* 4. Настройка начальных значений */
    printf("[Proc 1] Введите начальное число: ");
    scanf("%d", shm_ptr); // Записываем число прямо в разделяемую память

    /* Настройка семафоров:
       Sem[0] - очередь Процесса 1 (init)
       Sem[1] - очередь Процесса 2 (worker)
    */

    /* Разрешаем Процессу 1 работать сразу (значение 1) */
    arg.val = 1;
    semctl(sem_id, 0, SETVAL, arg);

    /* Запрещаем Процессу 2 работать пока что (значение 0) */
    arg.val = 0;
    semctl(sem_id, 1, SETVAL, arg);

    /* Настройка операций */
    /* Ждать разрешения для себя (Sem 0) */
    wait_my_turn.sem_num = 0; wait_my_turn.sem_op = -1; wait_my_turn.sem_flg = 0;
    /* Дать разрешение другому (Sem 1) */
    signal_next_turn.sem_num = 1; signal_next_turn.sem_op = 1; signal_next_turn.sem_flg = 0;

    printf("[Proc 1] Start logic loop...\n");

    /* 5. Основной цикл */
    while (1) {
        /* Ждем своей очереди (P-операция на Sem 0) */
        if (semop(sem_id, &wait_my_turn, 1) < 0) break;

        /* Работаем с числом */
        int val = *shm_ptr;
        printf("[Proc 1] Current: %d -> New: %d\n", val, val - 1);
        *shm_ptr = val - 1;

        sleep(1); /* Задержка для наглядности */

        /* Если достигли 0 или меньше - выход */
        if (*shm_ptr <= 0) {
            /* Важно! Нужно пнуть второго, чтобы он проснулся и тоже вышел */
            semop(sem_id, &signal_next_turn, 1);
            break;
        }

        /* Передаем ход (V-операция на Sem 1) */
        semop(sem_id, &signal_next_turn, 1);
    }

    /* 6. Удаление ресурсов (делает только init) */
    printf("[Proc 1] Finished. Cleaning up...\n");
    shmdt(shm_ptr);
    shmctl(shm_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID);

    return 0;
}