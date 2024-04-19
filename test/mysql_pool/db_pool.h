#include <mysql/mysql.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_POOL_SIZE 4

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
Db_pool *init_pool(const char *host, const char *user, const char *password, const char *db, unsigned int port)
{
  Db_pool *pool = (Db_pool *)malloc(sizeof(Db_pool));

  pthread_cond_init(&(pool->cond), NULL);
  pthread_mutex_init(&pool->mutex, NULL);
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
  pthread_mutex_lock(&pool->mutex);
  while (pool->available == 0)
  {
    pthread_cond_wait(&pool->cond, &pool->mutex);
  }
  for (int i = 0; i < MAX_POOL_SIZE; ++i)
  {
    if (!pool->connects[i].in_use)
    {
      pool->connects[i].in_use = true;
      pthread_mutex_unlock(&pool->mutex);
      printf("Connection acquired\n");
      return pool->connects[i].connection;
    }
  }
  pthread_mutex_unlock(&pool->mutex);
  return NULL; // No available connections
}

void release_connection(MYSQL *connection)
{
  pthread_mutex_lock(&pool->mutex);
  for (int i = 0; i < MAX_POOL_SIZE; ++i)
  {
    if (pool->connects[i].connection == connection)
    {
      pool->connects[i].in_use = false;
      pthread_cond_signal(&pool->cond);
      break;
    }
  }
  pthread_mutex_unlock(&pool->mutex);
  printf("Connection released\n");
}

void cleanup_pool()
{
  for (int i = 0; i < MAX_POOL_SIZE; ++i)
  {
    mysql_close(pool->connects[i].connection);
  }
  pthread_mutex_destroy(&pool->mutex);
  pthread_cond_destroy(&pool->cond);
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

  // 使用连接池中的连接进行数据库操作
  MYSQL *connection = get_connection();
  if (connection)
  {
    MYSQL_RES *res;
    res = mysql_query(connection, "show tables");
    int num_fields = mysql_num_fields(res);
    if (res)
    {
      MYSQL_ROW row;

      while ((row = mysql_fetch_row(res)))
      {
        for (int i = 0; i < num_fields; i++)
        {
          if (i == 0)
          {
            printf("%s", row[i] ? row[i] : "NULL");
          }
          else
          {
            printf(" | %s", row[i] ? row[i] : "NULL");
          }
        }
        printf("\n");
      }
      mysql_free_result(res);
    }
    release_connection(connection);
  }

  cleanup_pool();

  return 0;
}
