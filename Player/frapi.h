#ifndef FR_API_H
#define FR_API_H
#include "box-fixed.h"

typedef struct rb_node_t rb_node_t;
typedef struct thread_pool thread_pool;

typedef enum QueueType{
    QUEUE_SIZE,
    QUEUE_MSG
}QueueType;

struct MsgQueue{
    QueueType type;
    int len;
    int size;
    int cur_pos;
    int msg_len;
    char *msg;
};

typedef struct FRManager{
    BoxFixed *bf;
    GtkWidget *spice;
    int disp_width;
    int disp_height;
    //rb_node_t *plugs;
    struct thread_pool *pool;
    struct MsgQueue msg_queue;
    GHashTable *_plugs;
    GHashTable *hash_table;
}FRManager;

void fr_init(GtkWidget *widget, GtkWidget *spice);
void fr_process_msg(char *msg, int size);
void fr_disp_resize(int w, int h);

#endif
