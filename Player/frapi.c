#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#include "frapi.h"
//#include "rbtree.h"
#include "curl_url.h"
#include "npp_funcs.h"
#include "npn_funcs.h"

enum MsgType{
    PLUG_INIT,
    PLUG_CORD,
    PLUG_SIZE,
    PLUG_RGN,
    PLUG_HIDE,
    PLUG_DESTROY
};

struct Rect{
    int x, y, w, h;
};

struct Cord{
    int x, y;
};

struct WNH{
    int w, h;
};

struct Rgn{
    int num;
    struct Rect rects[0];
};

typedef struct FRPlugin{
    int id;
    int x, y, w, h;
    char *cookie;
    NPP_t instance;
    NPWindow *np_win;
    GtkWidget *socket;
    cairo_t *cr;
    GdkPixmap *pixmap;
    struct Rgn *rgn;
}FRPlugin;

FRManager g_FRManager;
#ifndef MAKE_SO
GtkWidget *main_window = NULL;
#endif
static FRPlugin *plug_new(int id);
static void plug_play(struct FRPlugin *plug, char *url, char *flashvars);
static void plug_move(struct FRPlugin *plug, int x, int y);
static void plug_resize(struct FRPlugin *plug, int w, int h);
static void plug_mask(struct FRPlugin *plug, struct Rgn *rgn);
static void plug_destroy(struct FRPlugin *plug);

//static void pixmap_recreate(void *data){
static void pixmap_recreate(gpointer a, gpointer data, gpointer b){
    int w, h;
    FRPlugin *plug = (FRPlugin*)data;

    cairo_destroy(plug->cr);
    g_object_unref(plug->pixmap);

    plug->pixmap = gdk_pixmap_new(NULL, g_FRManager.disp_width, g_FRManager.disp_height, 1);
    plug->cr = gdk_cairo_create(GDK_DRAWABLE(plug->pixmap));

    cairo_set_operator(plug->cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba(plug->cr, 0, 0, 0, 0);
    gdk_pixmap_get_size(plug->pixmap, &w, &h);
    cairo_rectangle(plug->cr, 0, 0, w, h);
    cairo_fill(plug->cr);
}

gboolean travel_callback(gpointer data){
    //rb_travel(g_FRManager.plugs, pixmap_recreate);
    g_hash_table_foreach(g_FRManager._plugs, pixmap_recreate, NULL);

    return FALSE;
}

gboolean configure_callback(GtkWidget *widget, GdkEvent *event, gpointer data){
    g_FRManager.disp_width = event->configure.width;
    g_FRManager.disp_height = event->configure.height;

    //rb_travel(g_FRManager.plugs, pixmap_recreate);
    g_idle_add(travel_callback, NULL);

    return FALSE;
}

void fr_init(GtkWidget *widget, GtkWidget *spice){
    constructor_npfuncs();

    gtk_widget_add_events(widget, GDK_STRUCTURE_MASK);
    //g_signal_connect(widget, "configure-event", (GCallback)configure_callback, NULL);
    g_signal_connect(spice, "configure-event", (GCallback)configure_callback, NULL);

    g_FRManager.bf = BOX_FIXED(widget);
    g_FRManager.spice = spice;
    g_FRManager.pool = pool_init(4);

    g_FRManager.disp_width = 1366;
    g_FRManager.disp_height = 768;
    //g_FRManager.plugs = NULL;
    g_FRManager.msg_queue.type = QUEUE_SIZE;
    g_FRManager.msg_queue.len = 0;
    g_FRManager.msg_queue.size = 4096;
    g_FRManager.msg_queue.cur_pos = 0;
    g_FRManager.msg_queue.msg = malloc(g_FRManager.msg_queue.size);
    g_FRManager._plugs = g_hash_table_new(g_direct_hash, g_direct_equal);
    g_FRManager.hash_table = g_hash_table_new(g_direct_hash, g_direct_equal);
}

gboolean plug_process(gpointer data){
    char *msg;
    char *tmp = (char*)data;
    char type = tmp[0];
    int id = *(int*)&tmp[1];
    //rb_node_t *node = NULL;
    gpointer _node = NULL;

    msg = &tmp[5];
    fprintf(stderr, "MsgType: ========== %d, %d\n", type, id);
    switch(type){
        case PLUG_INIT:
            {
                int len = strlen(msg) + 1;
                //if(node = rb_search(id, g_FRManager.plugs)){
                if(_node = g_hash_table_lookup(g_FRManager._plugs, (gpointer)(intptr_t)id)){
                    //plug_play(node->data, msg, msg + len);
                    FRPlugin *plug = (FRPlugin*)_node;
                    plug_play(plug, msg, msg + len);
                    len += strlen(msg + len) + 1;
                    //node->data->cookie = strdup(msg + len);
                    plug->cookie = strdup(msg + len);
                    g_hash_table_insert(g_FRManager.hash_table, (gpointer)&plug->instance, plug->cookie);
                }else{
                    FRPlugin *plug = plug_new(id);
                    plug_play(plug, msg, msg + len);
                    len += strlen(msg + len) + 1;
                    plug->cookie = strdup(msg + len);
                    g_hash_table_insert(g_FRManager.hash_table, (gpointer)&plug->instance, plug->cookie);
                    //g_FRManager.plugs = rb_insert(id, plug, g_FRManager.plugs);
                    g_hash_table_insert(g_FRManager._plugs, (gpointer)(intptr_t)id, plug);
                    //fprintf(stderr, "Flash has been inited!\n");
                }
            }
            break;
        case PLUG_CORD:
            {
                struct Cord *cord = (struct Cord*)msg;
                //if(node = rb_search(id, g_FRManager.plugs)){
                if(_node = g_hash_table_lookup(g_FRManager._plugs, (gpointer)(intptr_t)id)){
                    //FRPlugin *plug = node->data;
                    FRPlugin *plug = _node;
                    if(plug->socket){
                        plug_move(plug, cord->x, cord->y);
                    }else{
                        plug->x = cord->x;
                        plug->y = cord->y;
                    }
                }else{
                    FRPlugin *plug = plug_new(id);
                    plug->x = cord->x;
                    plug->y = cord->y;
                    //g_FRManager.plugs = rb_insert(id, plug, g_FRManager.plugs);
                    g_hash_table_insert(g_FRManager._plugs, (gpointer)(intptr_t)id, plug);
                    //fprintf(stderr, "Flash hasn't been inited!\n");
                }
            }
            break;
        case PLUG_SIZE:
            {
                struct WNH *wnh = (struct WNH*)msg;
                //if(node = rb_search(id, g_FRManager.plugs)){
                if(_node = g_hash_table_lookup(g_FRManager._plugs, (gpointer)(intptr_t)id)){
                    //FRPlugin *plug = node->data;
                    FRPlugin *plug = _node;
                    if(plug->socket){
                        plug_resize(plug, wnh->w, wnh->h);
                    }else{
                        plug->w = wnh->w;
                        plug->h = wnh->h;
                    }
                }else{
                    FRPlugin *plug = plug_new(id);
                    plug->w = wnh->w;
                    plug->h = wnh->h;
                    //g_FRManager.plugs = rb_insert(id, plug, g_FRManager.plugs);
                    g_hash_table_insert(g_FRManager._plugs, (gpointer)(intptr_t)id, plug);
                    //fprintf(stderr, "Flash hasn't been inited!\n");
                }
            }
            break;
        case PLUG_RGN:
            {
                struct Rgn *rgn = (struct Rgn*)msg;
                //if(node = rb_search(id, g_FRManager.plugs)){
                if(_node = g_hash_table_lookup(g_FRManager._plugs, (gpointer)(intptr_t)id)){
                    //FRPlugin *plug = node->data;
                    FRPlugin *plug = _node;
                    if(plug->socket){
                        plug_mask(plug, rgn);
                    }else{
                        if(plug->rgn) free(plug->rgn);
                        plug->rgn = malloc(sizeof(*rgn) + rgn->num * sizeof(struct Rect));
                        memcpy(plug->rgn, rgn, sizeof(*rgn) + rgn->num * sizeof(struct Rect));
                    }
                }else{
                    FRPlugin *plug = plug_new(id);
                    plug->rgn = malloc(sizeof(*rgn) + rgn->num * sizeof(struct Rect));
                    memcpy(plug->rgn, rgn, sizeof(*rgn) + rgn->num * sizeof(struct Rect));
                    //g_FRManager.plugs = rb_insert(id, plug, g_FRManager.plugs);
                    g_hash_table_insert(g_FRManager._plugs, (gpointer)(intptr_t)id, plug);
                    //fprintf(stderr, "Flash hasn't been inited!\n");
                }
            }
            break;
        case PLUG_HIDE:
            //if(node = rb_search(id, g_FRManager.plugs)){
            if(_node = g_hash_table_lookup(g_FRManager._plugs, (gpointer)(intptr_t)id)){
                //FRPlugin *plug = node->data;
                FRPlugin *plug = _node;
                if(plug->socket){
                    gtk_widget_hide(plug->socket);
                }
            }else{
                FRPlugin *plug = plug_new(id);
                //g_FRManager.plugs = rb_insert(id, plug, g_FRManager.plugs);
                g_hash_table_insert(g_FRManager._plugs, (gpointer)(intptr_t)id, plug);
                //fprintf(stderr, "Flash hasn't been inited!\n");
            }
            break;
        case PLUG_DESTROY:
            //if(node = rb_search(id, g_FRManager.plugs)){
            if(_node = g_hash_table_lookup(g_FRManager._plugs, (gpointer)(intptr_t)id)){
                //plug_destroy(node->data);
                plug_destroy(_node);
                //g_FRManager.plugs = rb_erase(id, g_FRManager.plugs);
                g_hash_table_remove(g_FRManager._plugs, (gpointer)(intptr_t)id);
            }else{
                fprintf(stderr, "Flash hasn't been inited!\n");
            }
            break;
        default:
            fprintf(stderr, "What the fuck is going on!\n");
            break;
    }
    free(tmp);

    return FALSE;
}

void fr_process_msg(char *msg, int size){
    QueueType type = g_FRManager.msg_queue.type;
    int len = g_FRManager.msg_queue.len;
    int msg_len = g_FRManager.msg_queue.msg_len;
    int queue_size = g_FRManager.msg_queue.size;
    int cur_pos = g_FRManager.msg_queue.cur_pos;
    char *queue_msg= g_FRManager.msg_queue.msg;

    //TODO
    memcpy(queue_msg + cur_pos + len, msg, size);
    len += size;
    fprintf(stderr, "%d ============= %d\n", size, msg[4]);
    if(type == QUEUE_SIZE){
        if(len > 4){
            type = QUEUE_MSG;
            len -= 4;
            cur_pos += 4;
            msg_len = *(int*)queue_msg;
            if(len >= msg_len){
                char *pmsg = malloc(msg_len);
                memcpy(pmsg, &queue_msg[cur_pos], msg_len);
                fprintf(stderr, "%d ----------- %d\n", msg_len, pmsg[0]);
                g_idle_add(plug_process, pmsg);

                type = QUEUE_SIZE;
                len -= msg_len;
                cur_pos += msg_len;
                memcpy(queue_msg, queue_msg + cur_pos, len);
                cur_pos = 0;
            }
        }
    }else{
        if(len >= msg_len){
            char *pmsg = malloc(msg_len);
            memcpy(pmsg, &queue_msg[cur_pos], msg_len);
            fprintf(stderr, "%d +++++++++++++ %d\n", msg_len, pmsg[0]);
            g_idle_add(plug_process, pmsg);

            type = QUEUE_SIZE;
            len -= msg_len;
            cur_pos += msg_len;
            memcpy(queue_msg, queue_msg + cur_pos, len);
            cur_pos = 0;
        }
    }

    g_FRManager.msg_queue.type = type ;
    g_FRManager.msg_queue.len = len;
    g_FRManager.msg_queue.msg_len = msg_len;
    g_FRManager.msg_queue.size = queue_size;
    g_FRManager.msg_queue.cur_pos = cur_pos;
    g_FRManager.msg_queue.msg = queue_msg;
}

static void npwindow_set(NPP instance, NPWindow *np_win, int w, int h){
    np_win->width = w;
    np_win->height = h;

    My_NPP_SetWindow(instance, np_win);
}

static void set_rgn(FRPlugin *plug, struct Rgn *rgn){
    int i;
    cairo_t *cr = plug->cr;
    int w, h;

    if(rgn && rgn->num >= 0){
        for(i = 0; i < rgn->num;){
            cairo_rectangle(cr, rgn->rects[i].x, rgn->rects[i].y, rgn->rects[i].w - rgn->rects[i].x, rgn->rects[i].h - rgn->rects[i].y);
            if(++i < rgn->num)
                cairo_new_sub_path(cr);
        }
        cairo_set_source_rgba(cr, 0, 0, 0, 1);
    }else{
        cairo_set_source_rgba(cr, 0, 0, 0, 0);
        gdk_pixmap_get_size(plug->pixmap, &w, &h);
        cairo_rectangle(cr, 0, 0, w, h);
    }
    cairo_fill(cr);
}

static gboolean scroll_callback(GtkWidget *widget, GdkEvent *event, gpointer data){
    static gboolean bret;
    static GdkEvent tmp;

    tmp= *event;
    g_signal_emit_by_name(g_FRManager.spice, "scroll-event", &tmp, &bret);

    return true;
}

#ifndef MAKE_SO
static gboolean scroll_callback2(GtkWidget *widget, GdkEvent *event, gpointer data){
    printf("++++++++++++%d\n", event->scroll.direction);

    return true;
}
#endif
static void npwindow_init(FRPlugin *plug, BoxFixed *bf){ //Must ensure widget has been anchored.(Widget must be a child of window.)
    NPWindow *npwindow;
    GtkWidget *socket;
    NPSetWindowCallbackStruct *ws_info;

    plug->socket = socket = gtk_socket_new();
    gtk_widget_add_events(socket, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(socket, "scroll-event", (GCallback)scroll_callback, NULL);
    box_fixed_put(bf, socket, plug->x, plug->y);//Must do this.
    gtk_widget_realize(socket);
    //gtk_widget_show(socket); //Seems there is no use doing this.

    if(!plug->np_win){
        plug->np_win = npwindow = (NPWindow*)malloc(sizeof(NPWindow));
        ws_info = npwindow->ws_info = (NPSetWindowCallbackStruct*)malloc(sizeof(NPSetWindowCallbackStruct));
    }else{
        npwindow = plug->np_win;
        ws_info = npwindow->ws_info;
    }
    npwindow->window = GINT_TO_POINTER(gtk_socket_get_id(GTK_SOCKET(socket)));

    GdkWindow *gwin = gtk_widget_get_window(socket);
    ws_info->type = NP_SETWINDOW;
    ws_info->display = gdk_x11_display_get_xdisplay(gdk_window_get_display(gwin));
    ws_info->visual = gdk_x11_visual_get_xvisual(gdk_window_get_visual(gwin));
    ws_info->colormap = XDefaultColormapOfScreen(XDefaultScreenOfDisplay(ws_info->display));
    ws_info->depth = gdk_visual_get_depth(gdk_window_get_visual(gwin));

    npwindow->type = NPWindowTypeWindow;
    npwindow->x = 0;
    npwindow->y = 0;

    Cursor cursor = XcursorLibraryLoadCursor(ws_info->display, "arrow");
    XDefineCursor(ws_info->display, gtk_socket_get_id(GTK_SOCKET(socket)), cursor);
}

static void plug_mask(struct FRPlugin *plug, struct Rgn *rgn){
    set_rgn(plug, NULL);
    set_rgn(plug, rgn);

    if(plug->rgn != rgn && rgn){
        if(plug->rgn) free(plug->rgn);
        plug->rgn = malloc(sizeof(struct Rgn) + rgn->num * sizeof(struct Rect));
        memcpy(plug->rgn, rgn, sizeof(struct Rgn) + rgn->num * sizeof(struct Rect));
    }

    gtk_widget_shape_combine_mask(plug->socket, plug->pixmap, 0, 0);
    gtk_widget_show(plug->socket);
}

static void plug_resize(struct FRPlugin *plug, int w, int h){
    plug->w = w;
    plug->h = h;

    npwindow_init(plug, g_FRManager.bf);
    npwindow_set(&plug->instance, plug->np_win, w, h);
    plug_mask(plug, plug->rgn);
}

static void plug_move(struct FRPlugin *plug, int x, int y){
    plug->x = x;
    plug->y = y;

    box_fixed_move(g_FRManager.bf, plug->socket, x, y);
}

static FRPlugin *plug_new(int id){
    int w, h;
    struct FRPlugin *plug;

    plug = malloc(sizeof(struct FRPlugin));
    //g_hash_table_add(g_FRManager.hash_table, (gpointer)&plug->instance);

    plug->id = id;
    plug->x = plug->y = 0;
    plug->w = plug->h = 0;
    memset(&plug->instance, 0, sizeof(plug->instance));
    //npwindow_init(plug, g_FRManager.bf);
    plug->pixmap = gdk_pixmap_new(NULL, g_FRManager.disp_width, g_FRManager.disp_height, 1);
    plug->cr = gdk_cairo_create(GDK_DRAWABLE(plug->pixmap));
    plug->rgn = NULL;
    plug->np_win = NULL;
    plug->socket = NULL;
    plug->cookie = NULL;

    cairo_set_operator(plug->cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba(plug->cr, 0, 0, 0, 0);
    gdk_pixmap_get_size(plug->pixmap, &w, &h);
    cairo_rectangle(plug->cr, 0, 0, w, h);
    cairo_fill(plug->cr);

    return plug;
}

static void plug_play(struct FRPlugin *plug, char *url, char *flashvars){
    NPObject object;
    char *xargv[] = {"allowFullScreen", "allowScriptAccess", "swLiveConnect", "quality", "menu", "flashvars"};
    char *xargm[] = {"true", "always", "true", "high", "false", flashvars};

    My_NPP_New("application/x-shockwave-flash", &plug->instance, NP_EMBED, sizeof(xargv)/sizeof(xargv[0]), xargv, xargm, 0);
    My_NPP_GetValue(&plug->instance, NPPVpluginScriptableNPObject, &object);

    fprintf(stderr, "%s, %s\n", url, flashvars);
    My_NPN_GetURLNotify(&plug->instance, url, NULL, NULL);
    if(plug->w * plug->h){
        if(!plug->rgn){
            struct Rgn *rgn = malloc(sizeof(struct Rgn) + sizeof(struct Rect));
            rgn->num = 1;
            rgn->rects[0].x = 0;
            rgn->rects[0].y = 0;
            rgn->rects[0].w = plug->w;
            rgn->rects[0].h = plug->h;
            plug->rgn = rgn;
        }
        plug_resize(plug, plug->w, plug->h);
    }
}

static void plug_destroy(struct FRPlugin *plug){
    My_NPP_Destroy(&plug->instance, NULL);
    g_hash_table_remove(g_FRManager.hash_table, (gpointer)&plug->instance);
    if(plug->np_win){
        free(plug->np_win->ws_info);
        free(plug->np_win);
    }
    if(plug->rgn) free(plug->rgn);
    cairo_destroy(plug->cr);
    g_object_unref(plug->pixmap);
    if(plug->cookie) free(plug->cookie);
    free(plug);
}

#if 0
static NPWindow *npwindow_construct(GtkWidget *widget, uint32_t width, uint32_t height){                                                             
    NPWindow *npwindow;
    NPSetWindowCallbackStruct *ws_info;

    GtkWidget *socket= gtk_socket_new();

    //g_signal_connect(socket, "plug_removed", G_CALLBACK(plug_removed_cb), NULL);
    //g_signal_connect(socket, "unrealize", G_CALLBACK(socket_unrealize_cb), NULL);
    //g_signal_connect(socket, "destroy", G_CALLBACK(gtk_widget_destroyed), &socket);

    gtk_container_add(GTK_CONTAINER(widget), socket);
    gtk_widget_realize(socket);

    gtk_widget_show(socket);
    gdk_flush();

    Window win = gtk_socket_get_id(GTK_SOCKET(socket));
    GdkWindow *gwin = gtk_widget_get_window(socket);
    npwindow = (NPWindow*)malloc(sizeof(NPWindow));
    npwindow->window = (void*)win;
    npwindow->x = 0;
    npwindow->y = 0;
    npwindow->width = width;
    npwindow->height = height;

    ws_info = (NPSetWindowCallbackStruct*)malloc(sizeof(NPSetWindowCallbackStruct));
    ws_info->type = NP_SETWINDOW;
    ws_info->display = gdk_x11_display_get_xdisplay(gdk_window_get_display(gwin));
    ws_info->visual = gdk_x11_visual_get_xvisual(gdk_window_get_visual(gwin));
    ws_info->colormap = XDefaultColormapOfScreen(XDefaultScreenOfDisplay(ws_info->display));
    ws_info->depth = gdk_visual_get_depth(gdk_window_get_visual(gwin));

    npwindow->ws_info = ws_info;
    npwindow->type = NPWindowTypeWindow;

    //XFlush(ws_info->display);

    return npwindow;
}

static gboolean plug_removed_cb (GtkWidget *widget, gpointer data) {
    printf("[!] plug_removed_cb\n");
    return TRUE;
}
static void socket_unrealize_cb(GtkWidget *widget, gpointer data) {
    printf("[!] socket_unrealize_cb\n");
    //gtk_widget_unrealize(widget);
}
#endif

#ifndef MAKE_SO
gboolean timer_callback(gpointer data){
    struct FRPlugin *plug = (struct FRPlugin*)data;
    static int val = 1;

    if(val)
        plug_move(plug, 100, 100);
    else
        plug_resize(plug, 400, 300);

    return val--;
}

int main(int argc, char **argv){
    int pid;
#if 1
    if(argc != 3){
        printf("\tUSAGE: player url flashvars\n"
               "\turl:       Video URL.\n"
               "\tflashvars: flashvars of URL.");
        return -1;
    }
#endif

    gtk_init(&argc, &argv);

    //gtk_window_new(GTK_WINDOW_TOPLEVEL);
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_add_events(main_window, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
#ifndef MAKE_SO
    g_signal_connect(main_window, "scroll-event", (GCallback)scroll_callback2, main_window);
#endif

    GtkWidget *bf = box_fixed_new();
    gtk_container_add(GTK_CONTAINER(main_window), bf);

    GtkWidget *label = gtk_label_new("hello");
    gtk_widget_set_size_request(label, 800, 600);
    gtk_container_add(GTK_CONTAINER(bf), label);
#if 0
    gtk_widget_show_all(main_window);

    box_fixed_put(BOX_FIXED(bf), gtk_socket_new(), 0, 0);

    gtk_main();

    return 0;
#endif
    fr_init(bf, main_window);

    gtk_widget_show_all(main_window);

    struct FRPlugin *plug = plug_new(0);
    plug->w = 600;
    plug->h = 400;
    plug_play(plug, argv[1], argv[2]);
    //My_NPP_GetValue(instance, NPPVpluginScriptableNPObject, &object);
    struct Rgn *rgn = malloc(sizeof(struct Rgn) + sizeof(struct Rect));
    rgn->num = 1;
    rgn->rects[0].x = 0;
    rgn->rects[0].y = 0;
    rgn->rects[0].w = 600;
    rgn->rects[0].h = 400;

    //plug_resize(plug, 600, 400);
    //plug_mask(plug, rgn);

    g_timeout_add(5000, timer_callback, plug);

    gtk_main();

    plug_destroy(plug);
    pool_destroy(g_FRManager.pool);

#if 0
    fprintf(stderr, "structVersion: %x\n", object._class->structVersion);
    fprintf(stderr, "NPAlloc: %p\n", object._class->allocate);
    fprintf(stderr, "NPDealloc: %p\n", object._class->deallocate);
    fprintf(stderr, "NPInval: %p\n", object._class->invalidate);
    fprintf(stderr, "NPHasMtd: %p\n", object._class->hasMethod);
    fprintf(stderr, "NPInvoke: %p\n", object._class->invoke);
    fprintf(stderr, "NPIvkDft: %p\n", object._class->invokeDefault);
    fprintf(stderr, "NPHasProp: %p\n", object._class->hasProperty);
    fprintf(stderr, "NPGetProp: %p\n", object._class->getProperty);
    fprintf(stderr, "NPSetProp: %p\n", object._class->setProperty);
    fprintf(stderr, "NPRmProp: %p\n", object._class->removeProperty);
    fprintf(stderr, "NPEnum: %p\n", object._class->enumerate);
    fprintf(stderr, "NPConstruct: %p\n", object._class->construct);
#endif

    return 0;
}
#endif
