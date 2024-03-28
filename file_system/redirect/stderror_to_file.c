#include <common_header.h>

int main(void)
{
  int pd = open("error.log", O_RDWR | O_APPEND);
  dup2(pd, STDERR_FILENO);
  perror("hello world");
  return 0;
}