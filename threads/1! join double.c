#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

/*
 * ОТВЕТ НА ВОПРОС 1:
 * Я оставил переменную 'a' глобальной.
 * Если сделать её локальной внутри any_func, то она уничтожится при выходе из функции,
 * и я верну "битый" указатель (на мусор в стеке). Глобальная переменная живет вечно.
 */
double a;

void * any_func (void * arg)
{
  printf ("[Поток] Получил указатель arg: %p\n", arg);
  printf ("[Поток] Значение *arg: %f\n", *(double *)arg);

  /* Копирую переданное число в глобальную переменную */
  a = *(double *)arg;
  a++;

  printf ("[Поток] Увеличил a до: %f\n", a);

  double * pa = &a; // Беру адрес глобальной переменной
  printf ("[Поток] Возвращаю адрес pa: %p\n", pa);

  return pa;
}

int main (void)
{
  pthread_t thread;
  double data = 2007;
  void * pp = &data;

  /*
     ИСПРАВИЛ ОШИБКУ ЗДЕСЬ:
     В исходнике была попытка записать указатель (8 байт) в int q (4 байта).
     Это ломало стек (segmentation fault).
     Я создал нормальный указатель void*, чтобы принять результат.
  */
  void * thread_result_ptr;

  printf ("[Main] Отправляю pp: %p\n", pp);

  if (pthread_create(&thread, NULL, &any_func, pp) != 0) {
    fprintf (stderr, "Ошибка\n");
    return 1;
  }

  /*
     ОТВЕТ НА ВОПРОС 2 (про двойную косвенную адресацию):
     pthread_join хочет изменить мой указатель thread_result_ptr,
     чтобы он указывал туда же, куда указывает pa из потока.
     В Си, чтобы функция изменила переменную, ей нужно передать АДРЕС этой переменной.
     Адрес указателя (void*) — это указатель на указатель (void**).
  */
  pthread_join(thread, &thread_result_ptr);

  printf ("\n[Main] Поток завершился.\n");
  printf ("[Main] Получил адрес: %p\n", thread_result_ptr);

  /* Привожу полученный void* к double* и смотрю значение */
  double * result_double_ptr = (double *)thread_result_ptr;
  printf ("[Main] Число по этому адресу: %f\n", *result_double_ptr);

  /* Проверка: меняю глобальную a, и число через указатель тоже меняется */
  a++;
  printf ("[Main] Проверка связи (еще +1): %f\n", *result_double_ptr);

  return 0;
}