#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include "pool.h"
#include "curl_url.h"
#include "npp_funcs.h"
#include "npn_funcs.h"
#include "box-fixed.h"
#include "frapi.h"

enum MsgType{
    FR_INIT,
    FR_CORD,
    FR_SIZE,
    FR_RGN,
    FR_HIDE,
    FR_DESTROY
};

FRManager g_FRManager;

gboolean configure_callback(GtkWidget *widget, GdkEvent *event, gpointer data){
    g_FRManager.disp_width = event->configure.width;
    g_FRManager.disp_height = event->configure.height;

    //TODO: forall plugs. recreate pixmap & cr.

    return FALSE;
}

void fr_init(GtkWidget *widget){
    gtk_widget_add_events(widget, GDK_STRUCTURE_MASK);
    g_signal_connect(widget, "configure-event", (GCallback)configure_callback, NULL);

    g_FRManager.bf = BOX_FIXED(widget);
    g_FRManager.pool = pool_init(4);

    g_FRManager.disp_width = 800;
    g_FRManager.disp_height = 600;
    //TODO. init plugs
}

static void npwindow_set(NPP instance, NPWindow *np_win, int w, int h){
    np_win->width = w;
    np_win->height = h;

    My_NPP_SetWindow(instance, np_win);
}

static void npwindow_init(FRPlugin *plug, BoxFixed *bf); //Must ensure widget has been anchored.(Widget must be a child of window.)

static void set_rgn(FRPlugin *plug, struct Rgn *rgn){
    int i;
    cairo_t *cr = plug->cr;
    int w, h;

    if(rgn && rgn->num >= 0){
        for(i = 0; i < rgn->num;){
            cairo_rectangle(cr, rgn->rects[i].x, rgn->rects[i].y, rgn->rects[i].w, rgn->rects[i].h);
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
    //gtk_widget_show(plug->socket);//should call plug_mask
}

static void plug_move(struct FRPlugin *plug, int x, int y){
    plug->x = x;
    plug->y = y;

    box_fixed_move(g_FRManager.bf, plug->socket, x, y);
}

static void npwindow_init(FRPlugin *plug, BoxFixed *bf){ //Must ensure widget has been anchored.(Widget must be a child of window.)
    NPWindow *npwindow;
    GtkWidget *socket;
    NPSetWindowCallbackStruct *ws_info;

    plug->socket = socket = gtk_socket_new();
    //g_signal_connect();
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

static FRPlugin *plug_new(int id, char *url, char *flashvars){
    int w, h;
    NPObject object;
    struct FRPlugin *plug;
    char *xargv[3] = {"allowFullScreen", "allowScriptAccess", "flashvars"};
    char *xargm[3] = {"true", "true", flashvars};

    plug = malloc(sizeof(struct FRPlugin));

    plug->id = id;
    plug->x = plug->y = 0;
    memset(&plug->instance, 0, sizeof(plug->instance));
    //npwindow_init(plug, g_FRManager.bf);
    plug->pixmap = gdk_pixmap_new(NULL, g_FRManager.disp_width, g_FRManager.disp_height, 1);
    plug->cr = gdk_cairo_create(GDK_DRAWABLE(plug->pixmap));
    plug->np_win = NULL;

    cairo_set_operator(plug->cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba(plug->cr, 0, 0, 0, 0);
    gdk_pixmap_get_size(plug->pixmap, &w, &h);
    cairo_rectangle(plug->cr, 0, 0, w, h);
    cairo_fill(plug->cr);

    My_NPP_New("application/x-shockwave-flash", &plug->instance, NP_EMBED, 3, xargv, xargm, 0);
    My_NPP_GetValue(&plug->instance, NPPVpluginScriptableNPObject, &object);

    //plug_resize(plug, 0, 0);//May not need.
    My_NPN_GetURLNotify(&plug->instance, url, NULL, NULL);

    return plug;
}

static void plug_destroy(struct FRPlugin *plug){
    My_NPP_Destroy(&plug->instance, NULL);
}

#if 1
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

#else
static gboolean plug_removed_cb (GtkWidget *widget, gpointer data) {
    printf("[!] plug_removed_cb\n");
    return TRUE;
}
static void socket_unrealize_cb(GtkWidget *widget, gpointer data) {
    printf("[!] socket_unrealize_cb\n");
    //gtk_widget_unrealize(widget);
}
#endif

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

    GtkWidget *main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    //gtk_window_set_decorated(GTK_WINDOW(main_window), FALSE);
    //gtk_window_move(GTK_WINDOW(main_window), 300, 100);
    //gtk_widget_realize(main_window);

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
    fr_init(bf);

    gtk_widget_show_all(main_window);

    struct FRPlugin *plug = plug_new(0, argv[1], argv[2]);
    //My_NPP_GetValue(instance, NPPVpluginScriptableNPObject, &object);
    struct Rgn *rgn = malloc(sizeof(struct Rgn) + sizeof(struct Rect));
    rgn->num = 1;
    rgn->rects[0].x = 0;
    rgn->rects[0].y = 0;
    rgn->rects[0].w = 300;
    rgn->rects[0].h = 200;

    plug_resize(plug, 600, 400);
    plug_mask(plug, rgn);

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
