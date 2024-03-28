#include <common_header.h>

int main(int argc, char *args[])
{
  char *pwd = getcwd(NULL, 0);
  ERROR_CHECK(pwd, NULL, "getcwd faild\n");
  char *old_direct = pwd;
  printf("获取当前工作目录为：%s\n", pwd);
  free(pwd);

  char *new_path = "./new_directory";
  int ret = mkdir(new_path, 0777);
  ERROR_CHECK(ret, -1, "mkdir failed \n");
  printf("创建了新目录/new_directory\n");

  ret = chdir(new_path);
  ERROR_CHECK(ret, -1, "chdir failed\n");
  pwd = getcwd(NULL, 0);
  ERROR_CHECK(pwd, NULL, "getcwd faild\n");
  printf("切换到当前工作目录为：%s\n", pwd);
  free(pwd);

  ret = chdir("..");
  ERROR_CHECK(ret, -1, "chdir faild \n");
  pwd = getcwd(NULL, 0);
  ERROR_CHECK(pwd, NULL, "getcwd faild\n");
  printf("当前的工作目录为：%s\n", pwd);

  ret = rmdir(new_path);
  ERROR_CHECK(ret, -1, "rmdir faild\n");
  printf("删除 %s success\n", new_path);

  ARGS_CHECK(argc, 3);
  mode_t mask = strtoul(args[2], NULL, 8);
  printf("the mask is %u \n", mask);
  ret = chmod(args[1], mask);
  ERROR_CHECK(ret, -1, "chmod faild");
  return 0;
}
