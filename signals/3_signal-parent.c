/* !!! ВАЖНО: Эта строка должна быть самой первой! Она включает POSIX-функции. */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h> /* Лучше использовать sys/wait.h вместо wait.h */
#include <signal.h>
#include <stdlib.h>

/* Программа запускает дочерний процесс, который анализирует чиcло (год),
 * введённое как параметр и возвращает различные сигналы. */

sig_atomic_t sig_status = 0;

void handle_usr1 (int s_num)
{
	sig_status = 1;
}

void handle_usr2 (int s_num)
{
	sig_status = 2;
}

int main (int argc, char ** argv)
{
	struct sigaction act_usr1, act_usr2;

	/* Очищаем маски сигналов */
	sigemptyset (&act_usr1.sa_mask);
	sigemptyset (&act_usr2.sa_mask);

	act_usr1.sa_flags = 0;
	act_usr2.sa_flags = 0;

	act_usr1.sa_handler = &handle_usr1;
	act_usr2.sa_handler = &handle_usr2;

	if (sigaction (SIGUSR1, &act_usr1, NULL) == -1)
	{
		perror("sigaction (act_usr1)");
		return 1;
	}

	/* Исправлено: используем act_usr2 для второго сигнала */
	if (sigaction (SIGUSR2, &act_usr2, NULL) == -1)
	{
		perror("sigaction (act_usr2)");
		return 1;
	}

	if (argc < 2)
	{
		fprintf (stderr, "Usage: %s <year>\n", argv[0]);
		return 1;
	}

	if (!fork())
	{
		/* Дочерний процесс */
		/* Запускаем программу ./child, передаем ей год */
		execl ("./child", "child", argv[1], NULL);

		perror("execl error");
		return 1;
	}

	/* Родительский процесс */
	while (1)
	{
		/* Ждем сигнала (чтобы не грузить процессор) */
		pause();

		if (sig_status == 1)
		{
			printf ("Year %s: LEAP year (Високосный)\n", argv[1]);
			return 0;
		}

		if (sig_status == 2)
		{
			printf ("Year %s: NOT leap year (Обычный)\n", argv[1]);
			return 0;
		}
	}

	return 0;
}