#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Функция проверки */
void check(char *a, char *b,
           int (*cmp)(const char *, const char *))
{
  printf("Проверка...\n");
  /* cmp возвращает 0, если условия "равенства" выполнены.
     !0 дает 1 (истину). */
  if(!(*cmp)(a, b))
      printf("Результат: РАВНЫ\n");
  else
      printf("Результат: НЕ РАВНЫ\n");
}

/* Новая функция: Сравнение по длине */
int cmp_length(const char *a, const char *b)
{
    /* strlen возвращает длину строки */
    if (strlen(a) == strlen(b))
        return 0; /* 0 означает "равны" (по длине) */
    else
        return 1; /* Не 0 означает "не равны" */
}

int main(int argc, char *argv[])
{
  char s1[80], s2[80];

  /* Объявление указателя на функцию.
     Он может указывать на любую функцию, принимающую два (const char*)
     и возвращающую int. */
  int (*p)(const char *, const char *);

  /* Проверка аргументов запуска */
  if (argc < 2) {
      fprintf(stderr, "Использование: %s <режим>\n", argv[0]);
      fprintf(stderr, "1 - сравнение содержимого\n");
      fprintf(stderr, "2 - сравнение длины\n");
      return 1;
  }

  /* Выбор логики в зависимости от аргумента */
  int mode = atoi(argv[1]); // Преобразуем аргумент в число

  if (mode == 2) {
      printf("[Режим]: Сравнение длины строк\n");
      p = cmp_length; /* Присваиваем адрес нашей новой функции */
  } else {
      printf("[Режим]: Сравнение содержимого (стандартный strcmp)\n");
      p = strcmp;     /* Присваиваем адрес стандартной функции */
  }

  printf("Введите две строки:\n");

  /* Используем fgets вместо gets для безопасности.
     Также удаляем символ переноса строки '\n', который fgets захватывает. */
  printf("Строка 1: ");
  fgets(s1, sizeof(s1), stdin);
  s1[strcspn(s1, "\n")] = 0; // Удаление \n

  printf("Строка 2: ");
  fgets(s2, sizeof(s2), stdin);
  s2[strcspn(s2, "\n")] = 0; // Удаление \n

  /* Вызываем check, передавая выбранную через указатель функцию */
  check(s1, s2, p);

  return 0;
}