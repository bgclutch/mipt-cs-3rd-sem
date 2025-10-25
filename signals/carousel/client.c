#define _POSIX_C_SOURCE 200809L
#include "common.h"

int shm_id;
struct SharedData *data;
int my_slot_index = -1; /* Запоминаем, в какую ячейку мы записались */

/* Обработчик рабочего сигнала от Диспетчера */
void work_handler(int sig) {
    /* 1. Увеличиваем число в общей памяти */
    data->counter++;

    /* 2. Печатаем результат */
    printf("[Клиент %d] Сигнал получен! Новое число: %d\n", getpid(), data->counter);
}

/* Обработчик завершения (Ctrl+C) - корректный выход из Карусели */
void exit_handler(int sig) {
    if (my_slot_index != -1) {
        printf("\n[Клиент %d] Удаляю себя из списка (ячейка %d)...\n", getpid(), my_slot_index);
        data->pids[my_slot_index] = 0; // Освобождаем слот
    }
    shmdt(data);
    exit(0);
}

int main() {
    key_t key;

    /* 1. Получение ключа */
    if ((key = ftok(FTOK_FILE, PROJECT_ID)) == -1) {
        perror("ftok (Запущен ли диспетчер?)"); return 1;
    }

    /* 2. Получение доступа к памяти (без IPC_CREAT, она уже должна быть) */
    if ((shm_id = shmget(key, sizeof(struct SharedData), 0666)) < 0) {
        perror("shmget (Сначала запустите manager!)"); return 1;
    }

    /* 3. Присоединение */
    if ((data = (struct SharedData *)shmat(shm_id, NULL, 0)) == (void *)-1) {
        perror("shmat"); return 1;
    }

    /* 4. Регистрация: ищем свободный слот в массиве */
    pid_t my_pid = getpid();
    for (int i = 0; i < MAX_CLIENTS; i++) {
        /* Если ячейка равна 0, она свободна */
        if (data->pids[i] == 0) {
            data->pids[i] = my_pid;
            my_slot_index = i;
            printf("[Клиент %d] Успешная регистрация в слоте %d\n", my_pid, i);
            break;
        }
    }

    if (my_slot_index == -1) {
        fprintf(stderr, "Нет свободных мест в карусели!\n");
        shmdt(data);
        return 1;
    }

    /* 5. Настройка сигналов */
    struct sigaction act_work, act_exit;

    /* Сигнал для работы */
    sigemptyset(&act_work.sa_mask);
    act_work.sa_handler = work_handler;
    act_work.sa_flags = SA_RESTART; /* Чтобы read/pause перезапускались */
    sigaction(WORK_SIGNAL, &act_work, NULL);

    /* Сигнал для выхода (Ctrl+C) */
    sigemptyset(&act_exit.sa_mask);
    act_exit.sa_handler = exit_handler;
    act_exit.sa_flags = 0;
    sigaction(SIGINT, &act_exit, NULL);

    printf("[Клиент] Готов к работе. Жду сигналов...\n");

    /* Бесконечный цикл ожидания */
    while (1) {
        pause();
    }

    return 0;
}