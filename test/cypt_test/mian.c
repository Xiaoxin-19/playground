#include "ow-crypt.h"
#include <stdio.h>
#include <strings.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>

void encrypt_password(const char *password, char *encrypted);
bool match_password(const char *password, const char *encrypted);
void generate_random_string(int length, char *output);

#define CRYPT_OUTPUT_SIZE 128
#define CRYPT_MAX_PASSPHRASE_SIZE 128

struct crypt_data
{
  char output[CRYPT_OUTPUT_SIZE];
  char setting[CRYPT_OUTPUT_SIZE];
  char phrase[CRYPT_MAX_PASSPHRASE_SIZE];
  char initialized;
};
int main(void)
{
  char encrypted[CRYPT_OUTPUT_SIZE];
  char *password = "123456";
  char *input_password = "hello world";

  encrypt_password(password, encrypted);
  printf("Encrypted: %s\n", encrypted);
  if (match_password(input_password, encrypted))
  {
    printf("Password match\n");
  }
  else
  {
    printf("Password does not match\n");
  }
  return 0;
}

void encrypt_password(const char *password, char *encrypted)
{
  struct crypt_data data;
  bzero(&data, sizeof(data));
  char rand_str[25];
  generate_random_string(24, rand_str);
  crypt_gensalt_rn("$2b$", 12, rand_str, strlen(rand_str), data.setting, CRYPT_OUTPUT_SIZE);
  printf("Salt: %s\n", data.setting);
  char *hash = crypt_r(password, data.setting, &data);
  strcpy(encrypted, hash);
}

bool match_password(const char *password, const char *encrypted)
{
  struct crypt_data data;
  bzero(&data, sizeof(data));
  struct timeval begin, end;
  gettimeofday(&begin, NULL);
  printf("begin sec : %lu usec:%lu \n", begin.tv_sec, begin.tv_usec);
  char *hash = crypt_r(password, encrypted, &data);
  gettimeofday(&end, NULL);
  printf("end sec : %lu usec:%lu \n", end.tv_sec, end.tv_usec);
  unsigned long elapsed = (end.tv_sec - begin.tv_sec) * 1000000LL + (end.tv_usec - begin.tv_usec);
  printf("the time diff si %lu\n", elapsed / 1000);
  printf("Hash: %s\n", hash);
  return strcmp(data.output, encrypted) == 0;
}

// 生成随机字符串
void generate_random_string(int length, char *output)
{
  static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_><?@#$^&*()";

  if (length <= 0)
  {
    return;
  }

  srand(time(NULL)); // 设置随机种子

  for (int i = 0; i < length; i++)
  {
    output[i] = charset[rand() % (sizeof(charset) - 1)];
  }
  output[length] = '\0'; // 添加字符串结束符
}