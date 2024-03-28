#include <common_header.h>

typedef struct dirent dirent_t;
typedef struct stat stat_t;
/// @brief 获取文件类型
/// @param permission 文件权限uint
/// @return 代表文件类型的单个字符
char get_type(mode_t permission)
{
  mode_t res = permission & __S_IFMT;
  switch (res)
  {
  case __S_IFBLK:
    return 'b';
  case __S_IFCHR:
    return 'c';
  case __S_IFDIR:
    return 'd';
  case __S_IFIFO:
    return 'p';
  case __S_IFLNK:
    return 'l';
  case __S_IFREG:
    return '-';
  case __S_IFSOCK:
    return 's';
  default:
    return '?';
  }
}
/// @brief 获取文件权限组
/// @param permission 文件权限代码
/// @param str 待处理的权限可视化字符串
void mode_to_str(mode_t permission, char str[10])
{
  // 所有者权限
  str[0] = (permission & S_IRUSR) ? 'r' : '-';
  str[1] = (permission & S_IWUSR) ? 'w' : '-';
  str[2] = (permission & S_IXUSR) ? 'x' : '-';

  // 所属组权限
  str[3] = (permission & S_IRGRP) ? 'r' : '-';
  str[4] = (permission & S_IWGRP) ? 'w' : '-';
  str[5] = (permission & S_IXGRP) ? 'x' : '-';

  // other 权限
  str[6] = (permission & S_IROTH) ? 'r' : '-';
  str[7] = (permission & S_IWOTH) ? 'w' : '-';
  str[8] = (permission & S_IXOTH) ? 'x' : '-';

  str[9] = '\0';
}
/// @brief 根据uid 获取用户名称
/// @param uid
/// @param user_name 获取的username存放地址
void get_user_name(uid_t uid, char user_name[128])
{
  struct passwd *pw = getpwuid(uid);
  ERROR_CHECK(pw, NULL, "getpwnam failed\n");
  strncpy(user_name, pw->pw_name, 127);
  user_name[127] = '\0';
}
/// @brief 根据gid获取组名
/// @param gid
/// @param group_name 获取的组名存放地址
void get_group_name(gid_t gid, char group_name[128])
{
  struct group *gp = getgrgid(gid);
  ERROR_CHECK(gp, NULL, "getgrgid failed\n");
  strncpy(group_name, gp->gr_name, 127);
  group_name[127] = '\0';
}
/// @brief 获取文件最后一次修改的时间
/// @param timestamp 时间戳
/// @param time 结果存放的地址
void get_last_time(const time_t *timestamp, struct tm *time)
{
  *time = *localtime(timestamp);
  ERROR_CHECK(time, NULL, "localtime failed\n");
}
/// @brief 打印文件名称，如果当前文件时链接文件，则继续获取所链接的文件名称
/// @param d_name 文件地址
/// @param is_link 是否时链接文件
void print_file_name(char *d_name, bool is_link)
{
  printf("%s", d_name);
  if (is_link)
  {
    char file_name[128] = {'\0'};
    ssize_t ret_size = readlink(d_name, file_name, 127);
    ERROR_CHECK(ret_size, -1, "readlink failed\n");
    printf("->%s", file_name);
  }
}
/// @brief 打印最后一次修改时间，如果和当前时间不是同一年，则打印年份
/// @param time_str 格式化时间字符串 从asctime（）获取
/// @param last_time 最后一次修改时间
/// @param now 当前时间
void print_file_time(char *time_str, struct tm *last_time, struct tm *now)
{
  // 输出月份和 当月天数
  char mon[5] = {'\0'};
  strncat(mon, time_str + 4, 3);
  printf("%3s %2d ", mon, last_time->tm_mday);
  if (last_time->tm_year != now->tm_year)
  {
    printf("%8d ", 1900 + last_time->tm_year);
  }
  else
  {
    char time_detail[20] = {'\0'};
    strncat(time_detail, time_str + 11, 8);
    printf("%s ", time_detail);
  }
}
/// @brief 打印文件信息
/// @param direntp
/// @param statpp
/// @param now
void print_file_info(struct dirent *direntp, struct stat *statpp, struct tm *now)
{
  // 获取类型信息
  char type = get_type(statpp->st_mode);
  // 获取权限信息
  char permission[10];
  mode_to_str(statpp->st_mode, permission);
  // 获取用户名和组名
  char user_name[128] = {'\0'}, group_name[128] = {'\0'};
  get_group_name(statpp->st_gid, group_name);
  get_user_name(statpp->st_uid, user_name);
  // 获取最后修改时间
  struct tm last_time;
  get_last_time(&statpp->st_mtime, &last_time);
  char *time_str = asctime(&last_time);
  // 输出信息
  printf("%c%s %3lu %s %s  %8ld ",
         type,
         permission,
         statpp->st_nlink,
         user_name,
         group_name,
         statpp->st_size);
  // 输出最后修改时间和文件名
  print_file_time(time_str, &last_time, now);
  print_file_name(direntp->d_name, type == 'l');
  printf("\n");
}

/// @brief 更新当前动作路径
/// @param pwd_path 目标路径
void update_pwd(char *pwd_path)
{
  int ret = chdir(pwd_path);
  ERROR_CHECK(ret, -1, "chdir failed \n");
  char *pwd = getcwd(NULL, 0);
  ERROR_CHECK(pwd, NULL, "getcwd failed\n");
  printf("当前工作路径为： %s\n", pwd);
  free(pwd);
}
int main(int argc, char *args[])
{
  char path[1024] = {'\0'};
  ERROR_CHECK(getcwd(path, sizeof(path)), NULL, "getcwd failed\n");
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
  // 打开目录流
  DIR *dirp = opendir(path);
  ERROR_CHECK(dirp, NULL, "opendir failed \n");

  dirent_t *direntp;
  int ret;

  // 切换当前工作目录
  update_pwd(path);

  // 获取当前时间
  time_t new_timestamp = time(NULL);
  struct tm now = *(localtime(&new_timestamp));

  // 遍历读取目录项
  while ((direntp = readdir(dirp)) != NULL)
  {
    // 获取文件stat
    stat_t statpp;
    ret = lstat(direntp->d_name, &statpp);
    ERROR_CHECK(ret, -1, "stat failed \n");
    // 输出文件信息
    print_file_info(direntp, &statpp, &now);
  }
  closedir(dirp);
  return 0;
}