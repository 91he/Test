#ifndef SCR_WIN_H
#define SCR_WIN_H

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define SCR_WIN_TYPE            (scr_win_get_type())
#define SCR_WIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), SCR_WIN_TYPE, Scrwin))
#define SCR_WIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), SCR_WIN_TYPE, ScrwinClass))
#define IS_SCR_WIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), SCR_WIN_TYPE))
#define IS_SCR_WIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SCR_WIN_TYPE))

typedef struct _Scrwin       Scrwin;
typedef struct _ScrwinClass  ScrwinClass;

struct _Scrwin
{
	GtkBin parent_instance;
//	GtkContainer parent_instance;
};

struct _ScrwinClass
{
	GtkBinClass parent_class;
//	GtkContainerClass parent_class;
};

GType scr_win_get_type(void) G_GNUC_CONST;
GtkWidget *scr_win_new(void);

G_END_DECLS

/* Private class member */
#define SCR_WIN_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), \
	SCR_WIN_TYPE, ScrwinPrivate))

typedef struct _ScrwinPrivate ScrwinPrivate;

struct _ScrwinPrivate
{
	GtkWidget *child;
};

#endif /* SCR_WIN_H */
