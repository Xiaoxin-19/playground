#include <common_header.h>
typedef struct dirent Dirent;
int main(int argc, char *args[])
{
  ARGS_CHECK(argc, 3);
  DIR *dirp = opendir(args[1]);
  Dirent *pdirent;
  long loc;
  while ((pdirent = readdir(dirp)) != NULL)
  {
    if (strcmp(pdirent->d_name, args[2]) != 0)
    {
      loc = telldir(dirp);
    }
    else
    {
      break;
    }
    printf("name: %s\n", pdirent->d_name);
  }
  seekdir(dirp, loc);
  pdirent = readdir(dirp);
  ERROR_CHECK(pdirent, NULL, "readdir faild \n");
  printf("查找到 %s 的inode:%lu \n", args[2], pdirent->d_ino);
  closedir(dirp);
  return 0;
}