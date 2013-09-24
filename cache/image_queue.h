#ifndef IMAGE_QUEUE_H
#define IMAGE_QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "image_cache.h"

#define NUM 100000
//int hash_index[NUM+1];

typedef struct IMAGE_QUEUE{
	int cur;
	int size;
	//int hash_index[NUM+1];
	int num[NUM];
}image_queue;

void init_imgq();
bool imgq_isfull();
bool imgq_isempty();
bool imgq_enque(int s);
int imgq_deque();
#endif
