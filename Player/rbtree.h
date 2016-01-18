#ifndef LEVI_RBTREE_H
#define LEVI_RBTREE_H
#include "frapi.h"

typedef int key_t;
typedef struct FRPlugin *data_t;

typedef enum color_t{
    RED,
    BLACK
}color_t;
typedef struct rb_node_t{
	data_t data;
	key_t key;
	color_t color;
	struct rb_node_t *parent;
	struct rb_node_t *left;
	struct rb_node_t *right;
}rb_node_t;

rb_node_t* rb_insert(key_t key, data_t data, rb_node_t* root);
rb_node_t* rb_search(key_t key, rb_node_t* root);
rb_node_t* rb_erase(key_t key, rb_node_t *root);
void *rb_travel(rb_node_t *node, void (*func)(void *));

#endif
