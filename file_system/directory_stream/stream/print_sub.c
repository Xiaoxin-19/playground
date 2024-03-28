#include <common_header.h>

typedef struct dirent Dirent;
int main(int argc, char *args[])
{
  ARGS_CHECK(argc, 2);
  DIR *dirp = opendir(args[1]);
  ERROR_CHECK(dirp, NULL, "opendir faild\n");
  Dirent *pdirent;
  while ((pdirent = readdir(dirp)) != NULL)
  {
    printf("inode = %ld, reclen = %d, type = %d, name = %s\n",
           pdirent->d_ino, pdirent->d_reclen,
           pdirent->d_type, pdirent->d_name);
  }
  closedir(dirp);
  return 0;
}