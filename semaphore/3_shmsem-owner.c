#include <stdio.h>
#include <string.h>
#include <stdlib.h> /* Добавлено для exit */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define SHMEM_SIZE	4096
#define SH_MESSAGE	"Hello World!\n"

#define SEM_KEY		2007
#define SHM_KEY		2007

/* Объявление объединения для семафоров (требуется для semctl) */
union semnum
{
	int val;
	struct semid_ds * buf;
	unsigned short * array;
} sem_arg;


int main (void)
{
	int shm_id, sem_id;
	char * shm_buf;
	int shm_size;
	struct shmid_ds ds;
	struct sembuf sb[1];
	unsigned short sem_vals[1];

	/* 1. Создание разделяемой памяти
	   Параметры:
	   - SHM_KEY: Ключ.
	   - SHMEM_SIZE: Размер (4096 байт).
	   - Флаги: 0600 (rw-------) | IPC_CREAT (создать) | IPC_EXCL (ошибка, если уже есть).
	*/
	shm_id = shmget (SHM_KEY, SHMEM_SIZE,
			IPC_CREAT | IPC_EXCL | 0600);

	if (shm_id == -1)
	{
		fprintf (stderr, "shmget() error\n");
		return 1;
	}

	/* 2. Создание массива семафоров
	   Параметры:
	   - SEM_KEY: Ключ.
	   - 1: Количество семафоров в наборе.
	   - Флаги: Те же самые.
	*/
	sem_id = semget (SEM_KEY, 1,
			0600 | IPC_CREAT | IPC_EXCL);

	if (sem_id == -1)
	{
		fprintf (stderr, "semget() error\n");
		/* Если не удалось создать семафор, надо удалить созданную память,
		   иначе она "повиснет" в системе */
		shmctl(shm_id, IPC_RMID, NULL);
		return 1;
	}

	printf ("Semaphore ID: %d\n", sem_id);

	/* 3. Инициализация семафора */
	sem_vals[0] = 1; /* Устанавливаем начальное значение 1 (открыт) */
	sem_arg.array = sem_vals;

	/* semctl - управление семафором
	   Параметры:
	   - sem_id: ID набора.
	   - 0: Индекс (все семафоры).
	   - SETALL: Команда установить значения из массива.
	   - sem_arg: Объединение с массивом значений.
	*/
	if (semctl (sem_id, 0, SETALL, sem_arg) == -1)
	{
		fprintf (stderr, "semctl() error\n");
		return 1;
	}

	/* 4. Подключение разделяемой памяти (Attach)
	   Параметры:
	   - shm_id: ID памяти.
	   - NULL: Система сама выбирает адрес.
	   - 0: Флаги (чтение и запись).
	*/
	shm_buf = (char *) shmat (shm_id, NULL, 0);
	if (shm_buf == (char *) -1)
	{
		fprintf (stderr, "shmat() error\n");
		return 1;
	}

	/* Проверка размера сегмента */
	shmctl (shm_id, IPC_STAT, &ds);
	shm_size = ds.shm_segsz;
	if (shm_size < strlen (SH_MESSAGE))
	{
		fprintf (stderr, "error: segsize=%d\n", shm_size);
		return 1;
	}

	/* !!! ИСПРАВЛЕНИЕ ОШИБКИ 1 !!!
	   Было: strcpy (SH_MESSAGE, shm_buf); -> Попытка записи в константу (Crash)
	   Стало: Копируем строку "Hello World" В буфер памяти. */
	strcpy (shm_buf, SH_MESSAGE);

	printf ("ID: %d\n", shm_id);
	printf ("Message written to shared memory.\n");


	/* 5. Работа с семафором (Блокировка) */
	sb[0].sem_num = 0;       /* Индекс семафора (0 - первый) */
	sb[0].sem_flg = SEM_UNDO; /* Если процесс упадет, система отменит изменение */
	sb[0].sem_op = -1;       /* Операция P (Wait): уменьшить на 1 */

	printf("Locking semaphore (Wait)...\n");
	semop (sem_id, sb, 1);

	/* Здесь могла быть Критическая секция (работа с общим ресурсом) */

	/* !!! ИСПРАВЛЕНИЕ ОШИБКИ 2 !!!
	   Было: sb[0].sem_op = -1; -> Вторая попытка вычесть приведет к зависанию (Deadlock), т.к. значение уже 0.
	   Стало: sb[0].sem_op = 1; -> Операция V (Signal): увеличить на 1 (освободить ресурс). */
	sb[0].sem_op = 1;

	printf("Unlocking semaphore (Post)...\n");
	semop (sem_id, sb, 1);

	/* 6. Удаление ресурсов */
	/* Удаляем семафор (индекс 0, команда IPC_RMID) */
	semctl (sem_id, 0, IPC_RMID, sem_arg);

	/* Отсоединяем память */
	shmdt (shm_buf);

	/* Удаляем сегмент памяти */
	shmctl (shm_id, IPC_RMID, NULL);

	printf("Clean exit.\n");
	return 0;
}