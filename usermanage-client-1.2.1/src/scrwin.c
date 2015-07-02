#include <gtk/gtk.h>
#include "scrwin.h"

/* Forward declarations */
static void scr_win_get_preferred_width(GtkWidget *widget, int *minimal, int *natural);
static void scr_win_get_preferred_height(GtkWidget *widget, int *minimal, int *natural);
static void scr_win_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
static GType scr_win_child_type(GtkContainer *container);
static void scr_win_add(GtkContainer *container, GtkWidget *widget);
static void scr_win_remove(GtkContainer *container, GtkWidget *widget);
static void scr_win_forall(GtkContainer *container, gboolean include_internals, GtkCallback callback, gpointer callback_data);
static gboolean scr_win_scroll_event(GtkWidget *widget, GdkEventScroll *event);
static gboolean scr_win_draw(GtkWidget *widget, cairo_t *cr);

/* Define the Scrwin type and inherit from GtkBin */
G_DEFINE_TYPE(Scrwin, scr_win, GTK_TYPE_BIN);

/* Initialize the Scrwin class */
static void
scr_win_class_init(ScrwinClass *klass)
{
	/* Override GtkWidget methods */
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	widget_class->get_preferred_width = scr_win_get_preferred_width;
	widget_class->get_preferred_height = scr_win_get_preferred_height;
	widget_class->size_allocate = scr_win_size_allocate;
	widget_class->scroll_event = scr_win_scroll_event;
	//widget_class->draw = scr_win_draw;

	/* Override GtkContainer methods */
	GtkContainerClass *container_class = GTK_CONTAINER_CLASS(klass);
	container_class->child_type = scr_win_child_type;
	container_class->add = scr_win_add;
	container_class->remove = scr_win_remove;
	container_class->forall = scr_win_forall;

	/* Add private indirection member */
	g_type_class_add_private(klass, sizeof(ScrwinPrivate));
}

/* Initialize a new PSquare instance */
static void
scr_win_init(Scrwin *swin)
{
	/* This means that PSquare doesn't supply its own GdkWindow */
	gtk_widget_set_has_window(GTK_WIDGET(swin), FALSE);
	/* Set redraw on allocate to FALSE if the top left corner of your widget
	 * doesn't change when it's resized; this saves time */
	/*gtk_widget_set_redraw_on_allocate(GTK_WIDGET(square), FALSE);*/

	/* Initialize private members */
	ScrwinPrivate *priv = SCR_WIN_PRIVATE(swin);
	priv->child = NULL;
}

/* Return a new PSquare cast to a GtkWidget */
GtkWidget *
scr_win_new()
{
	return GTK_WIDGET(g_object_new(scr_win_get_type(), NULL));
}

/* Get the width of the container */
static void
scr_win_get_preferred_width(GtkWidget *widget, int *minimal, int *natural)
{
	GtkRequisition child_requisition;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(IS_SCR_WIN(widget));

	ScrwinPrivate *priv = SCR_WIN_PRIVATE(widget);

	child_requisition.width = 0;
	child_requisition.height = 0;
	if(priv->child && gtk_widget_get_visible(priv->child)){
		gtk_widget_get_preferred_size(priv->child, &child_requisition, NULL);
	}

	guint border_width = gtk_container_get_border_width(GTK_CONTAINER(widget));

	*minimal = *natural = border_width * 2 + child_requisition.width - 120;
	//printf("get_p_width %d\n", *minimal);
}

/* Get the height of the container */
static void
scr_win_get_preferred_height(GtkWidget *widget, int *minimal, int *natural)
{
//	GtkWidget *parent;
	g_return_if_fail(widget != NULL);
	g_return_if_fail(IS_SCR_WIN(widget));

//	parent = gtk_widget_get_parent(widget);
	
	//printf("get_p_height\n");
	//get_size(P_SQUARE(widget), GTK_ORIENTATION_VERTICAL, minimal, natural);
}

/* Allocate the sizes of the container's children */
static void
scr_win_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
	g_return_if_fail(widget != NULL || allocation != NULL);
	g_return_if_fail(IS_SCR_WIN(widget));

	ScrwinPrivate *priv = SCR_WIN_PRIVATE(widget);

	//printf("+++%d, %d, %d, %d\n", allocation->x, allocation->y, allocation->width, allocation->height);
	gtk_widget_set_allocation(widget, allocation);

	/* Calculate how much extra space we need */
	unsigned border_width =
		gtk_container_get_border_width(GTK_CONTAINER(widget));
	int extra_width = allocation->width - 2 * border_width;
	int extra_height = allocation->height - 2 * border_width;

	/* Start positioning the items at the container's origin,
	 * less the border width */
	int x = allocation->x + border_width;
	int y = allocation->y + border_width;

	GtkAllocation child_allocation;
	child_allocation.x = x;
	child_allocation.y = y;
	child_allocation.width = extra_width;
	child_allocation.height = extra_height;
	//printf("---%d, %d, %d, %d\n", x, y, extra_width, extra_height);

	gtk_widget_size_allocate(priv->child, &child_allocation);
}

/* Return the type of children this container accepts */
static GType
scr_win_child_type(GtkContainer *container)
{
	return GTK_TYPE_WIDGET;
}

/* Add a child to the container */
static void
scr_win_add(GtkContainer *container, GtkWidget *widget)
{
	g_return_if_fail(container || IS_SCR_WIN(container));
	g_return_if_fail(widget || GTK_IS_WIDGET(widget));
	g_return_if_fail(gtk_widget_get_parent(widget) == NULL);

	ScrwinPrivate *priv = SCR_WIN_PRIVATE(container);

	/* Add the child to our list of children. 
	 * All the real work is done in gtk_widget_set_parent(). */
	priv->child = widget;
	gtk_widget_set_parent(widget, GTK_WIDGET(container));

	/* Queue redraw */
	if(gtk_widget_get_visible(widget))
		gtk_widget_queue_resize(GTK_WIDGET(container));
}

/* Remove a child from the container */
static void
scr_win_remove(GtkContainer *container, GtkWidget *widget)
{
	g_return_if_fail(container || IS_SCR_WIN(container));
	g_return_if_fail(widget || GTK_IS_WIDGET(widget));

	ScrwinPrivate *priv = SCR_WIN_PRIVATE(container);

	gboolean was_visible = gtk_widget_get_visible(widget);
	gtk_widget_unparent(widget);
	if(was_visible)
		gtk_widget_queue_resize(GTK_WIDGET(container));
}

static void scr_win_forall(GtkContainer *container, gboolean include_internals, GtkCallback callback, gpointer callback_data)
{
	ScrwinPrivate *priv = SCR_WIN_PRIVATE(container);
	if(priv->child)
		callback(priv->child, callback_data);
}

static gboolean scr_win_scroll_event(GtkWidget *widget, GdkEventScroll *event){
	gdouble x, y;

	GtkWidget *vport = SCR_WIN_PRIVATE(widget)->child;
	GtkAdjustment *adj = gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(vport));

	gdouble l = gtk_adjustment_get_lower(adj);
	gdouble u = gtk_adjustment_get_upper(adj);
	gdouble page_size = gtk_adjustment_get_page_size(adj);

	if(u - page_size <= 0) return FALSE;

	gdk_event_get_scroll_deltas((GdkEvent*)event, &x, &y);
	//gdouble value = gtk_adjustment_get_value(adj) + y * (page_size / 5);
	gdouble value = gtk_adjustment_get_value(adj) + y * 25;

	if(value < l) value = l;
	else if(value > u - page_size) value = u - page_size;

	gtk_adjustment_set_value(adj, value);

	return TRUE;
}


static gboolean scr_win_draw(GtkWidget *widget, cairo_t *cr){
	static gdouble len = 0; 
	int w = gtk_widget_get_allocated_width(widget);
	int h = gtk_widget_get_allocated_height(widget);

	GtkWidget *vport = SCR_WIN_PRIVATE(widget)->child;
	GtkAdjustment *adj = gtk_scrollable_get_hadjustment(GTK_SCROLLABLE(vport));

	GtkStyleContext *context = gtk_widget_get_style_context(widget);
	if(0 == len){
		gtk_style_context_add_class(context, "scrwin");
		gdouble u = gtk_adjustment_get_upper(adj);
		if(w < u) len = (w / u) * w;
		else len = -1;
	}
	gtk_render_background(context, cr, 0, 0, w, h);
	if(len > 0){
		gdouble value = gtk_adjustment_get_value(adj);
		value *= len / w;
		//gtk_render_background(context, cr, 0, h-3, len, 3);
		gtk_render_frame(context, cr, value, h-3, (int)len, 3);
	}
	return FALSE;
}
