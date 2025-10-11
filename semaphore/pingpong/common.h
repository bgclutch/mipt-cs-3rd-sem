#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>

#define FTOK_FILE "keyfile"
#define PROJECT_ID 1

/* Размер разделяемой памяти (нам нужно всего 4 байта под int, но берем с запасом) */
#define SHM_SIZE 1024

/* Объединение для семафоров (стандартная конструкция для semctl) */
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

#endif