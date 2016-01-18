#include <gtk/gtk.h>
#include "box-fixed.h"

typedef struct _Overlay Overlay;

struct _Overlay{
    gint x, y;
    GtkWidget *widget;
};

/* Forward declarations */
static void box_fixed_get_preferred_width(GtkWidget *widget, int *minimal, int *natural);
static void box_fixed_get_preferred_height(GtkWidget *widget, int *minimal, int *natural);
static void box_fixed_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
static GType box_fixed_child_type(GtkContainer *container);
static void box_fixed_add(GtkContainer *container, GtkWidget *widget);
static void box_fixed_remove(GtkContainer *container, GtkWidget *widget);
static void box_fixed_forall(GtkContainer *container, gboolean include_internals, GtkCallback callback, gpointer callback_data);

/* Define the BoxFixed type and inherit from GtkBin */
//G_DEFINE_TYPE(BoxFixed, box_fixed, GTK_TYPE_BIN);
G_DEFINE_TYPE(BoxFixed, box_fixed, GTK_TYPE_CONTAINER);

/* Initialize the BoxFixed class */
static void
box_fixed_class_init(BoxFixedClass *klass)
{
	/* Override GtkWidget methods */
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	widget_class->get_preferred_width = box_fixed_get_preferred_width;
	widget_class->get_preferred_height = box_fixed_get_preferred_height;
	widget_class->size_allocate = box_fixed_size_allocate;

	/* Override GtkContainer methods */
	GtkContainerClass *container_class = GTK_CONTAINER_CLASS(klass);
	container_class->child_type = box_fixed_child_type;
	container_class->add = box_fixed_add;
	container_class->remove = box_fixed_remove;
	container_class->forall = box_fixed_forall;

	/* Add private indirection member */
	g_type_class_add_private(klass, sizeof(BoxFixedPrivate));
}

/* Initialize a new PSquare instance */
static void
box_fixed_init(BoxFixed *bf)
{
	/* This means that PSquare doesn't supply its own GdkWindow */
	gtk_widget_set_has_window(GTK_WIDGET(bf), FALSE);
	/* Set redraw on allocate to FALSE if the top left corner of your widget
	 * doesn't change when it's resized; this saves time */
	/*gtk_widget_set_redraw_on_allocate(GTK_WIDGET(square), FALSE);*/

	/* Initialize private members */
	BoxFixedPrivate *priv = BOX_FIXED_PRIVATE(bf);
	priv->child = NULL;
    priv->children = NULL;
}

/* Return a new PSquare cast to a GtkWidget */
GtkWidget *
box_fixed_new()
{
	return GTK_WIDGET(g_object_new(box_fixed_get_type(), NULL));
}

/* Get the width of the container */
static void
box_fixed_get_preferred_width(GtkWidget *widget, int *minimal, int *natural)
{
	//GtkRequisition child_requisition;
    gint w = 0;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(IS_BOX_FIXED(widget));

	BoxFixedPrivate *priv = BOX_FIXED_PRIVATE(widget);

    if(priv->child && gtk_widget_get_visible(priv->child))
		gtk_widget_get_preferred_width(priv->child, &w, NULL);
		//gtk_widget_get_preferred_size(priv->child, &child_requisition, NULL);

	//guint border_width = gtk_container_get_border_width(GTK_CONTAINER(widget));

	*minimal = *natural = w;
	//*minimal = *natural = border_width * 2 + child_requisition.width;
	//printf("get_p_width %d\n", *minimal);
}

/* Get the height of the container */
static void
box_fixed_get_preferred_height(GtkWidget *widget, int *minimal, int *natural)
{
	gint h = 0;
	g_return_if_fail(widget != NULL);
	g_return_if_fail(IS_BOX_FIXED(widget));

	BoxFixedPrivate *priv = BOX_FIXED_PRIVATE(widget);
    if(priv->child && gtk_widget_get_visible(priv->child))
		gtk_widget_get_preferred_height(priv->child, &h, NULL);
	
	*minimal = *natural = h;
	//printf("get_p_height\n");
}

/* Allocate the sizes of the container's children */
static void
box_fixed_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
	g_return_if_fail(widget != NULL || allocation != NULL);
	g_return_if_fail(IS_BOX_FIXED(widget));

	BoxFixedPrivate *priv = BOX_FIXED_PRIVATE(widget);

	//printf("+++%d, %d, %d, %d\n", allocation->x, allocation->y, allocation->width, allocation->height);
	gtk_widget_set_allocation(widget, allocation);

    if(priv->child && gtk_widget_get_visible(priv->child))
        gtk_widget_size_allocate(priv->child, allocation);

    GList *iter;
    GtkRequisition req;
    for(iter = priv->children; iter; iter = g_list_next(iter)){
        Overlay *overlay = iter->data;
        if(!gtk_widget_get_visible(overlay->widget))
            continue;

        //gtk_widget_size_request(overlay->widget, &req);
        gtk_widget_get_preferred_size(overlay->widget, &req, NULL);
        //gtk_widget_get_child_requisition(overlay->widget, &req);
        GtkAllocation child_allocation;
        child_allocation.x = overlay->x;
        child_allocation.y = overlay->y;
        if(!gtk_widget_get_has_window(widget)){
            child_allocation.x += allocation->x;
            child_allocation.y += allocation->y;
        }
        child_allocation.width = req.width;
        child_allocation.height = req.height;
        //printf("%d, %d\n", req.width, req.height);
        gtk_widget_size_allocate(overlay->widget, &child_allocation);
    }
}

/* Return the type of children this container accepts */
static GType
box_fixed_child_type(GtkContainer *container)
{
	return GTK_TYPE_WIDGET;
}

/* Add a child to the container */
static void
box_fixed_add(GtkContainer *container, GtkWidget *widget)
{
	g_return_if_fail(container || IS_BOX_FIXED(container));
	g_return_if_fail(widget || GTK_IS_WIDGET(widget));
	g_return_if_fail(gtk_widget_get_parent(widget) == NULL);

	BoxFixedPrivate *priv = BOX_FIXED_PRIVATE(container);

	/* Add the child to our list of children. 
	 * All the real work is done in gtk_widget_set_parent(). */
	priv->child = widget;
	gtk_widget_set_parent(widget, GTK_WIDGET(container));

	/* Queue redraw */
	if(gtk_widget_get_visible(widget))
		gtk_widget_queue_resize(GTK_WIDGET(container));
}

void box_fixed_put(BoxFixed *bf, GtkWidget *widget, gint x, gint y){
    g_return_if_fail(IS_BOX_FIXED(bf));
    g_return_if_fail(widget || GTK_IS_WIDGET(widget));
    g_return_if_fail(gtk_widget_get_parent(widget) == NULL);

	BoxFixedPrivate *priv = BOX_FIXED_PRIVATE(bf);

    Overlay *overlay = g_malloc(sizeof(Overlay));
    overlay->x = x;
    overlay->y = y;
    overlay->widget = widget;
    priv->children = g_list_append(priv->children, overlay);
    gtk_widget_set_parent(widget, GTK_WIDGET(bf));

    if(gtk_widget_get_visible(widget))
        gtk_widget_queue_resize(GTK_WIDGET(bf));
}

void box_fixed_move(BoxFixed *bf, GtkWidget *widget, gint x, gint y){
    g_return_if_fail(IS_BOX_FIXED(bf));

	BoxFixedPrivate *priv = BOX_FIXED_PRIVATE(bf);

    GList *children = priv->children;

    while(children){
        Overlay *overlay = children->data;
        children = children->next;
        if(overlay->widget == widget){
            overlay->x = x;
            overlay->y = y;
            if(gtk_widget_get_visible(GTK_WIDGET(bf)) && gtk_widget_get_visible(widget))
                gtk_widget_queue_resize(GTK_WIDGET(bf));
            break;
        }
    }
}


/* Remove a child from the container */
static void
box_fixed_remove(GtkContainer *container, GtkWidget *widget)
{
	g_return_if_fail(container || IS_BOX_FIXED(container));
	g_return_if_fail(widget || GTK_IS_WIDGET(widget));

	BoxFixedPrivate *priv = BOX_FIXED_PRIVATE(container);

    GList *iter = NULL;
    for(iter = priv->children; iter; iter = g_list_next(iter)){
        if(((Overlay*)iter->data)->widget == widget)
            break;
    }

    if(priv->child == widget){
        priv->child = NULL;
    }else if(iter){
        g_free(iter->data);
        priv->children = g_list_delete_link(priv->children, iter);
    }else{
        return;
    }

    gboolean was_visible = gtk_widget_get_visible(widget);
    gtk_widget_unparent(widget);
    if(was_visible)
        gtk_widget_queue_resize(GTK_WIDGET(container));
}

static void box_fixed_forall(GtkContainer *container, gboolean include_internals, GtkCallback callback, gpointer callback_data)
{
	BoxFixedPrivate *priv = BOX_FIXED_PRIVATE(container);
	if(priv->child)
		callback(priv->child, callback_data);
    if(priv->children){
        GList *iter;
        for(iter = priv->children; iter; iter = g_list_next(iter)){
            callback(((Overlay*)iter->data)->widget, callback_data);
        }
    }
}
