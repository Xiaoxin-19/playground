#include <common_header.h>

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

void copy_file(const char *src, const char *dest)
{
  FILE *src_fp = fopen(src, "rb");
  ERROR_CHECK(src_fp, NULL, "fopen src faild!");
  FILE *dest_fp = fopen(dest, "wb");
  if (dest_fp == NULL)
  {
    free(src_fp);
    perror("fopen dest faild!");
    exit(1);
  }
  char buf[1024] = {0};
  size_t count;
  while ((count = fread(buf, 1, sizeof(buf), src_fp)) > 0)
  {
    fwrite(buf, 1, count, dest_fp);
  }
  fclose(src_fp);
  fclose(dest_fp);
}

void cat_file_name(char *path, char *file_name)
{
  int rear = strlen(path);
  if (path[rear - 1] != '/')
    strcat(path, "/\0");
  strcat(path, file_name);
}

void cp_dir(char *src_path, char *dest_path)
{

  // 创建目录
  char src_backup[1024];
  strcpy(src_backup, src_path);
  char *dir_name = basename(src_backup);
  if (strcmp(dir_name, "/") != 0 && strcmp(dir_name, ".") != 0)
  {
    cat_file_name(dest_path, dir_name);
  }
  // 获取文件状态
  struct stat statp;
  // 默认复制连接指向的文件而不是链接文件本身
  ERROR_CHECK(stat(src_path, &statp), -1, "stat failed");
  int ret = mkdir(dest_path, statp.st_mode);
  ERROR_CHECK(ret, -1, "mkdir failed \n");

  // 遍历复制字文件项
  DIR *dirp = opendir(src_path);
  ERROR_CHECK(dirp, NULL, "opendir failed in cp_dir");
  struct dirent *direntp;
  while ((direntp = readdir(dirp)) != NULL)
  {
    if (is_hide(direntp->d_name))
      continue;
    char *d_name = direntp->d_name;
    // 获取文件长度用于重置路径状态
    int rear_src = strlen(src_path);
    int rear_dest = strlen(dest_path);

    cat_file_name(src_path, d_name);

    struct stat sub_stat;
    ERROR_CHECK(stat(src_path, &sub_stat), -1, "stat failed in sub_stat");
    if (is_dir(sub_stat.st_mode))
    {
      cp_dir(src_path, dest_path);
      src_path[rear_src] = '\0';
      dest_path[rear_dest] = '\0';
      continue;
    }

    cat_file_name(dest_path, d_name);
    copy_file(src_path, dest_path);

    // 重置路径
    src_path[rear_src] = '\0';
    dest_path[rear_dest] = '\0';
  }
  closedir(dirp);
}

void cp(char *src_path, char *dest_path)
{
  if (strcmp(src_path, dest_path) == 0)
  {
    errno = 1;
    perror("src and dest are same file");
    exit(1);
  }

  struct stat statp;
  ERROR_CHECK(stat(src_path, &statp), -1, "stat src_path in pre_cp failed");
  if (is_dir(statp.st_mode))
  {
    cp_dir(src_path, dest_path);
    return;
  }

  // 获取文件名称
  char *file_name = strrchr(src_path, '/');
  ERROR_CHECK(file_name, NULL, "strrchr find / failed in pre_ce");
  // 添加文件名
  cat_file_name(dest_path, file_name);
  // 拷贝文件
  copy_file(src_path, dest_path);
}

int main(int argc, char *args[])
{
  ARGS_CHECK(argc, 3);
  char src[1024] = {0}, dest[1024] = {0};
  ERROR_CHECK(realpath(args[1], src), NULL, "realpath src failed");
  ERROR_CHECK(realpath(args[2], dest), NULL, "realpath dest failed");
  cp(src, dest);
  return 0;
}