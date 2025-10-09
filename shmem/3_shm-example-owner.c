/* Проверить совместную работу с 4.
   Написать комментарии, В ТОМ ЧИСЛЕ К ПАРАМЕТРАМ!*/
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>   /* Добавлено для корректной работы флагов IPC_* */
#include <sys/types.h> /* Добавлено для системных типов */

#define SHMEM_SIZE	4096
#define SH_MESSAGE	"Poglad Kota!\n"

int main (void)
{
  int shm_id;
  char * shm_buf;
  int shm_size;
  struct shmid_ds ds;

  /* shmget - создание сегмента разделяемой памяти
     Параметры:
     1. IPC_PRIVATE - ключ (создает новый уникальный сегмент, доступ только по ID)
     2. SHMEM_SIZE - размер сегмента в байтах (4096)
     3. Флаги: IPC_CREAT (создать) | IPC_EXCL (ошибка если есть) | 0600 (права rw-------)
  */
  shm_id = shmget (IPC_PRIVATE,
                   SHMEM_SIZE,
  		           IPC_CREAT | IPC_EXCL | 0600);

  if (shm_id == -1)
  {
    fprintf (stderr, "shmget() error\n");
    return 1;
  }

  /* shmat - подключение сегмента к адресному пространству
     Параметры:
     1. shm_id - ID сегмента, полученный от shmget (!!! ИСПРАВЛЕНА ОШИБКА: было shm_buf)
     2. NULL - система сама выбирает адрес выравнивания
     3. 0 - флаги (0 означает чтение и запись)
  */
  shm_buf = (char *) shmat (shm_id,
                            NULL,
                            0);
  if (shm_buf == (char *) -1) /* Исправлено сравнение с ошибкой */
  {
    fprintf (stderr, "shmat() error\n");
  	return 1;
  }

  /* shmctl - управление сегментом (получение информации)
     Параметры:
     1. shm_id - ID сегмента
     2. IPC_STAT - команда "получить состояние"
     3. &ds - указатель на структуру shmid_ds, куда запишутся данные
  */
  shmctl (shm_id,
          IPC_STAT,
          &ds);

  shm_size = ds.shm_segsz;
  if (shm_size < strlen (SH_MESSAGE))
  {
  	fprintf (stderr, "error: segsize=%d\n", shm_size);
  	return 1;
  }

  strcpy (shm_buf,
          SH_MESSAGE);

  printf ("ID: %d\n", shm_id);
  printf ("Press <Enter> to exit...");
  fgetc (stdin);

  /* shmdt - отсоединение сегмента от процесса
     Параметры:
     1. shm_buf - указатель на начало области памяти
  */
  shmdt (shm_buf);

  /* shmctl - управление сегментом (удаление)
     Параметры:
     1. shm_id - ID сегмента
     2. IPC_RMID - команда "удалить сегмент"
     3. NULL - буфер не нужен для удаления
  */
  shmctl(shm_id,
         IPC_RMID,
         NULL);

  return 0;
}