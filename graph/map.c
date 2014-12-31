#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "heap.h"

#define INF (1 << 24)
typedef struct Edge Edge;
typedef struct VNode VNode;
typedef struct Graph Graph;
typedef struct Path Path;

struct Edge{
	int dut;//权值
	Edge *next;
	VNode *node;
};

struct VNode{
	int id;//顶点的唯一标号,可为任意整数
	int adj;
	Edge *link;
};

struct Graph{
	unsigned int num;
	unsigned int size;
	VNode **array;
}graph;

struct Path{
	VNode *node;
	struct Path *next;
};

struct Graph *graph_init(){
	struct Graph *graph = malloc(sizeof(struct Graph));
	graph->num = 0;
	graph->size = 16;
	graph->array = malloc(sizeof(VNode*) * graph->size);
	return graph;
}

void graph_destroy(struct Graph *graph){
	int i;
	Edge *p = NULL, *q;

	for(i = 0; i < graph->num; i++){
		q = graph->array[i]->link;
		while(p = q){
			q = p->next;
			free(p);
		}
		free(graph->array[i]);
	}
	free(graph->array);
	graph->array = NULL;
}

VNode *vnode_new(struct Graph *graph, int id){
	int cur = graph->num++;
	VNode *ret = NULL;

	if(graph->num > graph->size){
		graph->size *= 2;
		graph->array = realloc(graph->array, sizeof(VNode*) * graph->size);
	}

	ret = malloc(sizeof(VNode));
	ret->id = id;
	ret->adj = cur;
	ret->link = NULL;

	return graph->array[cur] = ret;
}

VNode *find_node(struct Graph *graph, int id){
	int i;
	VNode *ret = NULL;
	
	for(i = 0; i < graph->num; i++){
		if(graph->array[i]->id == id){
			ret = graph->array[i];
			break;
		}
	}

	return ret;
}

void add_edge(struct Graph *graph, int from, int to, int dut){
	Edge *edge = malloc(sizeof(Edge));
	VNode *start, *end;

	if(!(start = find_node(graph, from)))
		start = vnode_new(graph, from);

	if(!(end = find_node(graph, to)))
		end = vnode_new(graph, to);

	edge->dut = dut;
	edge->next = start->link;
	edge->node = end;

	start->link = edge;
}

struct Path *road_min(struct Graph *graph, int from, int to){
	Heap *heap;
	HNode *array;
	Edge *edge, *tmp;
	struct Path *p, *q;
	int i, j, val, n = graph->num;
	int end = find_node(graph, to)->adj;
	int start = find_node(graph, from)->adj;
	struct VNode **path = malloc(sizeof(struct VNode*) * n);

	heap = heap_init();
	heap->num = n;
	array = heap->array;
	bzero(path, sizeof(struct VNode*) * n);

	for(i = 1; i <= n; i++){
		array[i].value = INF;
		array[i].vertex = i;
		array[i].pos = i;
	}

	array[start + 1].value = 0;
	heap_up(heap, start + 1);

	do{
		j = array[1].vertex - 1;

		if(j == end){
			break;
		}
		val = array[1].value;

		for(edge = graph->array[j]->link; edge; tmp = edge->next, edge = tmp){
			i = edge->node->adj + 1;
			if(array[array[i].pos].value > val + edge->dut){
				array[array[i].pos].value = val + edge->dut;
				heap_up(heap, array[i].pos);
				path[i - 1] = graph->array[j];
			}
		}
	}while(heap_top_del(heap));

	heap_destroy(heap);
	
	i = end;
	q = NULL;
	while(path[i]){
		j = path[i]->adj;
		p = malloc(sizeof(Path));
		p->node = path[i];
		p->next = q;
		q = p;
		i = j;
	}

	free(path);

	return q;
}

int main(){
	/*初始化*/
	Graph *graph = graph_init();

	/*struct Path *add_edge(struct Graph *graph, int from, int to, int dut);
	 *from&to:一条边的两个顶点。from为起点，to为终点。from和to可为任意整数.
	 *dut:边的权值,不可大于INF即16777216。
	 */
	add_edge(graph, 1, 2, 7);
	add_edge(graph, 2, 1, 7);
	add_edge(graph, 1, 3, 9);
	add_edge(graph, 3, 1, 9);
	add_edge(graph, 1, 6, 14);
	add_edge(graph, 6, 1, 14);
	add_edge(graph, 2, 3, 10);
	add_edge(graph, 3, 2, 10);
	add_edge(graph, 3, 6, 2);
	add_edge(graph, 6, 3, 2);
	add_edge(graph, 3, 4, 11);
	add_edge(graph, 4, 3, 11);
	add_edge(graph, 5, 6, 9);
	add_edge(graph, 6, 5, 9);
	add_edge(graph, 4, 5, 6);
	add_edge(graph, 5, 4, 6);
	add_edge(graph, 4, 7, 8);
	add_edge(graph, 7, 4, 8);
	add_edge(graph, 6, 7, 4);
	add_edge(graph, 7, 6, 4);
	add_edge(graph, 5, 7, 2);
	add_edge(graph, 7, 5, 2);

	/*
	 *road_min(struct Graph *grap, int from, int to);
	 *返回from--->to之间的最短路径————一个单向链表。
	 */
	int from = 1;
	int to = 5;

	Path *path = road_min(graph, from, to);
	Path *tmp;
	
	while(path){
		tmp = path->next;
		if(tmp)
			printf("%d->", path->node->id);
		else
			printf("%d->", path->node->id);
		path = tmp;
	}
	printf("%d\n", to);

	/*销毁*/
	graph_destroy(graph);

	return 0;
}
