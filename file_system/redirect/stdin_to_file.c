#include <common_header.h>

int main(void)
{
  close(STDIN_FILENO);
  int fd = open("file_out", O_RDONLY);
  char str[100];
  scanf("%s", str);
  printf("%s\n", str);
  return 0;
}