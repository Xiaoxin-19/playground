#include <mysql/mysql.h>
#include <common_header.h>

void select_query(MYSQL *conn);
void update_query(MYSQL *conn);
void insert_query(MYSQL *conn);
void delete_query(MYSQL *conn);

int main(void)
{
  MYSQL conn;

  ERROR_CHECK(mysql_init(&conn), NULL, "mysql_init failed");
  mysql_set_character_set(&conn, "utf8");
  ERROR_CHECK(mysql_real_connect(&conn, "172.17.0.8", "root", "y341Gwv7KQPOU1TiCHfS", "homework_01", 0, NULL, 0),
              NULL, "mysql_real_connect failed");
  select_query(&conn);
  update_query(&conn);
  select_query(&conn);
  insert_query(&conn);
  select_query(&conn);
  delete_query(&conn);
  select_query(&conn);
  mysql_close(&conn);

  return 0;
}

void select_query(MYSQL *conn)
{
  MYSQL_ROW row;
  MYSQL_RES *res;
  ERROR_CHECK(mysql_query(conn, "SELECT * FROM user"), 1, "mysql_query failed");
  res = mysql_store_result(conn);
  while ((row = mysql_fetch_row(res)))
  {
    printf("%s %s %s\n", row[0], row[1], row[2]);
  }
  mysql_free_result(res);
}

void update_query(MYSQL *conn)
{
  ERROR_CHECK(mysql_query(conn, "update user set nickname = 'no_1' where id = 1"), -1, "mysql query failed");
  MYSQL_RES *res;
  res = mysql_store_result(conn);
  mysql_free_result(res);
}

void insert_query(MYSQL *conn)
{
  ERROR_CHECK(mysql_query(conn, "insert into user values (default,'Hwx', 'wx', '女', 70, 172,77, '2024-04-11', '甘肃省',20)"), -1, "mysql query failed");
  fprintf(stderr, "%s\n", mysql_error(conn));
  MYSQL_RES *res;
  res = mysql_store_result(conn);
  mysql_free_result(res);
}

void delete_query(MYSQL *conn)
{
  ERROR_CHECK(mysql_query(conn, "delete from user where name = 'Hwx'"), -1, "mysql query failed");
  MYSQL_RES *res;
  res = mysql_store_result(conn);
  mysql_free_result(res);
}