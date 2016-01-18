#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <cairo/cairo.h>

#include "frapi.h"
typedef struct FRPlugin FRPlugin;

struct Rgn{
    int num;
    cairo_rectangle_t rects[0];
};

int main(int argc, char **argv){
    gtk_init(&argc, &argv);

    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(win, "destroy", gtk_main_quit, NULL);

    GtkWidget *overlay = gtk_overlay_new();
    gtk_container_add(GTK_CONTAINER(overlay), gtk_label_new("hello"));
    //gtk_overlay_add_overlay(GTK_OVERLAY(overlay), fixed);
    gtk_container_add(GTK_CONTAINER(win), overlay);

    fr_init(overlay, gtk_label_new(""));
    gtk_widget_show_all(win);

    FRPlugin *plug = plug_new(0);
    plug_move(plug, 100, 0);
    plug_resize(plug, 600, 400);
    struct Rgn *rgn = malloc(sizeof(struct Rgn) + sizeof(cairo_rectangle_t));
    rgn->num = 1;
    rgn->rects[0].x = 0;
    rgn->rects[0].y = 0;
    rgn->rects[0].width = 0;
    rgn->rects[0].height = 0;
    plug_mask(plug, rgn);

    plug_play(plug, "http://static.youku.com/v1.0.0567/v/swf/player_yknpsv.swf", "VideoIDS=XMTMzMTMwNzYyMA==&ShowId=298715&allowFullScreen=true&category=84&Cp=authorized&sv=true&ev=1&Light=on&THX=off&unCookie=0&frame=0&pvid=1441953523214FVHQUB&uepflag=0&Tid=0&isAutoPlay=true&Version=/v1.0.1090&show_ce=0&winType=interior&openScanCode=1&scanCodeText=\"限时\" 扫码免广告&embedid=AjMzMjgyNjkwNQJ3d3cueW91a3UuY29tAi8=&vext=bc%3D%26pid%3D1441953523214FVHQUB%26unCookie%3D0%26frame%3D0%26type%3D0%26svt%3D1%26stg%3D8%26emb%3DAjMzMjgyNjkwNQJ3d3cueW91a3UuY29tAi8%3D%26dn%3D%E7%BD%91%E9%A1%B5%26hwc%3D1%26mtype%3Doth&pageStartTime=1441953523210");

    g_timeout_add(5000, timer_callback, plug);
    gtk_main();

    return 0;
}


