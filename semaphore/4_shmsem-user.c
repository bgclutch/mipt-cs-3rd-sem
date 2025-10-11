#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <sys/sem.h>
#include <stdlib.h> /* Добавлено для exit() */

#define SEM_KEY 2007
#define SHM_KEY 2007
/* Написать комментарии, отладить работу */

int main (int argc, char ** argv)
{
	int shm_id, sem_id;
	char * shm_buf;
	struct sembuf sb[1];

	/* 1. Получение доступа к существующей разделяемой памяти
	   Параметры:
	   - SHM_KEY: Ключ (2007). Должен совпадать с ключом сервера.
	   - 0: Размер. При открытии существующего сегмента лучше указывать 0
	        (означает "не проверять размер"). В оригинале было 1.
	   - 0600: Права доступа (чтение/запись владельца). Флага IPC_CREAT нет,
	        значит, если памяти нет, будет ошибка.
	*/
	shm_id = shmget (SHM_KEY, 0, 0600);
	if (shm_id == -1)
	{
		fprintf (stderr, "shmget() error. (Запустили ли вы сервер?)\n");
		return 1;
	}

	/* 2. Получение доступа к существующему семафору
	   Параметры:
	   - SEM_KEY: Ключ (2007).
	   - 1: Количество семафоров в наборе (нужно указывать даже при поиске).
	   - 0600: Права доступа.
	*/
	sem_id = semget (SEM_KEY, 1, 0600);
	if (sem_id == -1)
	{
		fprintf (stderr, "semget() error\n");
		return 1;
	}

	/* 3. Подключение (Attach) памяти к процессу
	   Параметры:
	   - shm_id: ID сегмента.
	   - 0 (NULL): Система выбирает адрес сама.
	   - 0: Флаги (чтение и запись).
	*/
	/* !!! ИСПРАВЛЕНИЕ ОШИБКИ: приведение типа (char *), так как shm_buf это char* */
	shm_buf = (char *) shmat (shm_id, 0, 0);
	if (shm_buf == (char *) -1)
	{
		fprintf (stderr, "shmat() error\n");
		return 1;
	}

	/* 4. Чтение данных (просто читаем строку из памяти по указателю) */
	printf ("Message from server: %s\n", shm_buf);

	/* 5. Управление семафором (Signal / V-операция) */
	sb[0].sem_num = 0;       /* Номер семафора (первый) */
	sb[0].sem_flg = SEM_UNDO; /* Отменить действие при падении процесса */

	/* sem_op = 1: Увеличиваем значение семафора на 1.
	   Если сервер ждал (выполнял -1), то эта операция "разбудит" его. */
	sb[0].sem_op = 1;

	printf("Unlocking server (sem_op = 1)...\n");
	semop (sem_id, sb, 1);

	/* 6. Отсоединение памяти (Detach)
	   Мы только отключаемся. Удалять память (IPC_RMID) должен сервер,
	   когда проснется. */
	shmdt (shm_buf);

	return 0;
}