#ifndef BOX_FIXED_H
#define BOX_FIXED_H

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define BOX_FIXED_TYPE            (box_fixed_get_type())
#define BOX_FIXED(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), BOX_FIXED_TYPE, BoxFixed))
#define BOX_FIXED_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), BOX_FIXED_TYPE, BoxFixedClass))
#define IS_BOX_FIXED(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), BOX_FIXED_TYPE))
#define IS_BOX_FIXED_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), BOX_FIXED_TYPE))

typedef struct _BoxFixed       BoxFixed;
typedef struct _BoxFixedClass  BoxFixedClass;

struct _BoxFixed
{
	GtkContainer parent_instance;
};

struct _BoxFixedClass
{
	GtkContainerClass parent_class;
};

GType box_fixed_get_type(void) G_GNUC_CONST;
GtkWidget *box_fixed_new(void);
void box_fixed_put(BoxFixed *bf, GtkWidget *widget, gint x, gint y);
void box_fixed_move(BoxFixed *bf, GtkWidget *widget, gint x, gint y);

G_END_DECLS

/* Private class member */
#define BOX_FIXED_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), \
	BOX_FIXED_TYPE, BoxFixedPrivate))

typedef struct _BoxFixedPrivate BoxFixedPrivate;

struct _BoxFixedPrivate
{
	GtkWidget *child;
    GList *children;
};

#endif /* BOX_FIXED_H */
