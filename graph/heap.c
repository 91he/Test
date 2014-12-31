#include <stdio.h>
#include <stdlib.h>
#include "heap.h"

Heap *heap_init(){
	Heap *heap = malloc(sizeof(Heap));

	heap->num = 0;
	heap->size = 16;
	heap->array = malloc(sizeof(HNode) * heap->size);
	heap->array--;

	return heap;
}

void heap_add(Heap *heap, int val, int vertex){
	int i;
	int cur = ++heap->num;
	HNode *array = heap->array;

	if(cur > heap->size){
		heap->size *= 2;
		array = realloc(array + 1, heap->size * sizeof(HNode));
		heap->array = --array;
	}
	
	for(; cur > 1; cur = i){
		i = cur / 2;
		if(val >= array[i].value)
			break;
		array[cur].value = array[i].value;
		array[cur].vertex = array[i].vertex;
		array[array[cur].vertex].pos = cur;
	}

	array[cur].value = val;
	array[cur].vertex = vertex;
	array[array[cur].vertex].pos = cur;
}

int heap_is_empty(Heap *heap){
	return heap->num == 0;
}

int heap_top_del(Heap *heap){
	int i, j, cur;
	HNode *array = heap->array;
	HNode node;

	if(heap_is_empty(heap)) return 0;

	cur = heap->num--;
	node = array[cur];

	for(i = 2, j = 1; i < cur; j = i, i *= 2){
		if(i + 1 < cur && array[i + 1].value < array[i].value)
			i++;

		if(node.value <= array[i].value) break;

		array[j].value = array[i].value;
		array[j].vertex = array[i].vertex;
		array[array[j].vertex].pos = j;
	}

	array[j].value = node.value;
	array[j].vertex = node.vertex;
	array[array[j].vertex].pos = j;

	return 1;
}

void heap_up(Heap *heap, int n){
	HNode node;
	int i, val, pos;
	HNode *array = heap->array;

	if(n <= 1 || n > heap->num) return;

	node = array[n];

	for(; n > 1; n = i){
		i = n / 2;
		if(node.value >= array[i].value)
			break;
		array[n].value = array[i].value;
		array[n].vertex = array[i].vertex;
		array[array[n].vertex].pos = n;
	}

	array[n].value = node.value;
	array[n].vertex = node.vertex;
	array[array[n].vertex].pos = n;
}

void heap_down(Heap *heap, int n){
	HNode node;
	int i, j, cur, val, pos;
	HNode *array = heap->array;

	cur = heap->num;
	if(n < 0 || n >= cur) return;

	node = array[n];

	for(j = n, i = j * 2; i <= cur; j = i, i *= 2){
		if(i + 1 <= cur && array[i + 1].value < array[i].value)
			i++;
		if(node.value <= array[i].value) break;

		array[j].value = array[i].value;
		array[j].vertex = array[i].vertex;
		array[array[j].vertex].pos = j;
	}

	array[j].value = node.value;
	array[j].vertex = node.vertex;
	array[array[j].vertex].pos = j;
}

void heap_destroy(Heap *heap){
	free(heap->array + 1);
	free(heap);
}
#if 0
int main(){
	Heap *heap = heap_init();
	heap_add(heap, 3, 1);
	heap_add(heap, 5, 2);
	heap_add(heap, 7, 3);
	heap_add(heap, 1, 4);
	heap_add(heap, 8, 5);
	heap_add(heap, 4, 6);
	heap_add(heap, 2, 7);
	heap_add(heap, 9, 8);
	heap_add(heap, 0, 9);

	int i;
	for(i = 1; i <= heap->num; i++){
		printf("%d ", heap->array[i].value);
	}
	printf("\n");

	heap->array[4].value -= 3;
	heap_up(heap, 4);

	for(i = 1; i <= heap->num; i++){
		printf("%d ", heap->array[i].value);
	}
	printf("\n");

	while(!heap_is_empty(heap)){
		printf("%d ", heap->array[1].value);
		heap_top_del(heap);
	}
	printf("\n");
	heap_destroy(heap);
	return 0;
}
#endif
