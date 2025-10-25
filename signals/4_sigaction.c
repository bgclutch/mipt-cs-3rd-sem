#define _POSIX_C_SOURCE 200809L /* Для корректной работы struct sigaction */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> /* Для pause() */

/* Флаг, который меняется в обработчике */
sig_atomic_t sig_occured = 0;

void sig_handler (int snum)
{
	sig_occured = 1;
}

int main (void)
{
	struct sigaction act;
    int count = 0; /* Счетчик полученных сигналов */

	sigemptyset (&act.sa_mask);
	act.sa_handler = &sig_handler;
	act.sa_flags = 0;

	/* Устанавливаем наш обработчик на сигнал SIGINT (Ctrl+C) */
	if (sigaction (SIGINT, &act, NULL) == -1)
	{
		perror("sigaction() error");
		return 1;
	}

	printf("Программа запущена. Нажмите Ctrl+C 5 раз для восстановления стандартного поведения.\n");

	while (1)
	{
		/* pause() переводит процесс в сон до прихода сигнала.
		   Это предотвращает 100% загрузку процессора пустым циклом. */
		pause();

		if (sig_occured)
		{
            count++;
			fprintf(stderr, "Пойман сигнал SIGINT! (%d из 5)\n", count);
			sig_occured = 0;

            /* Если поймали 5 раз */
            if (count >= 5)
            {
                printf("Лимит исчерпан. Восстанавливаю стандартное поведение (SIG_DFL).\n");

                /* Меняем обработчик на SIG_DFL (Default - стандартный) */
                act.sa_handler = SIG_DFL;

                /* Применяем изменения */
                if (sigaction(SIGINT, &act, NULL) == -1) {
                    perror("Не удалось восстановить обработчик");
                    return 1;
                }

                printf("Теперь следующее нажатие Ctrl+C убьет программу.\n");
            }
		}
	}

	return 0;
}