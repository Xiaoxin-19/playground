#include "queue.h"
#include "error.h"
#include <stdlib.h>

Queue *create_queue()
{
  Queue *q = (Queue *)calloc(1, sizeof(Queue));
  ERROR_CHECK(q, NULL, "create queue error");
  return q;
}

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

E front(Queue *q)
{
  if (q->size == 0)
  {
    return -1;
  }
  return q->front->data;
}