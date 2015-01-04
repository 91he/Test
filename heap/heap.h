#ifndef HEAP_H
#define HEPA_H

#include <stdlib.h>

typedef struct HNode{
	unsigned int pos;
	int value;
}HNode;

typedef struct Heap{
	unsigned int num;
	unsigned int size;
	HNode *array;
}Heap;

Heap *heap_init();

void heap_add(Heap *heap, int val);

int heap_is_empty(Heap *heap);

int heap_top_del(Heap *heap);

void heap_up(Heap *heap, int n);

void heap_down(Heap *heap, int n);

void heap_destroy(Heap *heap);
#endif
