#include <common_header.h>
/*
 * 定义一个学生结构体类型struct student，
 * 里边含有学号(int)，姓名，分数(float)。
 * 定义结构体数组struct student s[3]
 * 给数组赋初值后，写入文件，然后通过lseek偏移到开头，然后再读取并使用printf输出。
 */
typedef struct
{
  int id;
  char name[128];
  float score;
} student;

int main(void)
{
  student stus[3] = {{1, "001", 100}, {2, "002", 99}, {3, "98", 90}};
  int fd = open("student", O_RDWR | O_CREAT | O_TRUNC, 0777);
  ERROR_CHECK(fd, -1, "open filed");
  write(fd, stus, sizeof(stus));
  lseek(fd, 0, SEEK_SET);
  student stuss[3] = {{}};
  read(fd, stuss, sizeof(stuss));
  for (int i = 0; i < 3; i++)
  {
    printf("id : %d name:%s score:%.2f\n", stuss[i].id, stuss[i].name, stuss[i].score);
  }

  return 0;
}