#include <stdio.h>
#include <stdlib.h>
#include "heap.h"

Heap *heap_init(){
	Heap *heap = malloc(sizeof(Heap));

	heap->num = 0;
	heap->size = 8;
	heap->array = malloc(sizeof(HNode) * heap->size);
	heap->array--;

	return heap;
}

void heap_add(Heap *heap, int val){
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
	}

	array[cur].value = val;
}

int heap_is_empty(Heap *heap){
	return heap->num == 0;
}

int heap_top_del(Heap *heap){
	int i, j, cur;
	HNode *array = heap->array;
	int val;

	if(heap_is_empty(heap)) return 0;

	cur = heap->num--;
 	val = array[cur].value;

	for(i = 2, j = 1; i < cur; j = i, i *= 2){
		if(i + 1 < cur && array[i + 1].value < array[i].value)
			i++;

		if(val <= array[i].value) break;

		array[j].value = array[i].value;
	}

	array[j].value = val;

	return 1;
}

void heap_up(Heap *heap, int n){
	int i, val;
	HNode *array = heap->array;

	if(n <= 1 || n > heap->num) return;

	val = array[n].value;

	for(; n > 1; n = i){
		i = n / 2;
		if(val >= array[i].value)
			break;
		array[n].value = array[i].value;
	}

	array[n].value = val;
}

void heap_down(Heap *heap, int n){
	int i, j, cur, val;
	cur = heap->num;
	HNode *array = heap->array;

	if(n < 0 || n >= cur) return;

	val = array[n].value;

	for(j = n, i = j * 2; i <= cur; j = i, i *= 2){
		if(i + 1 <= cur && array[i + 1].value < array[i].value)
			i++;
		if(val <= array[i].value) break;

		array[j].value = array[i].value;
	}

	array[j].value = val;
}

void heap_destroy(Heap *heap){
	free(heap->array + 1);
	free(heap);
}
/*
int main(){
	Heap *heap = heap_init();
	heap_add(heap, 3);
	heap_add(heap, 5);
	heap_add(heap, 7);
	heap_add(heap, 1);
	heap_add(heap, 8);
	heap_add(heap, 4);
	heap_add(heap, 2);
	heap_add(heap, 9);
	heap_add(heap, 0);

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
*/
