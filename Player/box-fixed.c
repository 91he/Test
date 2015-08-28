#include "box-fixed.h"

/* Private data structure */
struct _BoxFixedPrivate {
    GtkWidget *child;
    GList *children;
};

typedef struct _Overlay Overlay;

struct _Overlay{
    gint x, y;
    GtkWidget *widget;
};

/* Internal API */
static void box_fixed_realize (GtkWidget *widget);
static void box_fixed_size_request(GtkWidget *widget, 
        GtkRequisition *requisition);
static void box_fixed_size_allocate(GtkWidget *widget, 
        GtkAllocation *allocation);
static GType box_fixed_child_type(GtkContainer *container);
static void box_fixed_add(GtkContainer *container, GtkWidget *widget);
static void box_fixed_remove(GtkContainer *container, GtkWidget *widget);
static void box_fixed_forall(GtkContainer *container, gboolean include_internals, GtkCallback callback, gpointer callback_data);

/* Define type */
G_DEFINE_TYPE(BoxFixed, box_fixed, GTK_TYPE_CONTAINER)

/* Initialization */
static void box_fixed_class_init(BoxFixedClass *klass){

    GObjectClass *g_class;
    GtkWidgetClass *w_class;
    GtkContainerClass *c_class;

    g_class = G_OBJECT_CLASS(klass);
    w_class = GTK_WIDGET_CLASS(klass);
    c_class = GTK_CONTAINER_CLASS(klass);

    /* Override widget class methods */
    w_class->realize = box_fixed_realize;
    w_class->size_request  = box_fixed_size_request;
    w_class->size_allocate = box_fixed_size_allocate;

    c_class->child_type = box_fixed_child_type;
    c_class->add    = box_fixed_add;
    c_class->remove = box_fixed_remove;
    c_class->forall = box_fixed_forall;

    /* Add private data */
    g_type_class_add_private(g_class, sizeof(BoxFixedPrivate));
}

static void box_fixed_init(BoxFixed *bf) {

    BoxFixedPrivate *priv;

    priv = G_TYPE_INSTANCE_GET_PRIVATE(bf, BOX_TYPE_FIXED, BoxFixedPrivate);

    gtk_widget_set_has_window(GTK_WIDGET(bf), FALSE);

    priv->child = NULL;
    priv->children = NULL;

    /* Create cache for faster access */
    bf->priv = priv;
}

/* Overriden virtual methods */
static void box_fixed_realize (GtkWidget *widget){
    GdkWindowAttr attributes;
    gint attributes_mask;

    if (!gtk_widget_get_has_window(widget))
        GTK_WIDGET_CLASS(box_fixed_parent_class)->realize (widget);
    else
    {   
        gtk_widget_set_realized (widget, TRUE);

        attributes.window_type = GDK_WINDOW_CHILD;
        attributes.x = widget->allocation.x;
        attributes.y = widget->allocation.y;
        attributes.width = widget->allocation.width;
        attributes.height = widget->allocation.height;
        attributes.wclass = GDK_INPUT_OUTPUT;
        attributes.visual = gtk_widget_get_visual (widget);
        attributes.colormap = gtk_widget_get_colormap (widget);
        attributes.event_mask = gtk_widget_get_events (widget);
        attributes.event_mask |= GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK;

        attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

        widget->window = gdk_window_new (gtk_widget_get_parent_window (widget), &attributes, 
                attributes_mask);
        gdk_window_set_user_data (widget->window, widget);

        widget->style = gtk_style_attach (widget->style, widget->window);
        gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);
    }   
}


static void box_fixed_size_request(GtkWidget *widget, GtkRequisition *requisition){
    GtkRequisition req;
    BoxFixedPrivate *priv;

    priv = BOX_FIXED(widget)->priv;

    requisition->width  = 0;
    requisition->height = 0;

    if(priv->child){
        gtk_widget_size_request(priv->child, &req);
        //gtk_widget_get_child_requisition(priv->child, &req);
        requisition->width = req.width;
        requisition->height = req.height;
    }
}

static void box_fixed_size_allocate(GtkWidget *widget, GtkAllocation *allocation){
    BoxFixedPrivate *priv;

    priv = BOX_FIXED(widget)->priv;

    //gtk_widget_set_allocation(widget, allocation);

    if (gtk_widget_get_has_window(widget) && gtk_widget_get_realized(widget)) {
        gdk_window_move_resize(widget->window, allocation->x, allocation->y,
                allocation->x, allocation->y);
    }

    //printf("%d, %d, %d, %d\n", allocation->x, allocation->y, allocation->width, allocation->height);
    if(priv->child && gtk_widget_get_visible(priv->child))
        gtk_widget_size_allocate(priv->child, allocation);

    GList *iter;
    GtkRequisition req;
    for(iter = priv->children; iter; iter = g_list_next(iter)){
        Overlay *overlay = iter->data;
        if(!gtk_widget_get_visible(overlay->widget))
            continue;

        gtk_widget_size_request(overlay->widget, &req);
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

/*
static gboolean box_fixed_expose(GtkWidget *widget, 
        GdkEventExpose *event) {

    BoxFixedPrivate *priv = BOX_FIXED(widget)->priv;
    cairo_t *cr;
    gint limit;
    gint i;

    cr = gdk_cairo_create(event->window);

    cairo_translate(cr, 0, 7);

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_paint(cr);

    limit = 20 - priv->percent / 5;

    for (i = 1; i <= 20; i++) {

        if (i > limit) {
            cairo_set_source_rgb(cr, 0.6, 1.0, 0);
        } else {
            cairo_set_source_rgb(cr, 0.2, 0.4, 0);
        }

        cairo_rectangle(cr, 8,  i * 4, 30, 3);
        cairo_rectangle(cr, 42, i * 4, 30, 3);
        cairo_fill(cr);
    }

    cairo_destroy(cr);

    return TRUE;
}
*/

/* Public API */
GtkWidget *box_fixed_new(void) {
    return g_object_new(BOX_TYPE_FIXED, NULL);
}

static GType box_fixed_child_type(GtkContainer *container){
    return GTK_TYPE_WIDGET;
}

void box_fixed_put(BoxFixed *bf, GtkWidget *widget, gint x, gint y){
    g_return_if_fail(BOX_IS_FIXED(bf));
    g_return_if_fail(widget || GTK_IS_WIDGET(widget));
    g_return_if_fail(gtk_widget_get_parent(widget) == NULL);

    BoxFixedPrivate *priv = BOX_FIXED(bf)->priv;
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
    g_return_if_fail(BOX_IS_FIXED(bf));

    BoxFixedPrivate *priv = bf->priv;

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

static void box_fixed_add(GtkContainer *container, GtkWidget *widget){
    g_return_if_fail(BOX_IS_FIXED(container));
    g_return_if_fail(widget || GTK_IS_WIDGET(widget));
    g_return_if_fail(gtk_widget_get_parent(widget) == NULL);

    BoxFixedPrivate *priv = BOX_FIXED(container)->priv;

    g_return_if_fail(priv->child == NULL);

    priv->child = widget;
    gtk_widget_set_parent(widget, GTK_WIDGET(container));

    if(gtk_widget_get_visible(widget))
        gtk_widget_queue_resize(GTK_WIDGET(container));
}
static void box_fixed_remove(GtkContainer *container, GtkWidget *widget){
    g_return_if_fail(container || BOX_IS_FIXED(container));
    g_return_if_fail(widget || GTK_IS_WIDGET(widget));

    BoxFixedPrivate *priv = BOX_FIXED(container)->priv;

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
static void box_fixed_forall(GtkContainer *container, gboolean include_internals, GtkCallback callback, gpointer callback_data){
    g_return_if_fail(BOX_IS_FIXED(container));
    
    BoxFixedPrivate *priv = BOX_FIXED(container)->priv;
    if(priv->child)
        callback(priv->child, callback_data);

    if(priv->children){
        GList *iter;
        for(iter = priv->children; iter; iter = g_list_next(iter)){
            callback(((Overlay*)iter->data)->widget, callback_data);
        }
    }
}
