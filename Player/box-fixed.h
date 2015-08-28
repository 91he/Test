#ifndef __BOX_FIXED_H__
#define __BOX_FIXED_H__

#include <gtk/gtk.h>
#include <cairo.h>

G_BEGIN_DECLS

/* Standart GObject macros */
#define BOX_TYPE_FIXED (box_fixed_get_type())
#define BOX_FIXED(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), BOX_TYPE_FIXED, BoxFixed))
#define BOX_FIXED_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), BOX_TYPE_FIXED, BoxFixedClass))
#define BOX_IS_FIXED(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), BOX_TYPE_FIXED))
#define BOX_IS_FIXED_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), BOX_TYPE_FIXED))
#define BOX_FIXED_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), BOX_TYPE_FIXED, BoxFixedClass))

/* Type definition */
typedef struct _BoxFixed        BoxFixed;
typedef struct _BoxFixedClass   BoxFixedClass;
typedef struct _BoxFixedPrivate BoxFixedPrivate;

struct _BoxFixed {
    //GtkWidget parent;
    GtkContainer parent;

    /*< Private >*/
    BoxFixedPrivate *priv;
};

struct _BoxFixedClass {
    GtkContainerClass parent_class;
};

/* Public API */
GType      box_fixed_get_type(void) G_GNUC_CONST;
GtkWidget *box_fixed_new(void);

void box_fixed_put(BoxFixed *bf, GtkWidget *widget, gint x, gint y);
void box_fixed_move(BoxFixed *bf, GtkWidget *widget, gint x, gint y);

G_END_DECLS

#endif /* __BOX_FIXED_H__ */
