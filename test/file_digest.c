#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>

#define BUFFER_SIZE (4096 * 1024)
#define DIGEST_LEN (64 + 1)
// 将字符串转换为十六进制字符串
void string_to_hex(const unsigned char *input, int length, char *output);
void generate_digest(int fd, char *output);
bool match_digest(int fd, char *digest);
int main()
{
  int file_fd;

  // 打开文件
  file_fd = open("./digest.c", O_RDONLY);
  if (file_fd == -1)
  {
    fprintf(stderr, "Failed to open file.\n");
    return 1;
  }

  char digest[DIGEST_LEN];
  generate_digest(file_fd, digest);
  // 输出摘要
  printf("SHA-256 Digest: %s\n", digest);
  bool ret = match_digest(file_fd, digest);
  printf("the match result is %d\n", ret);
  return 0;
}

/// @brief 生成文件摘要
/// @param fd 文件描述符
/// @param output 结果字符串
void generate_digest(int fd, char *output)
{
  lseek(fd, 0, SEEK_SET);
  unsigned char buffer[BUFFER_SIZE];
  ssize_t bytesRead;
  SHA256_CTX sha256Context;
  unsigned char hash[SHA256_DIGEST_LENGTH];

  // 初始化SHA-256上下文
  SHA256_Init(&sha256Context);

  // 逐块读取文件并更新哈希值
  while ((bytesRead = read(fd, buffer, BUFFER_SIZE)) > 0)
  {
    SHA256_Update(&sha256Context, buffer, bytesRead);
  }

  // 计算最终哈希值
  SHA256_Final(hash, &sha256Context);

  // 将哈希值转换为十六进制字符串
  string_to_hex(hash, SHA256_DIGEST_LENGTH, output);
}

/// @brief 将hash值转换为十六进制摘要字符串
/// @param input hash 值
/// @param length hash值长度
/// @param output 结果返回指针
void string_to_hex(const unsigned char *input, int length, char *output)
{
  if (!output)
  {
    return;
  }

  for (size_t i = 0; i < length; i++)
  {
    sprintf(output + i * 2, "%02X", input[i]);
  }
  output[length * 2] = '\0'; // 添加字符串结束符
}

/// @brief 判断文件是否和给定的摘要匹配, fd 文件游标位置自动定位到文件开始
/// @param fd 文件描述符
/// @param digest 目标摘要
/// @return 匹配返回 true，否则返回 false
bool match_digest(int fd, char *digest)
{
  lseek(fd, 0, SEEK_SET);
  char inputDigest[DIGEST_LEN];
  generate_digest(fd, inputDigest);
  printf("inputDigest: %s\n", inputDigest);
  return strcmp(digest, inputDigest) == 0;
}