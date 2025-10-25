#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h> /* Добавлено для функции getpid() */

/* Программа посылающая себе сигнал на уничтожение.*/
/* Дописать игнорирование данного сигнала.*/
int main (void)
{
    /* 1. Получаем ID текущего процесса (себя) */
	pid_t dpid = getpid();

    /* 2. Устанавливаем игнорирование сигнала SIGABRT.
       SIG_IGN - это макрос (Signal Ignore).
       Эту строчку нужно писать ДО того, как сигнал будет отправлен. */
    signal(SIGABRT, SIG_IGN);

    printf("Мой PID: %d. Посылаю себе SIGABRT...\n", dpid);

	if (kill (dpid, SIGABRT) == -1) {
		fprintf (stderr, "Cannot send signal\n");
		return 1;
	}

    /* Если программа дошла до сюда, значит сигнал был успешно проигнорирован */
    printf("Жив! Сигнал был проигнорирован.\n");

	return 0;
}