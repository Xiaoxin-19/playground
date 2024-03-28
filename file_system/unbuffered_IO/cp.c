#include <common_header.h>

long long current_time_millis()
{
  struct timespec TS;
  clock_gettime(CLOCK_REALTIME, &TS);
  return (long long)TS.tv_sec * 1000LL + TS.tv_nsec / 1000000;
}

void cp_unbuffered(char *src, char *dest)
{
  int fd_src = open(src, O_RDONLY, 0777);
  ERROR_CHECK(fd_src, -1, "open src failed");
  int fd_dest = open(dest, O_WRONLY | O_CREAT, 0777);
  ERROR_CHECK(fd_dest, -1, "open dest faild");
  int count;

  char buf[4096 * 1024] = {0};
  while ((count = read(fd_src, buf, sizeof(buf))) > 0)
  {
    ERROR_CHECK(write(fd_dest, buf, count), -1, "write failed ");
  }
  if (count == -1)
  {
    perror("read failed");
    exit(1);
  }
  close(fd_dest);
  close(fd_src);
}

void cp_mmap(char *src, char *dest)
{
  int size = 4096 * 1024;
  int fd_src = open(src, O_RDONLY);
  ERROR_CHECK(fd_src, -1, "open src failed");
  struct stat statp;
  stat(src, &statp);

  int fd_dest = open(dest, O_RDWR | O_CREAT | O_TRUNC, statp.st_mode);
  ERROR_CHECK(fd_dest, -1, "open dest faild");
  ERROR_CHECK(ftruncate(fd_dest, statp.st_size), -1, "occupt faild");

  int copy_size;

  off_t offset = 0;
  void *srcp, *destp;
  while (offset < statp.st_size)
  {
    copy_size = (statp.st_size - offset) > size ? size : statp.st_size - offset;
    ERROR_CHECK((srcp = mmap(0, copy_size, PROT_READ, MAP_SHARED, fd_src, offset)), MAP_FAILED, "mmap res failed");
    destp = mmap(NULL, copy_size, PROT_WRITE, MAP_SHARED, fd_dest, offset);
    ERROR_CHECK(destp, MAP_FAILED, "map dest failed");
    memcpy(destp, srcp, copy_size);
    munmap(srcp, copy_size);
    munmap(destp, copy_size);
    offset += copy_size;
  };
  close(fd_src);
  close(fd_dest);
}

void cp_buffered(char *src, char *dest)
{
  FILE *f_frc = fopen(src, "rb");
  ERROR_CHECK(f_frc, NULL, "fopen failed");
  FILE *f_dest = fopen(dest, "wb");
  ERROR_CHECK(f_dest, NULL, "open dest failed in cp buffered");
  char buf[4096] = {0};
  size_t count = 0;
  while ((count = fread(buf, 1, 4096, f_frc)) > 0)
  {
    fwrite(buf, 1, count, f_dest);
  }
  if (count == -1)
  {
    perror("fread failed");
    exit(errno);
  }
  fclose(f_frc);
  fclose(f_dest);
}

int main(int argc, char *args[])
{
  ARGS_CHECK(argc, 3);
  long long begin = current_time_millis();
  cp_buffered(args[1], "./copy_buffered.mkv");
  long long end = current_time_millis();
  printf("comsume time in test buffered: %lld\n", end - begin);
  begin = current_time_millis();
  cp_unbuffered(args[1], "./copy_unbuffered.mkv");
  end = current_time_millis();
  printf("comsume time in unbuffered: %lld\n", end - begin);

  begin = current_time_millis();
  cp_mmap(args[1], "./copy_mmap.mkv");
  end = current_time_millis();
  printf("comsume time in mmap: %lld\n", end - begin);

  return 0;
}
