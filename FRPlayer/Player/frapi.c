#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <webkit2/webkit2.h>

#include "frapi.h"
#include "box-fixed.h"

enum MsgType{
    PLUG_INIT,
    PLUG_CORD,
    PLUG_SIZE,
    PLUG_RGN,
    PLUG_HIDE,
    PLUG_DESTROY
};

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
    struct MsgQueue msg_queue;
    GHashTable *plugs;
    GHashTable *hash_table;
}FRManager;

struct Cord{
    int x, y;
};

struct WNH{
    int w, h;
};

struct Rgn{
    int num;
    cairo_rectangle_int_t rects[0];
};

typedef struct FRPlugin{
    int id;
    int x, y, w, h;
    char *cookie;
    GtkWidget *view;
    struct Rgn *rgn;
}FRPlugin;

FRManager g_FRManager;

static FRPlugin *plug_new(int id);
static void plug_play(struct FRPlugin *plug, char *host, char *url, char *flashvars);
static void plug_move(struct FRPlugin *plug, int x, int y);
static void plug_resize(struct FRPlugin *plug, int w, int h);
static void plug_mask(struct FRPlugin *plug, struct Rgn *rgn);
static void plug_destroy(struct FRPlugin *plug);

cairo_region_t *make_region(struct Rgn *rgn){
    int i;
    cairo_region_t *ret = NULL;

    if(rgn && rgn->num >= 0){ 
        ret = cairo_region_create();

        for(i = 0; i < rgn->num; i++){
            rgn->rects[i].width -= rgn->rects[i].x;
            rgn->rects[i].height -= rgn->rects[i].y;
            cairo_region_union_rectangle(ret, &rgn->rects[i]);
        }   
    }   

    return ret;
}   

gboolean configure_callback(GtkWidget *widget, GdkEvent *event, gpointer data){
    g_FRManager.disp_width = event->configure.width;
    g_FRManager.disp_height = event->configure.height;

    //g_idle_add(travel_callback, NULL);

    return FALSE;
}

GtkWidget *fr_init(GtkWidget *spice){
    GtkWidget *widget;

    gtk_widget_add_events(spice, GDK_STRUCTURE_MASK);
    g_signal_connect(spice, "configure-event", (GCallback)configure_callback, NULL);

    widget = box_fixed_new();
    gtk_container_add(GTK_CONTAINER(widget), spice);

    g_FRManager.bf = BOX_FIXED(widget);
    g_FRManager.spice = spice;

    g_FRManager.disp_width = 800;
    g_FRManager.disp_height = 600;
    g_FRManager.msg_queue.type = QUEUE_SIZE;
    g_FRManager.msg_queue.len = 0;
    g_FRManager.msg_queue.size = 4096;
    g_FRManager.msg_queue.cur_pos = 0;
    g_FRManager.msg_queue.msg = malloc(g_FRManager.msg_queue.size);
    g_FRManager.plugs = g_hash_table_new(g_direct_hash, g_direct_equal);

    return widget;
}

gboolean plug_process(gpointer data){
    char *msg;
    char *tmp = (char*)data;
    char type = tmp[0];
    int id = *(int*)&tmp[1];
    FRPlugin *plug = NULL;

    msg = &tmp[5];
    switch(type){
        case PLUG_INIT:
            {
                int i = 0;
                char *str[4];
                int len = 0;

                do{
                    str[i] = msg + len;
                    len += strlen(str[i]) + 1;
                }while(i++ < 4);

                if(plug = g_hash_table_lookup(g_FRManager.plugs, (gpointer)(intptr_t)id)){
                    plug_play(plug, str[0], str[1], str[2]);
                    plug->cookie = strdup(str[3]);
                }else{
                    plug = plug_new(id);
                    plug_play(plug, str[0], str[1], str[2]);
                    plug->cookie = strdup(str[3]);
                    g_hash_table_insert(g_FRManager.plugs, (gpointer)(intptr_t)id, plug);
                    //fprintf(stderr, "Flash has been inited!\n");
                }
            }
            break;
        case PLUG_CORD:
            {
                struct Cord *cord = (struct Cord*)msg;
                if(plug = g_hash_table_lookup(g_FRManager.plugs, (gpointer)(intptr_t)id)){
                    if(plug->view){
                        plug_move(plug, cord->x, cord->y);
                    }else{
                        plug->x = cord->x;
                        plug->y = cord->y;
                    }
                }else{
                    plug = plug_new(id);
                    plug->x = cord->x;
                    plug->y = cord->y;
                    plug_move(plug, plug->x, plug->y);
                    g_hash_table_insert(g_FRManager.plugs, (gpointer)(intptr_t)id, plug);
                    //fprintf(stderr, "Flash hasn't been inited!\n");
                }
            }
            break;
        case PLUG_SIZE:
            {
                struct WNH *wnh = (struct WNH*)msg;
                if(plug = g_hash_table_lookup(g_FRManager.plugs, (gpointer)(intptr_t)id)){
                    if(plug->view){
                        plug_resize(plug, wnh->w, wnh->h);
                    }else{
                        plug->w = wnh->w;
                        plug->h = wnh->h;
                    }
                }else{
                    plug = plug_new(id);
                    plug->w = wnh->w;
                    plug->h = wnh->h;
                    g_hash_table_insert(g_FRManager.plugs, (gpointer)(intptr_t)id, plug);
                    //fprintf(stderr, "Flash hasn't been inited!\n");
                }
            }
            break;
        case PLUG_RGN:
            {
                struct Rgn *rgn = (struct Rgn*)msg;
                if(plug = g_hash_table_lookup(g_FRManager.plugs, (gpointer)(intptr_t)id)){
                    if(plug->view){
                        plug_mask(plug, rgn);
                    }else{
                        if(plug->rgn) free(plug->rgn);
                        plug->rgn = malloc(sizeof(*rgn) + rgn->num * sizeof(cairo_rectangle_int_t));
                        memcpy(plug->rgn, rgn, sizeof(*rgn) + rgn->num * sizeof(cairo_rectangle_int_t));
                    }
                }else{
                    plug = plug_new(id);
                    plug->rgn = malloc(sizeof(*rgn) + rgn->num * sizeof(cairo_rectangle_int_t));
                    memcpy(plug->rgn, rgn, sizeof(*rgn) + rgn->num * sizeof(cairo_rectangle_int_t));
                    g_hash_table_insert(g_FRManager.plugs, (gpointer)(intptr_t)id, plug);
                    //fprintf(stderr, "Flash hasn't been inited!\n");
                }
            }
            break;
        case PLUG_HIDE:
            if(plug = g_hash_table_lookup(g_FRManager.plugs, (gpointer)(intptr_t)id)){
                if(plug->view) gtk_widget_hide(plug->view);
            }else{
                plug = plug_new(id);
                g_hash_table_insert(g_FRManager.plugs, (gpointer)(intptr_t)id, plug);
                //fprintf(stderr, "Flash hasn't been inited!\n");
            }
            break;
        case PLUG_DESTROY:
            if(plug = g_hash_table_lookup(g_FRManager.plugs, (gpointer)(intptr_t)id)){
                g_hash_table_remove(g_FRManager.plugs, (gpointer)(intptr_t)id);
                plug_destroy(plug);
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

    if(type == QUEUE_SIZE){
        if(len > 4){
            type = QUEUE_MSG;
            len -= 4;
            cur_pos += 4;
            msg_len = *(int*)queue_msg;
            if(len >= msg_len){
                char *pmsg = malloc(msg_len);
                memcpy(pmsg, &queue_msg[cur_pos], msg_len);
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

static gboolean scroll_callback(GtkWidget *widget, GdkEvent *event, gpointer data){
    static gboolean bret;
    static GdkEvent tmp;

    tmp = *event;
    if(tmp.scroll.direction == GDK_SCROLL_SMOOTH){
        tmp.scroll.direction = event->scroll.delta_y > 0 ? GDK_SCROLL_DOWN : GDK_SCROLL_UP;
    }
    gtk_widget_event(g_FRManager.spice, &tmp);
    g_signal_emit_by_name(g_FRManager.spice, "scroll-event", &tmp, &bret);

    return TRUE;
}


static void plug_mask(struct FRPlugin *plug, struct Rgn *rgn){
    cairo_region_t *crgn = NULL;

    if(plug->rgn != rgn && rgn){
        if(plug->rgn) free(plug->rgn);
        plug->rgn = malloc(sizeof(struct Rgn) + rgn->num * sizeof(cairo_rectangle_int_t));
        memcpy(plug->rgn, rgn, sizeof(struct Rgn) + rgn->num * sizeof(cairo_rectangle_int_t));
    }

    crgn = make_region(rgn);

    gtk_widget_shape_combine_region(plug->view, crgn);
    gtk_widget_show(plug->view);
}

static void plug_resize(struct FRPlugin *plug, int w, int h){
    plug->w = w;
    plug->h = h;

    gtk_widget_set_size_request(plug->view, w, h);
    plug_mask(plug, plug->rgn);
}

static void plug_move(struct FRPlugin *plug, int x, int y){
    //plug->x = x;
    //plug->y = y;

    box_fixed_move(g_FRManager.bf, plug->view, x, y);
}

static FRPlugin *plug_new(int id){
    int w, h;
    struct FRPlugin *plug;

    plug = malloc(sizeof(struct FRPlugin));

    plug->id = id;
    plug->x = plug->y = 0;
    plug->w = plug->h = 0;
    plug->rgn = NULL;
    plug->view = webkit_web_view_new();
    plug->cookie = NULL;

    gtk_widget_add_events(plug->view, GDK_SCROLL_MASK);
    g_signal_connect(plug->view, "scroll-event", (GCallback)scroll_callback, NULL);
    box_fixed_put(g_FRManager.bf, plug->view, plug->x, plug->y);

    return plug;
}

static void plug_play(struct FRPlugin *plug, char *host, char *url, char *flashvars){
    char full_host[128];
    char html[8192];

    if(!strstr(host, "http"))
        sprintf(full_host, "http://%s", host);
    else
        strcpy(full_host, host);

    sprintf(html, "<html><body style=\"height: 100%; width: 100%; overflow: hidden; margin: 0\"><object type=\"application/x-shockwave-flash\" data=\"%s\" width=\"100%\" height=\"100%\"><param name=\"allowFullScreen\" value=\"true\"><param name=\"allowscriptaccess\" value=\"always\"><param name=\"allowFullScreenInteractive\" value=\"true\"><param name=\"flashvars\" value=\"%s\"><param name=\"movie\" value=\"%s\"></object></body><style type=\"text/css\"></style></html>", url, flashvars, url);

    fprintf(stderr, "%s\n", html);
    if(strstr(host, "letv")){
        webkit_web_view_load_html(WEBKIT_WEB_VIEW(plug->view), html, full_host);
    }else{
        webkit_web_view_load_html(WEBKIT_WEB_VIEW(plug->view), html, NULL);
    }
}

static void plug_destroy(struct FRPlugin *plug){
    gtk_widget_destroy(plug->view);
    if(plug->rgn) free(plug->rgn);
    if(plug->cookie) free(plug->cookie);
    free(plug);
}

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
#if 0
    if(argc != 3){
        printf("\tUSAGE: player url flashvars\n"
               "\turl:       Video URL.\n"
               "\tflashvars: flashvars of URL.");
        return -1;
    }
#endif
    gtk_init(&argc, &argv);

    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request(win, 800, 600);
    g_signal_connect(win, "destroy", gtk_main_quit, NULL);

    //GtkWidget *overlay = gtk_overlay_new();
    //gtk_container_add(GTK_CONTAINER(overlay), gtk_button_new_with_label("hello"));
    GtkWidget *bf = fr_init(gtk_button_new_with_label("hello"));
    //box_fixed_put(BOX_FIXED(fixed), gtk_button_new_with_label("hello"), 0, 0);
    //gtk_overlay_add_overlay(GTK_OVERLAY(overlay), fixed);
    //gtk_container_add(GTK_CONTAINER(bf), gtk_button_new_with_label("hello"));
    gtk_container_add(GTK_CONTAINER(win), bf);

    //fr_init(overlay, gtk_label_new(""));

    FRPlugin *plug = plug_new(0);
    plug_resize(plug, 600, 400);
    plug_move(plug, 100, 0);
    struct Rgn *rgn = malloc(sizeof(struct Rgn) + sizeof(cairo_rectangle_t));
    rgn->num = 1;
    rgn->rects[0].x = 0;
    rgn->rects[0].y = 0;
    rgn->rects[0].width = 600;
    rgn->rects[0].height = 400;
    plug_mask(plug, rgn);

    //plug_play(plug, "v.youku.com", argv[1], argv[2]);
    plug_play(plug, "http://v.youku.com", "http://static.youku.com/v1.0.0568/v/swf/loader.swf", "VideoIDS=XMTI1ODc5MjU2NA==&THX=off&Version=/v1.0.1091&frame=0&Light=on&embedid=AjMxNDY5ODE0MQJ3d3cueW91a3UuY29tAi8=&isAutoPlay=true&ShowId=299011&Tid=0&Cp=authorized&uepflag=0&openScanCode=1&pageStartTime=1442384940838&category=97&pvid=1442384940840luOJP8&winType=interior&scanCodeText=xx&vext=bc=&pid=1442384940840luOJP8&unCookie=0&frame=0&type=0&svt=1&stg=1&emb=AjMxNDY5ODE0MQJ3d3cueW91a3UuY29tAi8=&dn=xx&hwc=1&mtype=oth&unCookie=0&show_ce=0");

    gtk_widget_show_all(win);
    //g_timeout_add(5000, timer_callback, plug);
    gtk_main();

    return 0;
}

#endif
