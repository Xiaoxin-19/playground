#include "queue.h"
#include "error.h"
#include <stdlib.h>

/// @brief 创建队列
/// @return Queue* 队列指针
Queue *create_queue()
{
  Queue *q = (Queue *)calloc(1, sizeof(Queue));
  ERROR_CHECK(q, NULL, "create queue error");
  return q;
}

/// @brief 入队
/// @param q 队列指针
/// @param data 数据
void enqueue(Queue *q, E data)
{
  Node *new_node = (Node *)calloc(1, sizeof(Node));
  ERROR_CHECK(new_node, NULL, "create node error");
  new_node->data = data;
  if (q->size == 0)
  {
    q->front = new_node;
    q->rear = new_node;
  }
  else
  {
    q->rear->next = new_node;
    q->rear = new_node;
  }
  q->size++;
}

/// @brief  出队
/// @param q
void dequeue(Queue *q)
{
  if (q->size == 0)
  {
    return;
  }
  Node *temp = q->front;
  q->front = q->front->next;
  free(temp);
  q->size--;
}

/// @brief 获取队列头元素
E front(Queue *q)
{
  if (q->size == 0)
  {
    return -1;
  }
  return q->front->data;
}