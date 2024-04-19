#include <mysql/mysql.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>

#define MAX_POOL_SIZE 4

typedef struct
{
  int size;
  void *data;
} query_set;

typedef struct
{
  unsigned long id;
  char name[30];
  char password[70];
  time_t delete_time;
} user_t;

typedef struct
{
  MYSQL *connection;
  bool in_use;
} Connection;

typedef struct
{
  Connection connects[MAX_POOL_SIZE];
  int available;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
} Db_pool;
Db_pool *pool;
query_set *get_result_set(MYSQL *conn, void (*convert)(MYSQL_ROW *row, void *target), unsigned int data_size);
void convert_to_user(MYSQL_ROW *row, void *user);
void free_result_set(query_set *set);
void query(MYSQL *conn, const char *format, ...);
void init_pool(const char *host, const char *user, const char *password, const char *db, unsigned int port)
{
  pool = (Db_pool *)malloc(sizeof(Db_pool));

  pthread_cond_init(&(pool->cond), NULL);
  pthread_mutex_init(&(pool->mutex), NULL);
  pool->available = MAX_POOL_SIZE;

  for (int i = 0; i < MAX_POOL_SIZE; ++i)
  {

    pool->connects[i].connection = mysql_init(NULL);
    if (!mysql_real_connect(pool->connects[i].connection, host, user, password, db, port, NULL, 0))
    {
      write(STDOUT_FILENO, "Failed to connect to MySQL: Error: ", 36);
      printf("%s\n", mysql_error(pool->connects[i].connection));
      exit(1);
    }
    mysql_set_character_set(pool->connects[i].connection, "utf8");
    pool->connects[i].in_use = false;
  }
}

MYSQL *get_connection()
{
  pthread_mutex_lock(&(pool->mutex));
  while (pool->available == 0)
  {
    pthread_cond_wait(&(pool->cond), &(pool->mutex));
  }

  for (int i = 0; i < MAX_POOL_SIZE; ++i)
  {
    if (!pool->connects[i].in_use)
    {
      pool->connects[i].in_use = true;
      pool->available--;
      pthread_mutex_unlock(&(pool->mutex));
      printf("Connection acquired\n");
      return pool->connects[i].connection;
    }
  }
  pthread_mutex_unlock(&(pool->mutex));
  return NULL; // No available connections
}

void release_connection(MYSQL *connection)
{
  pthread_mutex_lock(&(pool->mutex));
  for (int i = 0; i < MAX_POOL_SIZE; ++i)
  {
    if (pool->connects[i].connection == connection)
    {
      pool->connects[i].in_use = false;
      pool->available++;
      pthread_cond_signal(&(pool->cond));
      break;
    }
  }
  pthread_mutex_unlock(&(pool->mutex));
  printf("Connection released\n");
}

void cleanup_pool()
{
  pthread_mutex_lock(&(pool->mutex));
  while (pool->available != MAX_POOL_SIZE)
  {
    pthread_cond_wait(&(pool->cond), &(pool->mutex));
  }
  for (int i = 0; i < MAX_POOL_SIZE; ++i)
  {
    mysql_close(pool->connects[i].connection);
  }
  pthread_mutex_unlock(&(pool->mutex));
  pthread_mutex_destroy(&(pool->mutex));
  pthread_cond_destroy(&(pool->cond));
  free(pool);
}

int main()
{
  const char *host = "172.17.0.8";
  const char *user = "root";
  const char *password = "y341Gwv7KQPOU1TiCHfS";
  const char *db = "netdisk";
  unsigned int port = 3306;

  init_pool(host, user, password, db, port);
  int count = 0;
  while (count < 1)
  {
    printf("Iteration %d\n", count++);
    MYSQL *connection = get_connection();

    if (connection)
    {
      query(connection, "select * from user where id < %d", 5);
      query_set *result_set = get_result_set(connection, convert_to_user, sizeof(user_t));
      for (int i = 0; i < result_set->size; i++)
      {
        user_t user = ((user_t *)(result_set->data))[i];
        printf("id: %lu, name: %s, password: %s, delete_time: %lu\n", user.id, user.name, user.password, user.delete_time);
      }
      free_result_set(result_set);
      release_connection(connection);
    }
    printf("--------------------------------\n");
  }
  // 使用连接池中的连接进行数据库操作

  cleanup_pool();

  return 0;
}

/// @brief 查询辅助函数
/// @param conn 数据库连接
/// @param format 查询语句格式串
/// @param ... 可变长的插入变量
void query(MYSQL *conn, const char *format, ...)
{
  va_list args;
  va_start(args, format);
  char query[1024];
  vsnprintf(query, 1024, format, args);
  va_end(args);
  printf("Query: %s\n", query);
  if (mysql_query(conn, query))
  {
    fprintf(stderr, "Query failed: %s\n", mysql_error(conn));
  }
}

void convert_to_user(MYSQL_ROW *row, void *user)
{

  MYSQL_ROW row_curr = *row;
  user_t *u = (user_t *)user;
  u->id = strtoul(row_curr[0], NULL, 10);
  printf("user id: %lu\n", u->id);
  strcpy(u->name, row_curr[1]);
  printf("user name: %s\n", u->name);
  strcpy(u->password, row_curr[2]);
  printf("user password: %s\n", u->password);
  printf("delete time: %s\n", row_curr[3]);
  if (row_curr[3] == NULL)
  {
    return;
  }
  u->delete_time = strtoul(row_curr[3], NULL, 10);
}

query_set *get_result_set(MYSQL *conn, void (*convert)(MYSQL_ROW *row, void *target), unsigned int data_size)
{
  MYSQL_RES *result = mysql_store_result(conn);
  int num_rows = mysql_num_rows(result);
  query_set *set = (query_set *)malloc(sizeof(query_set));
  set->size = num_rows;
  set->data = malloc(num_rows * sizeof(user_t));
  int idx = 0;
  MYSQL_ROW row_curr;
  while ((row_curr = mysql_fetch_row(result)))
  {
    convert(&row_curr, (set->data) + idx * data_size);
    idx++;
  }
  mysql_free_result(result);
  return set;
}

void free_result_set(query_set *set)
{
  free(set->data);
  free(set);
}