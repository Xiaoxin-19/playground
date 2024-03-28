#include <common_header.h>

int main(void)
{
  fd_set rfds;
  int config_fd, trans_fd;
  ERROR_CHECK((config_fd = open("../1.pipe", O_RDONLY)), -1, "1 open failed");
  ERROR_CHECK((trans_fd = open("../2.pipe", O_RDONLY)), -1, "2 open failed");
  int max_fd = config_fd > trans_fd ? config_fd : trans_fd;
  struct timeval tv;
  char dir_name[128] = "./store/";
  char tran_buf[4096 * 512] = {0};
  int fd = 0;
  if (opendir(dir_name) == NULL)
    ERROR_CHECK(mkdir(dir_name, 0777), -1, "mkdir failed");

  while (true)
  {
    FD_ZERO(&rfds);
    FD_SET(config_fd, &rfds);
    FD_SET(trans_fd, &rfds);
    tv.tv_sec = 20;
    tv.tv_usec = 0;
    int ret = select(max_fd + 1, &rfds, NULL, NULL, &tv);

    ERROR_CHECK(ret, -1, "select error");

    if (ret == 0)
    {
      printf("超时断开。。。\n");
      break;
    }

    if (FD_ISSET(config_fd, &rfds))
    {
      char info[512] = {0};
      int count = 0;
      ERROR_CHECK(count = read(config_fd, info, sizeof(info)), -1, "read config in recv failed");
      if (count == 0)
        break;
      char *p = info;
      char path[128] = {0};
      int idx = 0;
      while ((*p) != ' ' && *p != '\0')
      {
        p++;
        idx++;
      }
      // 分割文件名 和空格 分开的文件大小
      strcpy(path, info);
      path[idx] = '\0';

      printf("文件路径为：%s\n", path);
      int len = atoi(info + idx);
      printf("文件长度为：%d", len);

      // 占位
      ftruncate(fd, len);
      strcat(dir_name, path);
      ERROR_CHECK((fd = open(dir_name, O_RDWR | O_CREAT | O_TRUNC, 077)), -1, "create failed");
    }
    if (FD_ISSET(trans_fd, &rfds))
    {
      int count;
      ERROR_CHECK((count = read(trans_fd, tran_buf, sizeof(tran_buf))), -1, "read failed in recev");
      ERROR_CHECK(write(fd, tran_buf, count), -1, "write failed in rece");
    }
  }
  printf("复制完成\n");
  close(fd);
  close(config_fd);
  close(trans_fd);
  return 0;
}