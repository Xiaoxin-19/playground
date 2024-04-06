#ifndef QUEUE_H
#define QUEUE_H

typedef int E;
#include <stdbool.h>

typedef struct node{
  E data;
  struct node *next;
} Node;

typedef struct queue{
  Node *front;
  Node *rear;
  int size;
} Queue;

Queue *create_queue();
void enqueue(Queue *q, E data);
void dequeue(Queue *q);
E front(Queue *q);


#endif // QUEUE_H