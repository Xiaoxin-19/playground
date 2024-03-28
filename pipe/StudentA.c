#include <common_header.h>
int main(int argc, char *argv[])
{
  int fd_read = open("1.pipe", O_RDONLY);
  int fd_write = open("2.pipe", O_WRONLY);
  char buf[60];

  fd_set set;

  struct timeval time_val;
  time_val.tv_sec = 10;
  time_val.tv_usec = 0;
  while (1)
  {
    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set);
    FD_SET(fd_read, &set);

    int res_select = select(10, &set, NULL, NULL, &time_val);

    if (FD_ISSET(fd_read, &set))
    {
      bzero(buf, sizeof(buf));
      int read_num = read(fd_read, buf, sizeof(buf));
      if (read_num == 0)
      {
        printf("对方断开链接 \n");
        break;
      }
      printf("UserA: %s", buf);

      time_val.tv_sec = 10;
      time_val.tv_usec = 0;
    }
    else
    {
      if (time_val.tv_sec <= 0)
      {
        printf("对方十秒未说话 \n");
        break;
      }
    }
    if (FD_ISSET(STDIN_FILENO, &set))
    {
      bzero(buf, sizeof(buf));
      int read_stdin = read(STDIN_FILENO, buf, sizeof(buf));
      if (read_stdin == 0)
      {
        // 用户按下ctrl+d; 输入文件终止符; 终止标准输入; read返回0
        break;
      }
      write(fd_write, buf, sizeof(buf));
    }
  }
  close(fd_write);
  close(fd_read);
  return 0;
}