#include <stdio.h>
#include <pthread.h>
#include <stdint.h> /* Подключил, чтобы использовать intptr_t для безопасного каста */

/*
   Компиляция: gcc task1.c -o task1 -pthread
*/

void * any_func (void * arg)
{
  /* Получаю значение int из указателя. */
  int a = *(int *) arg;
  a++;

  /* Возвращаем число, "пряча" его в тип void*.
     Использую intptr_t, чтобы компилятор не ругался на размер типов. */
  return (void *)(intptr_t) a;
}

int main (void)
{
  pthread_t thread;
  int parg = 2007;
  void *thread_result;
  int result_int;

  /* Создаю поток. Он возьмет 2007, сделает +1 и вернет 2008. */
  if (pthread_create (&thread, NULL, &any_func, &parg) != 0)
  {
    fprintf (stderr, "Ошибка создания потока\n");
    return 1;
  }

  /* Жду поток и забираю результат (2008) в переменную thread_result */
  pthread_join (thread, &thread_result);

  /* Превращаю результат обратно в число */
  result_int = (int)(intptr_t)thread_result;

  /* ЗАДАНИЕ ВЫПОЛНЕНО ТУТ:
     Я вызываю ту же функцию any_func еще раз, но уже в main.
     Передаю ей адрес числа 2008. Она сделает +1 -> получится 2009. */
  void * final_ptr = any_func(&result_int);

  /* Преобразую итоговый результат в int для печати */
  int final_result = (int)(intptr_t)final_ptr;

  printf("%d\n", final_result); // Должно вывести 2009

  return 0;
}