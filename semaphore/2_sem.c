/* Программа 2: Увеличение семафора (V-операция / Post) */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h> /* !!! ОБЯЗАТЕЛЬНО: добавлено для работы с семафорами */
#include <stdio.h>
#include <stdlib.h>  /* Добавлено для функции exit() */

int main(int argc, char *argv[], char *envp[])
{
  int   semid;
  /* Имя файла для генерации ключа.
     ВАЖНО: Чтобы эта программа "увидела" тот же семафор, что и первая,
     здесь должен быть указан ТОТ ЖЕ файл, что и в первой программе!
     Если в первой было "1_sem.c", то и тут должно быть "1_sem.c". */
  char pathname[]="1_sem.c";
  key_t key;
  struct sembuf mybuf;

  /* ftok - Генерация ключа IPC
     Параметры:
     1. pathname - путь к существующему файлу.
     2. 0 - id проекта (должен совпадать с id в первой программе).
  */
  key = ftok(pathname, 0);

  /* semget - Получение ID массива семафоров
     Параметры:
     1. key - ключ IPC.
     2. 1 - количество семафоров (1 штук).
     3. 0666 | IPC_CREAT - права доступа и флаг создания.
        (Если семафор уже создан первой программой, IPC_CREAT просто вернет его ID).
  */
  if((semid = semget(key, 1, 0666 | IPC_CREAT)) < 0)
  {
    printf("Can\'t create semaphore set\n");
    exit(-1);
  }

  /* Настройка операции */
  /* sem_num - индекс семафора в массиве (0 - первый и единственный) */
  mybuf.sem_num = 0;

  /* sem_op - операция.
     1 : (V-операция) Увеличиваем значение семафора на 1.
         Это действие снимает блокировку с процессов, которые ждут (делают -1).
  */
  mybuf.sem_op  = 1;

  /* sem_flg - флаги.
     0 - стандартное поведение.
  */
  mybuf.sem_flg = 0;

  printf("Adding 1 to semaphore...\n");

  /* semop - выполнение операции
     Параметры:
     1. semid - ID массива семафоров.
     2. &mybuf - указатель на структуру с операцией.
     3. 1 - количество операций.
  */
  if(semop(semid, &mybuf, 1) < 0)
  {
    printf("Can\'t add 1 to semaphore\n");
    exit(-1);
  }

  printf("The condition is present (Semaphore incremented)\n");
  return 0;
}