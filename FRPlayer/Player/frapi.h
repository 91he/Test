#ifndef FR_API_H
#define FR_API_H
#include <gtk/gtk.h>

GtkWidget *fr_init(GtkWidget *spice);
void fr_process_msg(char *msg, int size);
void fr_disp_resize(int w, int h);

#endif
