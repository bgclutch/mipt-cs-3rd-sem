#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>

/*
   Модифицировал программу.
   Нужно компилировать с ключом -lm (для math.h):
   gcc trig.c -o trig -pthread -lm
*/

/* Функция 1-го потока: считает квадрат СИНУСА */
void * calc_sin_sq (void * arg)
{
    /* Получаю значение угла, разыменовывая указатель */
    double x = *(double *) arg;

    /* Считаю синус в квадрате */
    double result = sin(x) * sin(x);

    /* Вывожу промежуточный результат, чтобы видеть, что поток работает */
    printf("Поток 1 (sin): угол %.2f, результат %f\n", x, result);

    /* Выделяю память в куче (heap), чтобы вернуть результат в main.
       Использовать адрес локальной переменной нельзя — она удалится при выходе. */
    double * ret_val = (double *) malloc(sizeof(double));
    if (ret_val == NULL) {
        perror("Ошибка выделения памяти");
        pthread_exit(NULL);
    }
    *ret_val = result;

    /* Возвращаю указатель на результат (привожу к void*) */
    return (void *) ret_val;
}

/* Функция 2-го потока: считает квадрат КОСИНУСА */
void * calc_cos_sq (void * arg)
{
    double x = *(double *) arg;
    double result = cos(x) * cos(x);

    printf("Поток 2 (cos): угол %.2f, результат %f\n", x, result);

    /* Тоже выделяю память под результат */
    double * ret_val = (double *) malloc(sizeof(double));
    if (ret_val == NULL) {
        perror("Ошибка выделения памяти");
        pthread_exit(NULL);
    }
    *ret_val = result;

    return (void *) ret_val;
}

int main (void)
{
    pthread_t thread1, thread2;
    double angle;

    /* Указатели, куда pthread_join запишет адреса результатов потоков */
    void * res1_ptr;
    void * res2_ptr;

    printf("Введи угол (в радианах) для проверки: ");
    if (scanf("%lf", &angle) != 1) {
        fprintf(stderr, "Ошибка ввода числа\n");
        return 1;
    }

    /* Запускаю первый поток для синуса, передаю адрес переменной angle */
    if (pthread_create (&thread1, NULL, &calc_sin_sq, &angle) != 0) {
        fprintf (stderr, "Ошибка создания потока 1\n");
        return 1;
    }

    /* Запускаю второй поток для косинуса */
    if (pthread_create (&thread2, NULL, &calc_cos_sq, &angle) != 0) {
        fprintf (stderr, "Ошибка создания потока 2\n");
        return 1;
    }

    /* Жду завершения потоков и забираю результаты */
    pthread_join(thread1, &res1_ptr);
    pthread_join(thread2, &res2_ptr);

    /* Если оба потока вернули нормальные указатели, считаю сумму */
    if (res1_ptr && res2_ptr) {
        /* Привожу void* обратно к double* и беру значение */
        double val1 = *(double *)res1_ptr;
        double val2 = *(double *)res2_ptr;

        /* Главное доказательство: сумма должна быть равна 1 */
        double sum = val1 + val2;

        printf("---------------------------\n");
        printf("Итог в main: %f + %f = %f\n", val1, val2, sum);

        /* Чищу память, которую выделял в потоках */
        free(res1_ptr);
        free(res2_ptr);
    } else {
        fprintf(stderr, "Ошибка: потоки не вернули данные\n");
    }

    return 0;
}