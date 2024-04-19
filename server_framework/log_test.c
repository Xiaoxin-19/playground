#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#define MAX_LOG_LENGH 2048
// 日志级别
#define LOG_LEVEL_INFO 0
#define LOG_LEVEL_WARNING 1
#define LOG_LEVEL_ERROR 2
void print_log(int level, const char *format, ...);
#define LOG(level, format, ...)                                                                                       \
  do                                                                                                                  \
  {                                                                                                                   \
    char *levels[] = {                                                                                                \
        "INFO",                                                                                                       \
        "WARN",                                                                                                       \
        "ERRO"};                                                                                                      \
    char target_level[20] = {0};                                                                                      \
    if (level == LOG_LEVEL_INFO)                                                                                      \
      strncpy(target_level, levels[LOG_LEVEL_INFO], strlen(levels[LOG_LEVEL_INFO]));                                  \
    else if (level == LOG_LEVEL_WARNING)                                                                              \
      strncpy(target_level, levels[LOG_LEVEL_WARNING], strlen(levels[LOG_LEVEL_WARNING]));                            \
    else if (level == LOG_LEVEL_ERROR)                                                                                \
      strncpy(target_level, levels[LOG_LEVEL_ERROR], strlen(levels[LOG_LEVEL_ERROR]));                                \
    char temp_info[512] = {0};                                                                                        \
    snprintf(temp_info, MAX_LOG_LENGH, "[%s] [%s:%d:%s] %s", target_level, __FILE__, __LINE__, __FUNCTION__, format); \
    print_log(level, temp_info, ##__VA_ARGS__);                                                                       \
                                                                                                                      \
  } while (0)

#define LOG_WITH_TIME(level, format, ...)                                      \
  do                                                                           \
  {                                                                            \
    time_t rawtime;                                                            \
    struct tm *timeinfo;                                                       \
    time(&rawtime);                                                            \
    timeinfo = localtime(&rawtime);                                            \
    char new_format[1024] = {0};                                               \
    sprintf(new_format, "[%04d-%02d-%02d %02d:%02d:%02d] %s",                  \
            timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, \
            timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, (format));  \
    LOG(level, new_format, ##__VA_ARGS__);                                     \
  } while (0)

// 测试
int main()
{
  char buf[1024] = "hello log !";
  LOG_WITH_TIME(LOG_LEVEL_INFO, "This is an information message.%lu", pthread_self());
  LOG(LOG_LEVEL_WARNING, "This is a warning message.");
  LOG(LOG_LEVEL_ERROR, "This is an error message.");
  return 0;
}

void print_log(int level, const char *format, ...)
{
  va_list args;
  va_start(args, format); // 初始化参数列表
  char buf[1024] = {0};
  vsprintf(buf, format, args);
  va_end(args); // 结束参数列表
  if (level == LOG_LEVEL_ERROR)
  {
    strncat(buf, ":", strlen(":"));
    strncat(buf, strerror(errno), strlen(strerror(errno)));
  }
  strncat(buf, "\n", strlen("\n"));
  int fd = get_log_fd();
  write(fd, buf, strlen(buf));
}
