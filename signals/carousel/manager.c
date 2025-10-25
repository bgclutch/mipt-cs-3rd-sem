#define _POSIX_C_SOURCE 200809L
#include "common.h"

int shm_id;
struct SharedData *data;

/* Обработчик Ctrl+C для очистки памяти перед выходом */
void cleanup_handler(int sig) {
    printf("\n[Диспетчер] Завершение работы. Удаление сегмента памяти.\n");
    shmdt(data);
    shmctl(shm_id, IPC_RMID, NULL);
    exit(0);
}

int main() {
    key_t key;

    /* 1. Генерация ключа */
    /* ВАЖНО: Файл "keyfile" должен существовать: touch keyfile */
    if ((key = ftok(FTOK_FILE, PROJECT_ID)) == -1) {
        perror("ftok"); return 1;
    }

    /* 2. Создание разделяемой памяти */
    if ((shm_id = shmget(key, sizeof(struct SharedData), IPC_CREAT | 0666)) < 0) {
        perror("shmget"); return 1;
    }

    /* 3. Присоединение */
    if ((data = (struct SharedData *)shmat(shm_id, NULL, 0)) == (void *)-1) {
        perror("shmat"); return 1;
    }

    /* Инициализация данных */
    data->counter = 0;
    /* Заполняем массив PID нулями (0 означает пустое место) */
    memset(data->pids, 0, sizeof(data->pids));

    /* Настройка очистки по Ctrl+C */
    signal(SIGINT, cleanup_handler);

    printf("--- КОМПЛЕКС КАРУСЕЛЬ: ДИСПЕТЧЕР ---\n");
    printf("ID памяти: %d\n", shm_id);
    printf("Нажмите ENTER, чтобы отправить сигнал всем модулям.\n");
    printf("Нажмите Ctrl+C для выхода и удаления памяти.\n");

    while (1) {
        /* Ждем нажатия Enter */
        getchar();

        printf("[Диспетчер] Рассылка сигналов...\n");
        int active_count = 0;

        /* Проходим по массиву PID и шлем сигнал тем, кто зарегистрировался */
        for (int i = 0; i < MAX_CLIENTS; i++) {
            pid_t client_pid = data->pids[i];

            if (client_pid != 0) {
                /* Проверяем, жив ли процесс (kill с сигналом 0 просто проверяет наличие) */
                if (kill(client_pid, 0) == 0) {
                    kill(client_pid, WORK_SIGNAL);
                    printf(" -> Сигнал отправлен клиенту PID %d\n", client_pid);
                    active_count++;
                    /* Небольшая пауза, чтобы не было гонки данных при записи в counter */
                    usleep(10000);
                } else {
                    /* Если процесс умер "неаккуратно", удаляем его ID */
                    printf(" -> Клиент PID %d не отвечает (удален из списка)\n", client_pid);
                    data->pids[i] = 0;
                }
            }
        }

        if (active_count == 0) {
            printf("[Внимание] Нет активных клиентов. Запустите ./client в других терминалах.\n");
        } else {
            printf("[Диспетчер] Текущее значение числа: %d\n", data->counter);
        }
        printf("----------------------------------------\n");
    }

    return 0;
}