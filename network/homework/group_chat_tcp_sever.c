#include <common_header.h>

#define MAX_LISTEN 10
#define MAX_EVENTS MAX_LISTEN + 2
#define MAX_BUF 1024

typedef struct node
{
  int fd;
  struct sockaddr_in addr;
  struct epoll_event ev;
  time_t last_heart;
  struct node *next;
  struct node *prev;
} Node;

typedef struct list
{
  Node *head;
  Node *tail;
  int size;
} List;

typedef struct
{
  char addr[17];
  int port;
} ClientInfo;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

List *list_init(void);
Node *list_insert(List *list, int fd, struct sockaddr_in *addr);
void list_delete(List *list, Node *node);
int init_server(int type, const char *addr, int port);
void server_start(int fd);
void add_new_client(List *list, int epoll_fd, int fd);
void disconnect_client(List *list, int epoll_fd, Node *node);
void broadcast_message(List *list, int fd, char *buf);
char *receive_message(List *list, Node *node, int epoll_fd);
ClientInfo *get_client_info(struct sockaddr_in *addr);
void *heart_check(void *arg);

int main(int argc, char *argv[])
{
  int server_fd;
  ARGS_CHECK(argc, 3);
  ERROR_CHECK((server_fd = init_server(SOCK_STREAM, argv[1], atoi(argv[2]))), -1, "init_server error");
  server_start(server_fd);
  close(server_fd);
  return 0;
}

List *list_init(void)
{
  List *list = (List *)malloc(sizeof(List));
  ERROR_CHECK(list, NULL, "malloc error");
  list->head = NULL;
  list->tail = NULL;
  list->size = 0;
  return list;
}

Node *list_insert(List *list, int fd, struct sockaddr_in *addr)
{
  pthread_mutex_lock(&mutex);
  Node *node = (Node *)malloc(sizeof(Node));
  ERROR_CHECK(node, NULL, "malloc error");
  node->fd = fd;
  node->addr = *addr;
  node->next = NULL;
  node->prev = list->tail;
  // 如果链表不为空
  if (list->tail)
  {
    list->tail->next = node;
  }
  list->tail = node;
  // 如果链表为空
  if (!list->head)
  {
    list->head = node;
  }
  list->size++;
  pthread_mutex_unlock(&mutex);
  return node;
}

void list_delete(List *list, Node *node)
{
  pthread_mutex_lock(&mutex);
  if (node->prev)
  {
    // 如果不是第一个节点
    node->prev->next = node->next;
  }
  else
  {
    // 如果是第一个节点
    list->head = node->next;
  }

  if (node->next)
  {
    // 如果不是最后一个节点
    node->next->prev = node->prev;
  }
  else
  {
    // 如果是最后一个节点
    list->tail = node->prev;
  }
  free(node);
  list->size--;
  pthread_mutex_unlock(&mutex);
}

// 初始化服务器 如果 addr 为 NULL 则绑定到所有地址
int init_server(int type, const char *addr, int port)
{
  int fd;
  struct sockaddr_in server_addr;

  // 建立套接字
  ERROR_CHECK((fd = socket(AF_INET, type, 0)), -1, "socket error");

  // 设置socket属性
  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  if (addr == NULL)
  {
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  }
  else
  {
    ERROR_CHECK(inet_aton(addr, &server_addr.sin_addr), 0, "inet_aton error");
  }
  server_addr.sin_port = htons(port);

  int reuse = 1;
  // 设置地址重用
  ERROR_CHECK(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)), -1, "setsockopt error");

  // 绑定地址
  ERROR_CHECK(bind(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)), -1, "bind error");

  // 如果是流式套接字或者顺序包套接字
  if (type == SOCK_STREAM || type == SOCK_SEQPACKET)
  {
    // 监听
    ERROR_CHECK(listen(fd, MAX_LISTEN), -1, "listen error");
  }
  printf("server: server start\n");
  return fd;
}

void server_start(int fd)
{
  // 初始化链表
  List *list = list_init();
  int epoll_fd;
  struct epoll_event evlist[MAX_EVENTS];
  struct epoll_event ev;
  pthread_t tid;

  // 创建心跳检测线程
  THREAD_ERROR_CHECK(pthread_create(&tid, NULL, heart_check, list), "pthread_create error");

  ERROR_CHECK((epoll_fd = epoll_create1(EPOLL_CLOEXEC)), -1, "epoll_create error");
  ev.events = EPOLLIN;
  ev.data.fd = fd;
  epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);
  while (true)
  {
    int ret = epoll_wait(epoll_fd, evlist, sizeof(evlist) / sizeof(struct epoll_event), -1);
    for (int i = 0; i < ret; i++)
    {
      if (evlist[i].data.fd == fd)
      {
        // 新的客户端连接
        add_new_client(list, epoll_fd, fd);
      }
      else
      {
        // 接收消息
        Node *node = (Node *)evlist[i].data.ptr;
        char *msg = receive_message(list, node, epoll_fd);

        // 广播消息
        if (msg)
        {
          broadcast_message(list, node->fd, msg);
          free(msg);
        }
      }
    }
  }

  close(epoll_fd);
  free(list);
}

/// @brief 添加新的客户端, 并将其加入到 epoll 监听中和链表中，同时设置其回调函数,其中 epoll_event 和 node 都是动态分配的内存,需要在外部释放
/// @param list 连接客户端链表
/// @param epoll_fd epoll 文件描述符
/// @param fd 服务器 socket 文件描述符
void add_new_client(List *list, int epoll_fd, int fd)
{
  int new_fd;
  struct sockaddr_in *client_addr = malloc(sizeof(struct sockaddr_in));
  ERROR_CHECK(client_addr, NULL, "malloc error");
  socklen_t len = sizeof(struct sockaddr_in);
  ERROR_CHECK((new_fd = accept(fd, (SA *)client_addr, &len)), -1, "accept error");
  Node *node = list_insert(list, new_fd, client_addr);

  // 设置心跳时间
  node->last_heart = time(NULL);

  // 将ev保存在node中，防止内存泄漏，会随着node的删除一起销毁，同时也方便后续的操作
  node->ev.events = EPOLLIN;
  node->ev.data.ptr = &(*node);
  ERROR_CHECK(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_fd, &(node->ev)), -1, "epoll_ctl error");

  ClientInfo *info = get_client_info(client_addr);
  printf("server: client[%s:%d] connected\n", info->addr, info->port);
  char temp_buf[MAX_BUF] = {0};
  sprintf(temp_buf, "[Server]: Wecome to Chat room, current online: %d \n", list->size);

  send(new_fd, temp_buf, strlen(temp_buf), 0);
  free(client_addr);
  free(info);
}

/// @brief 断开客户端连接
/// @param list 连接客户端链表
/// @param epoll_fd epoll 文件描述符
/// @param node 当前客户端节点
void disconnect_client(List *list, int epoll_fd, Node *node)
{
  int fd = node->fd;
  epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
  close(fd);
  ClientInfo *info = get_client_info(&node->addr);
  printf("server: client[%s:%d] disconnected\n", info->addr, info->port);
  list_delete(list, node);
}

/// @brief 广播消息
/// @param list 连接客户端链表
/// @param fd 发送消息的客户端文件描述符
/// @param buf 消息内容
void broadcast_message(List *list, int fd, char *buf)
{
  pthread_mutex_lock(&mutex);
  Node *node = list->head;
  char temp_buf[MAX_BUF];
  while (node)
  {
    if (node->fd != fd)
    {
      bzero(temp_buf, MAX_BUF);
      // 获取客户端地址和端口
      ClientInfo *info = get_client_info(&node->addr);

      // 设置消息格式
      sprintf(temp_buf, "[%s:%d]: %s", info->addr, info->port, buf);
      free(info);

      // 发送消息
      int n = send(node->fd, temp_buf, strlen(temp_buf), 0);
      if (n < 0)
      {
        perror("send error");
      }
    }
    node = node->next;
  }
  pthread_mutex_unlock(&mutex);
}

/// @brief  获取客户端信息 动态分配内存,需要在外部释放
/// @param sockaddr_in 客户端地址
/// @return ClientInfo*
ClientInfo *get_client_info(struct sockaddr_in *addr)
{
  ClientInfo *info = (ClientInfo *)malloc(sizeof(ClientInfo));
  ERROR_CHECK(info, NULL, "malloc error");
  ERROR_CHECK(inet_ntop(AF_INET, &(addr->sin_addr), info->addr, sizeof(info->addr)), NULL, "inet_ntop error");
  info->port = ntohs(addr->sin_port);
  return info;
}

/// @brief 接收消息,buf 为动态分配的内存,需要在外部释放
/// @param list 连接客户端链表
/// @param node 当前客户端节点
/// @param epoll_fd epoll 文件描述符
/// @return char* 返回接收到的消息
char *receive_message(List *list, Node *node, int epoll_fd)
{
  int n;
  int fd = node->fd;
  char *buf = calloc(MAX_BUF, sizeof(char));
  // 设置心跳时间
  node->last_heart = time(NULL);

  // 接收数据 使用while循环接收数据，防止丢失分包
  n = recv(fd, buf, MAX_BUF, 0);
  if (n == 0)
  {
    // 客户端关闭
    disconnect_client(list, epoll_fd, node);
    goto errout;
  }
  else if (n < 0)
  {
    perror("recv error");
    goto errout;
  }
  else
  {
    // 返回消息
    return buf;
  }

errout:
  return NULL;
}

/// @brief 心跳检测,如果超过 20s 没有收到消息，则断开连接
/// @param arg
/// @return
void *heart_check(void *arg)
{
  List *list = (List *)arg;
  Node *node, *temp;
  while (true)
  {
    node = list->head;
    while (node)
    {
      // 如果超过 20s 没有收到心跳包，则断开连接
      if (time(NULL) - node->last_heart > 20)
      {
        char buf[1024] = "server: heart check failed (no response for more than 20 seconds) , disconnect\n";
        ClientInfo *info = get_client_info(&node->addr);
        printf("server: client[%s:%d] heart check failed(No response for more than 20 seconds)\n", info->addr, info->port);
        free(info);
        send(node->fd, buf, strlen(buf), 0);
        temp = node->next;
        disconnect_client(list, -1, node);
      }
      if (temp == NULL)
        break;
      node = temp;
    }
    sleep(1);
  }
  return NULL;
}