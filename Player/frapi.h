#ifndef FR_API_H
#define FR_API_H
#include "box-fixed.h"

struct Rect{
    int x, y, w, h;
};

struct Rgn{
    int num;
    struct Rect rects[0];
};

typedef struct FRPlugin{
    int id;
    int x, y, w, h;
    NPP_t instance;
    NPWindow *np_win;
    GtkWidget *socket;
    cairo_t *cr;
    GdkPixmap *pixmap;
    struct Rgn *rgn;
}FRPlugin;

typedef struct FRManager{
    FRPlugin *plugs;
    BoxFixed *bf;
    struct thread_pool *pool;
    int disp_width;
    int disp_height;
}FRManager;

void fr_init(GtkWidget *widget);

void fr_process_msg(char *msg, int size);

void fr_disp_resize(int w, int h);

#endif
