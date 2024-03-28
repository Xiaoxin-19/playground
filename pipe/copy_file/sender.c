#include <common_header.h>
typedef enum
{
  CONFIG_PATH,
  CONFIG_LEN,
  TRANSFORM,
  END,
} State;

int main(void)
{
  fd_set wfds;
  int config_fd, trans_fd;
  ERROR_CHECK((config_fd = open("../1.pipe", O_WRONLY)), -1, "1 open failed");
  ERROR_CHECK((trans_fd = open("../2.pipe", O_WRONLY)), -1, "2 open failed");
  int max_fd = config_fd > trans_fd ? config_fd : trans_fd;
  struct timeval tv;
  State state = CONFIG_PATH;
  struct stat statp;
  char path[512] = "/home/ubuntu/etc/source/file_system/unbuffered_IO/assp.mkv";
  char filename[123] = "assp.mkv";
  ERROR_CHECK(stat(path, &statp), -1, "stat failed");

  int fd = open(path, O_RDONLY);
  char tran_buf[4096 * 512] = {0};
  while (true)
  {
    FD_ZERO(&wfds);
    FD_SET(config_fd, &wfds);
    FD_SET(trans_fd, &wfds);
    tv.tv_sec = 20;
    tv.tv_usec = 0;
    int ret = select(max_fd + 1, NULL, &wfds, NULL, &tv);
    ERROR_CHECK(ret, -1, "select error");
    if (state != TRANSFORM)
    {

      strcat(filename, " ");
      char len[12] = {'\0'};
      sprintf(len, "%ld", statp.st_size);
      strcat(filename, len);
      write(config_fd, filename, sizeof(filename));
      state = TRANSFORM;
    }
    else if (state == TRANSFORM && FD_ISSET(trans_fd, &wfds))
    {
      int count;
      ERROR_CHECK((count = read(fd, tran_buf, sizeof(tran_buf))), -1, "read in sender failed");
      if (count <= 0)
      {
        state = END;
        break;
      }

      ERROR_CHECK(write(trans_fd, tran_buf, count), -1, "write failed in trans");
    }
  }
  printf("传输完成\n");
  close(config_fd);
  close(trans_fd);
  return 0;
}