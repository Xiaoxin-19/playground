#include <common_header.h>
#include <stdarg.h>
int max_int(int n, ...);
int min_int(int n, ...);
int my_test(void *str, void *int_a, void *double_b, void *my_struct);
#define even_handler int (*f)(int, ...)
typedef struct
{
  int a;
  char *str;
  double b;
} my_struct;

int main(void)
{
  int (*f[])(char *) = {max_int, min_int, my_test};
  my_struct tt = {1, "hello world", 12.12};
  for (int i = 0; i < 2; i++)
  {
    printf("the result is : %d\n", f[i](3, 1, 2, 3));
  }
  return 0;
}


int min_int(int n, ...) /* n must be at least 1 */
{
  va_list ap;
  int i, current, largest;
  va_start(ap, n);
  largest = va_arg(ap, int);
  for (i = 1; i < n; i++)
  {
    current = va_arg(ap, int);
    printf("current val is : %d \n", current);
    if (current < largest)
      largest = current;
  }
  va_end(ap);
  return largest;
}

int my_test(void *str, void *int_a, void *double_b, void *my_struc)
{
  char *my_str = (char *)str;
  int a = *(int *)int_a;
  double b = *((double *)double_b);
  my_struct my = *((my_struct *)my_struc);
  printf("str: %s, int :%d, double : %lf \n", my_str, a, b);
  printf("mystruct str: %s, int :%d, double : %lf \n", my.str, my.a, my.b);
}