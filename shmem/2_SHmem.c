/* Программа 2 для чтения текста исходного файла из разделяемой памяти.*/
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
/* Предполагаем, что размер исходного файла < SIZE */
#define SIZE 65535

int main()
{
  /* Указатель на разделяемую память */
  char *memory;
  /* IPC дескриптор для области разделяемой памяти */
  int shmid;
  /* Имя файла, использующееся для генерации ключа.
     Файл с таким именем должен существовать в текущей директории */
  char pathname[] = "QQQ.Q";
  /* IPC ключ */
  key_t key;

  /* Генерируем IPC ключ из имени файла  в текущей директории
     и номера экземпляра области разделяемой памяти 0 */
  /* ftok - параметры:
     1. путь к файлу
     2. id проекта (0)
  */
  if((key = ftok(pathname,0)) < 0)
  {
    printf("Can\'t generate key\n");
    exit(-1);
  }

  /* Пытаемся найти разделяемую память по сгенерированному ключу */
  /* shmget - параметры:
     1. key - ключ
     2. SIZE - размер (должен совпадать с созданным или быть 0 при поиске)
     3. 0666|IPC_CREAT - права доступа. IPC_CREAT здесь найдет существующую или создаст новую.
  */
  if((shmid = shmget(key, SIZE, 0666|IPC_CREAT)) < 0)
  {
    printf("Can\'t create shared memory\n");
    exit(-1);
  }

  /* Пытаемся отобразить разделяемую память в адресное пространство текущего процесса */
  /* shmat - параметры:
     1. shmid - ID
     2. NULL - авто-выбор адреса
     3. 0 - чтение/запись
  */
  if((memory = (char *)shmat(shmid, NULL, 0)) == (char *)(-1))
  {
    printf("Can't attach shared memory\n");
    exit(-1);
  }

  /* Печатаем содержимое разделяемой памяти */
  printf ("%s\n", memory);

  /* Отсоединяем разделяемую память и завершаем работу */
  /* shmdt - параметры:
     1. memory - адрес сегмента
  */
  if(shmdt(memory) < 0)
  {
    printf("Can't detach shared memory\n");
    exit(-1);
  }

  /* Удаляем разделяемую память из системы */
  /* shmctl - управление/удаление
     Параметры:
     1. shmid - ID сегмента
     2. IPC_RMID - команда удаления
     3. NULL - буфер для информации (не нужен при удалении)
  */
  (void)shmctl(shmid, IPC_RMID, (struct shmid_ds *)NULL);

  return 0;
}