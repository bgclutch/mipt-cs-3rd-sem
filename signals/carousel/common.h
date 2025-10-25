#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <string.h>

/* Имя файла для генерации ключа ftok */
#define FTOK_FILE "keyfile"
#define PROJECT_ID 10

/* Максимальное количество клиентов в карусели */
#define MAX_CLIENTS 10

/* Структура в разделяемой памяти */
struct SharedData {
    int counter;                // Общее число, которое будем увеличивать
    pid_t pids[MAX_CLIENTS];    // Массив PID всех запущенных клиентов
};

/* Сигнал, который диспетчер шлет клиентам */
#define WORK_SIGNAL SIGUSR1

#endif