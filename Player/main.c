#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include "pool.h"
#include "curl_url.h"
#include "npp_funcs.h"
#include "npn_funcs.h"

struct FRPlugin{
    //UUID
    NPP instance;
    NPWindow *np_win;
    GtkWidget *socket;
    cairo_t *cr;
    GdkPixmap *pixmap;
    //Rgn *rgn;
};

GtkWidget *main_window = NULL;
struct thread_pool *g_pool = NULL;

void npwindow_set(NPP instance, NPWindow *np_win, int w, int h){
    np_win->width = w;
    np_win->height = h;

    My_NPP_SetWindow(instance, np_win);
}

NPWindow *npwindow_init(GtkWidget *widget){ //Must ensure widget has been anchored.(Widget must be a child of window.)
    NPWindow *npwindow;
    GtkWidget *socket;
    NPSetWindowCallbackStruct *ws_info;

    socket = gtk_socket_new();
    //g_signal_connect();
    //TODO box_fixed_put(BOX_FIXED(widget), socket, 0, 0);//Must do this.
    gtk_widget_realize(socket);
    //gtk_widget_show(socket); //Seems there is no use doing this.

    GdkWindow *gwin = gtk_widget_get_window(socket);
    npwindow = (NPWindow*)malloc(sizeof(NPWindow));
    npwindow->window = GINT_TO_POINTER(gtk_socket_get_id(GTK_SOCKET(socket)));

    ws_info = (NPSetWindowCallbackStruct*)malloc(sizeof(NPSetWindowCallbackStruct));
    ws_info->type = NP_SETWINDOW;
    ws_info->display = gdk_x11_display_get_xdisplay(gdk_window_get_display(gwin));
    ws_info->visual = gdk_x11_visual_get_xvisual(gdk_window_get_visual(gwin));
    ws_info->colormap = XDefaultColormapOfScreen(XDefaultScreenOfDisplay(ws_info->display));
    ws_info->depth = gdk_visual_get_depth(gdk_window_get_visual(gwin));

    npwindow->ws_info = ws_info;
    npwindow->type = NPWindowTypeWindow;

    npwindow->x = 0;
    npwindow->y = 0;
}

#if 1
static NPWindow *npwindow_construct(GtkWidget *widget, uint32_t width, uint32_t height){                                                             
    NPWindow *npwindow;
    NPSetWindowCallbackStruct *ws_info;
#if 1
    GtkWidget *socket= gtk_socket_new();

    //g_signal_connect(socket, "plug_removed", G_CALLBACK(plug_removed_cb), NULL);
    //g_signal_connect(socket, "unrealize", G_CALLBACK(socket_unrealize_cb), NULL);
    //g_signal_connect(socket, "destroy", G_CALLBACK(gtk_widget_destroyed), &socket);

    gtk_container_add(GTK_CONTAINER(widget), socket);
    gtk_widget_realize(socket);

    gtk_widget_show(socket);
    gdk_flush();

#if 0
    GtkAllocation new_allocation;
    new_allocation.x = 0;
    new_allocation.y = 0;
    new_allocation.width = width;
    new_allocation.height = height;
    gtk_widget_size_allocate(socket, &new_allocation);
#endif

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
#else
    Display* display = gdk_x11_display_get_xdisplay(gdk_display_get_default());
    npwindow->type = NPWindowTypeDrawable;
    npwindow->window = 0;
    //npwindow->window = gdk_x11_window_get_xid(gtk_widget_get_window(widget));
    int depth = gdk_visual_get_depth(gdk_screen_get_system_visual(gdk_screen_get_default()));                                                                                                                      
    ws_info->display = display;
    ws_info->depth = depth;

    XVisualInfo visualTemplate;
    visualTemplate.screen = gdk_screen_get_number(gdk_screen_get_default());
    visualTemplate.depth = depth;
    visualTemplate.class = TrueColor;
    int numMatching;
    XVisualInfo* visualInfo = XGetVisualInfo(display, VisualScreenMask | VisualDepthMask | VisualClassMask,
            &visualTemplate, &numMatching);
    //ASSERT(visualInfo);
    Visual* visual = visualInfo[0].visual;
    //ASSERT(visual);
    XFree(visualInfo);
    fprintf(stderr, "%p, %p, %d\n", visualInfo, visual, numMatching);

    ws_info->visual = visual;
    ws_info->colormap = XCreateColormap(display, gdk_x11_get_default_root_xwindow(), visual, AllocNone);
    fprintf(stderr, "%x, %x, %x, %x\n", ws_info->display, ws_info->visual, ws_info->colormap, ws_info->depth);

    npwindow->x = 0;
    npwindow->y = 0;
    npwindow->width = width;
    npwindow->height = height;

#endif

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
static NPWindow *npwindow_construct(GtkWidget *widget, unsigned int width, unsigned int height){
    NPWindow *npwindow;
    NPSetWindowCallbackStruct *ws_info = NULL;

    GdkWindow *parent_win = widget->window;

    GtkWidget *socketWidget = gtk_socket_new();
    gtk_widget_set_parent_window(socketWidget, parent_win);

    g_signal_connect(socketWidget, "plug_removed", G_CALLBACK(plug_removed_cb), NULL);
    g_signal_connect(socketWidget, "unrealize", G_CALLBACK(socket_unrealize_cb), NULL);
    g_signal_connect(socketWidget, "destroy", G_CALLBACK(gtk_widget_destroyed), &socketWidget);

    gpointer user_data = NULL;
    gdk_window_get_user_data(parent_win, &user_data);

    GtkContainer *container = GTK_CONTAINER(user_data);
    gtk_container_add(container, socketWidget);
    gtk_widget_realize(socketWidget);

    GtkAllocation new_allocation;
    new_allocation.x = 0;
    new_allocation.y = 0;
    new_allocation.width = width;
    new_allocation.height = height;
    gtk_widget_size_allocate(socketWidget, &new_allocation);

    gtk_widget_show(socketWidget);
    gdk_flush();

    GdkNativeWindow ww = gtk_socket_get_id(GTK_SOCKET(socketWidget));
    GdkWindow *w = gdk_window_lookup(ww);

    npwindow = (NPWindow*)malloc(sizeof(NPWindow));
    npwindow->window = (void*)(unsigned long)ww;
    npwindow->x = 0;
    npwindow->y = 0;
    npwindow->width = width;
    npwindow->height = height;

    ws_info = (NPSetWindowCallbackStruct*)malloc(sizeof(NPSetWindowCallbackStruct));
    ws_info->type = NP_SETWINDOW;
    ws_info->display = GDK_WINDOW_XDISPLAY(w);
    ws_info->colormap = GDK_COLORMAP_XCOLORMAP(gdk_drawable_get_colormap(w));
    GdkVisual* gdkVisual = gdk_drawable_get_visual(w);
    ws_info->visual = GDK_VISUAL_XVISUAL(gdkVisual);
    ws_info->depth = gdkVisual->depth;

    npwindow->ws_info = ws_info;
    npwindow->type = NPWindowTypeWindow;

    return npwindow;
}
#endif

gboolean hide_callback(GtkWidget *widget, GdkEvent *event, gpointer user_data){
    fprintf(stderr, "============================================================================\n");
    gtk_widget_hide(widget);
    return false;
}

int main(int argc, char **argv){
    int pid;
#if 1
    if(argc != 2){
        printf("\tUSAGE: player url\n\turl: Video URL which consist of swf address and flashvars. \n");
        return -1;
    }
#endif
#if 0
    if(pid = fork()){//dad
        printf("%d: %d\n", getpid(), pid);
    }else{//son
#else
    {
#endif
        g_pool = pool_init(4);
        gtk_init(&argc, &argv);

        main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        //main_window = gtk_window_new(GTK_WINDOW_POPUP);
        gtk_window_set_decorated(GTK_WINDOW(main_window), FALSE);
        gtk_window_move(GTK_WINDOW(main_window), 300, 100);
        gtk_widget_realize(main_window);

        //GtkWidget *entry = gtk_entry_new();
        //gtk_container_add(GTK_CONTAINER(main_window), entry);

        gtk_widget_show_all(main_window);
        g_signal_connect(main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

        NPP_t instance_t = {NULL};
        NPP instance = &instance_t;
        NPObject object;

        char *xargv[] = {"id", "allowFullScreen", "flashvars", "allowScriptAccess", "swLiveConnect"};
        char *xargm[] = {"flash", "true", "albumId=202203201&tvId=373008100&autoplay=true&isMember=false&cyclePlay=false&exclusive=1&qiyiProduced=1&share_sTime=0&share_eTime=0&tipdataurl=http://static.iqiyi.com/ext/common/tipdata_201501161615.xml&components=feffffe6e&qiyiProducedPreloader=http://www.iqiyi.com/common/flashplayer/20141229/loading.swf&exclusivePreloader=http://www.iqiyi.com/player/20140120131607/loading.swf&preloader=http://www.iqiyi.com/common/flashplayer/20150612/loading_daomu.swf&vipPreloader=http://www.iqiyi.com/common/flashplayer/20150608/vip_loading.swf&adurl=http://www.iqiyi.com/common/flashplayer/20150612/am-2-3-7-3.swf&flashP2PCoreUrl=http://www.iqiyi.com/common/flashplayer/20150624/3018.swf&cpnc=b4f1dec2ca18f063d6bfda5a6ca047bf&cpnv=1.0&cid=qc_100001_100015&origin=flash&outsite=false&yhls=1450328048458&playerCTime=1435126133566&webEventID=928c19d23a6a1be83861a771664a93fa&definitionID=17c1b4c8fec0d0e10a2ce05d778a52de", "true", "true"};
        My_NPP_New("application/x-shockwave-flash", instance, NP_EMBED, sizeof(xargv)/sizeof(char*), xargv, xargm, 0);
        NPWindow *np_win = npwindow_construct(main_window, 800, 500);
        My_NPP_GetValue(instance, NPPVpluginScriptableNPObject, &object);
        My_NPP_SetWindow(instance, np_win);

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

        My_NPN_GetURLNotify(instance, argv[1], NULL, NULL);

        gtk_main();

        My_NPP_Destroy(instance, NULL);
        pool_destroy(g_pool);
    }

    return 0;
}
