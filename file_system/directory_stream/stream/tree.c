#include <common_header.h>

#define GAP 3
#define MAX_DEPTH 2
/// @brief 判断一个文件是否是隐藏的
/// @param file_name 文件名
/// @return 隐藏返回true
bool is_hide(char *file_name)
{
  return '.' == *file_name;
}

/// @brief 判断一个文件是否是文件夹
/// @param mode 文件 mode
/// @return 如果是文件夹返回true
bool is_dir(mode_t mode)
{
  return (mode & __S_IFMT) == __S_IFDIR;
}

/// @brief 判断一个文件是否是链接文件
/// @param mode
/// @return 是连接文件返回true
bool is_link(mode_t mode)
{
  return (mode & __S_IFMT) == __S_IFLNK;
}

/// @brief 更新当前动作路径
/// @param pwd_path 目标路径
void update_pwd(char *pwd_path)
{
  int ret = chdir(pwd_path);
  ERROR_CHECK(ret, -1, "chdir failed \n");
}

/// @brief 打印文件名称，如果当前文件时链接文件，则继续获取所链接的文件名称
/// @param d_name 文件地址
/// @param is_link 是否时链接文件
void print_file_name(const char *d_name, const char *relative_file_name, bool is_link, int dep)
{
  for (int i = 0; i < dep * GAP; i++)
  {
    if (i % GAP == 0)
      printf("│");
    else
      printf(" ");
  }
  printf("├── %s", d_name);
  if (is_link)
  {
    char file_name[128] = {'\0'};
    ssize_t ret_size = readlink(relative_file_name, file_name, 127);
    ERROR_CHECK(ret_size, -1, "readlink failed\n");
    printf("->%s", file_name);
  }
  printf("\n");
}

void do_sth(const struct dirent *direntp, const struct stat *statp, const char *file_name, int dep, int *file_count, int *dir_count)
{
  const char *d_name = direntp->d_name;
  mode_t mode = statp->st_mode;
  // 打印路径信息
  print_file_name(d_name, file_name, is_link(mode), dep);

  if (!is_dir(mode))
  {
    *(file_count) += 1;
  }
  else
  {
    *(dir_count) += 1;
  }
}

void tree(char *file_name, int dep, int *dir_count, int *file_count)
{
  DIR *dirp = opendir(file_name);
  ERROR_CHECK(dirp, NULL, file_name);

  struct dirent *direntp;
  while ((direntp = readdir(dirp)) != NULL)
  {
    char *d_name = direntp->d_name;
    // 隐藏文件夹跳过
    if (is_hide(direntp->d_name))
      continue;

    // 拼接路径
    if (strlen(file_name) + strlen(d_name) > 1023)
    {
      perror("file_name shortage!");
      exit(1);
    }
    strcat(file_name, "/");
    int rear = strlen(file_name);
    strcat(file_name, d_name);

    // 获取文件状态
    struct stat statp;
    ERROR_CHECK(lstat(file_name, &statp), -1, "stat failed");

    // 处理当前结点
    do_sth(direntp, &statp, file_name, dep, file_count, dir_count);

    // 不是文件则跳过
    if (!is_dir(statp.st_mode))
    {
      file_name[rear] = '\0';
      continue;
    }

    // 递归处理
    tree(file_name, dep + 1, dir_count, file_count);
    file_name[rear] = '\0';
  }
  // 关闭目录流
  closedir(dirp);
}

void get_path(int argc, char *args[], char *path)
{
  strcpy(path, ".\0");
  // 参数处理
  if (argc == 2)
  {
    strncpy(path, args[1], 1023);
  }
  else if (argc > 2)
  {
    errno = 1;
    char *msg = "参数个数错误";
    perror(msg);
    exit(1);
  }
}

int main(int argc, char *args[])
{
  char path[1024] = {'.', '\0'};
  get_path(argc, args, path);
  int dir_count = 0, file_count = 0;
  tree(path, 0, &dir_count, &file_count);
  printf("\ndirectory:%d   file:%d\n", dir_count, file_count);
  return 0;
}