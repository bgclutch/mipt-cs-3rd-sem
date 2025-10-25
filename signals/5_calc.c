#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

/* Глобальные переменные нужны, чтобы их видел обработчик сигнала.
   volatile говорит компилятору, что эти переменные могут измениться
   в любой момент (извне основного потока выполнения). */
volatile double current_fact = 1.0;
volatile int current_n = 1;
time_t start_time;

/* Обработчик сигнала */
void status_handler(int sig)
{
    time_t current_time = time(NULL);
    double elapsed = difftime(current_time, start_time);

    printf("\n--- [SIGNAL RECEIVED] ---\n");
    printf("Прошло времени: %.0f сек.\n", elapsed);
    printf("Текущий шаг: %d!\n", current_n);
    printf("Текущий результат: %g\n", current_fact);
    printf("-------------------------\n");

    /* Примечание: использование printf внутри обработчика сигнала
       технически небезопасно (not async-signal-safe), но в учебных
       задачах это стандартная практика. */
}

int main(void)
{
    struct sigaction act;

    /* Запоминаем время начала */
    start_time = time(NULL);

    /* Настраиваем обработчик на сигнал SIGUSR1 */
    sigemptyset(&act.sa_mask);
    act.sa_handler = &status_handler;
    act.sa_flags = 0; // Можно добавить SA_RESTART, если нужно

    if (sigaction(SIGUSR1, &act, NULL) == -1) {
        perror("sigaction error");
        return 1;
    }

    printf("Программа запущена. PID процесса: %d\n", getpid());
    printf("Чтобы узнать статус, введите в другом терминале:\n");
    printf("kill -USR1 %d\n\n", getpid());
    printf("Начинаю считать факториал...\n");

    /* Бесконечный цикл счета */
    /* Мы используем usleep, чтобы замедлить процесс, иначе факториал
       улетит в бесконечность (inf) за доли секунды */
    for (current_n = 1; ; current_n++) {
        current_fact *= current_n;

        /* Задержка 0.5 секунды (500000 микросекунд) */
        usleep(500000);

        /* Если число стало слишком большим (infinity), можно сбросить,
           чтобы было интереснее наблюдать */
        if (current_fact > 1e300) {
             printf("\n[Достигнут предел double, сброс]\n");
             current_fact = 1.0;
             current_n = 1;
        }
    }

    return 0;
}