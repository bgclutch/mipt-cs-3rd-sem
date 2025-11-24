#define _POSIX_C_SOURCE 199309L /* Нужно для clock_gettime */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#define NUM_TESTS 10 /* Прогоню тест 10 раз, чтобы посчитать среднее */

void * any_func (void * arg)
{
    /* На всякий случай явно разрешаю отмену (хотя она и так включена) */
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    while (1)
    {
        /* sleep(10) — это "точка отмены". Поток прервется именно здесь. */
        sleep (10);
    }
    return NULL;
}

/* Функция для подсчета разницы времени (в секундах) */
double get_time_diff(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

int main (void)
{
    pthread_t thread;
    void * result;
    struct timespec start_time, end_time;
    double total_time = 0.0;

    printf("Запускаю тест (%d итераций)...\n", NUM_TESTS);

    for (int i = 0; i < NUM_TESTS; i++)
    {
        if (pthread_create (&thread, NULL, &any_func, NULL) != 0) {
            fprintf (stderr, "Ошибка создания потока\n");
            return 1;
        }

        /* Жду немного (10мс), чтобы поток точно успел запуститься и войти в sleep */
        usleep(10000);

        /* Засекаю время ПЕРЕД отправкой сигнала */
        clock_gettime(CLOCK_MONOTONIC, &start_time);

        /* Отправляю сигнал на отмену */
        pthread_cancel (thread);

        /* Жду, пока поток реально умрет и система освободит ресурсы.
           Join вернет управление только когда всё закончится. */
        if (!pthread_equal (pthread_self (), thread))
        {
            pthread_join (thread, &result);
        }

        /* Засекаю время ПОСЛЕ завершения */
        clock_gettime(CLOCK_MONOTONIC, &end_time);

        if (result != PTHREAD_CANCELED) {
            fprintf (stderr, "Странно, поток завершился не через cancel\n");
        }

        /* Добавляю разницу в копилку */
        double diff = get_time_diff(start_time, end_time);
        total_time += diff;
    }

    /* Считаю среднее */
    double avg_time = total_time / NUM_TESTS;

    printf("--------------------------------------\n");
    printf("Среднее время отмены потока: %.9f сек\n", avg_time);
    printf("--------------------------------------\n");

    return 0;
}