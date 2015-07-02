#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <curl/curl.h>
#include "pool.h"
#include "cJSON.h"
#include "scrwin.h"

GtkWidget *notebook;
GtkWidget *infobook;
GtkWidget *list_view = NULL;
GtkWidget *info_label;
GtkWidget *prompt_widget;
GtkWidget *prompt_label;
GtkWidget *cur_win;
GtkWidget *list_widget;
GtkTreeIter cur_iter;
GtkTreeIter del_iter;
GtkListStore *user_model = NULL;
struct thread_pool *gtk_pool = NULL;
static int sfd = 0;
static int remote = 0;
char loguser[64] = "root";
char conf_path[512];
//char opt_path[512];
char regmask[512];
char server_head_url[128];

struct{
	GdkPixbuf *small;
	GdkPixbuf *big;
}def_head;

struct{
	GtkWidget *head_image;
	GtkWidget *nick_label;
	GtkWidget *name_label;
	GtkWidget *mail_label;
}info_widgets;

struct StoreInfo{
	const char *mail;
	GtkListStore *store;
	GtkTreeIter iter;
	GdkPixbuf *small;
	GdkPixbuf *big;
};

struct{
	char name[64];
	char mail[64];
}req_infos[4];


enum{
	COL_IMAGE_SMALL,
	COL_IMAGE_BIG,
	COL_NAME,
	COL_MAIL,
	NUM_COLS
};

struct required_widgets{
	GtkWidget *name;
	GtkWidget *pwd;
	GtkWidget *pwd1;
	GtkWidget *pwd2;
	GtkWidget *mail;
	GtkWidget *other;
};

struct animation{
	gboolean showing;
	gboolean hiding;
	int ani_margin;
	int cur_margin;
	int frame_count;
	GtkWidget *animate_widget;
	GtkWidget *scrwin;
};

struct EditorDrawContext{
	int x, y; //pointer coordinate
	int w, h; //image width/height
	int cx, cy; //cairo translate x/y
	int cw, ch; //cairo width/height
	int rx, ry; //square start point
	int rw, rh; //square width/height
	int start; //is button pressed
	int in_big; //is pointer in square
	int in_small; //is pointer in sizing square
	int sizing_start; //is sizing
	double scale; //cairo surface scale value
	GdkCursor *fleur; //fleur cursor
	GdkCursor *sizing; //sizing cursor
	GdkCursor *left_ptr; //left_ptr_cursor
	cairo_surface_t *surface; //origin image surface
	cairo_surface_t *preview; //preview image surface
	GdkPixbuf *pixbuf; //the pixbuf showing
	GtkWidget *widget; //the drawing_area widget
	GtkWidget *preview_widget; //the preview widget show the head image
};

GtkWidget *um_log_win();

int readn(int fd, char *buf, int len){
	int r, sum = 0;
	while(len){
		r = read(fd, buf + sum, len);
		if(r <= 0){
			perror("read");
			sfd = -1;
			return r;
		}
		sum += r;
		len -= r;
	}
	return sum;
}

int writen(int fd, char *buf, int len){
	int r, sum = 0;
	while(len){
		r = write(fd, buf + sum, len);
		if(r <= 0){
			perror("write");
			sfd = -1;
			return r;
		}
		sum += r;
		len -= r;
	}
	return sum;
}

int sendreq(int fd, char *buf){
	int r;
	int len = strlen(buf) + 1;
	if((r = writen(fd, (char*)&len, sizeof(int))) <= 0){
		return r;
	}
	if((r = writen(fd, buf, len)) <= 0){
		return r;
	}
}

/*
  * This function is used to set the CSS style
  * of the application.
  * theme_path: the path of css file.
  */
static void app_set_theme(const gchar *theme_path)
{
	static GtkCssProvider *provider = NULL;
	GFile *file;
	GdkScreen *screen;
	screen = gdk_screen_get_default();
	if(theme_path!=NULL)
	{
		file = g_file_new_for_path(theme_path);
		if(file!=NULL)
		{
			if(provider==NULL)
				provider = gtk_css_provider_new();
			gtk_css_provider_load_from_file(provider, file, NULL);
			gtk_style_context_add_provider_for_screen(screen,
					GTK_STYLE_PROVIDER(provider),
					GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
					//GTK_STYLE_PROVIDER_PRIORITY_USER);
			gtk_style_context_reset_widgets(screen);
		}
	}
	else
	{
		if(provider!=NULL)
		{
			gtk_style_context_remove_provider_for_screen(screen,
					GTK_STYLE_PROVIDER(provider));
			g_object_unref(provider);
			provider = NULL;
		}
		gtk_style_context_reset_widgets(screen);
	}
}

int con_with_server(char *ip){
	int fd = socket(PF_INET, SOCK_STREAM, 0);
	if(fd < 0){
		perror("socket");
		return -1;
	}
	struct sockaddr_in addr;
	addr.sin_family = PF_INET;
	addr.sin_port = htons(3210);
	addr.sin_addr.s_addr = inet_addr(ip);
	int r = connect(fd, (struct sockaddr*)&addr, sizeof(addr));
	if(r < 0){
		close(fd);
		perror("connect");
		return -1;
	}
	return fd;
}

void add_class(GtkWidget *widget, gchar *class_name){
	GtkStyleContext *context = gtk_widget_get_style_context(widget);
	if(!gtk_style_context_has_class(context, class_name)){
		gtk_style_context_add_class(context, class_name);
		gtk_widget_queue_draw(widget);
	}
}

void remove_class(GtkWidget *widget, gchar *class_name){
	GtkStyleContext *context = gtk_widget_get_style_context(widget);
	if(gtk_style_context_has_class(context, class_name)){
		gtk_style_context_remove_class(context, class_name);
		gtk_widget_queue_draw(widget);
	}
}

gboolean draw_border(GtkWidget *widget, cairo_t *cr){
	GtkStyleContext *context = gtk_widget_get_style_context(widget);

	gtk_render_frame(context, cr, 0, 0, 
					 gtk_widget_get_allocated_width(widget),
					 gtk_widget_get_allocated_height(widget));
	return FALSE;
}

void remove_focus(GtkWindow *window, GtkWidget *widget, gpointer user_data){
	gtk_window_set_focus_visible(GTK_WINDOW(window), FALSE);
}

/*
gboolean selection_clicked(GtkWidget *widget, gpointer user_data){
	char *nums = user_data;
	GtkWidget **psbtn = (GtkWidget**)(nums + (8 - *nums));

	remove_class(GTK_WIDGET(*psbtn), "selection_focused");
	add_class(widget, "selection_focused");
	*psbtn = widget;

	gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), *nums);
	
	return FALSE;
}
*/
gboolean selection_clicked(GtkWidget *widget, GdkEvent *event, gpointer user_data){
	char *nums = user_data;
	GtkWidget **psbtn = (GtkWidget**)(nums + (8 - *nums));

	remove_class(GTK_WIDGET(*psbtn), "selection_focused");
	add_class(widget, "selection_focused");
	*psbtn = widget;

	gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), *nums);
	gtk_label_set_text(GTK_LABEL(info_label), gtk_button_get_label(GTK_BUTTON(widget)));
	/*
	if(*nums > 2)
		gtk_widget_hide(list_widget);
	else
		gtk_widget_show(list_widget);
		*/
	
	return FALSE;
}

GtkWidget *um_selection_button_box(){
	GtkWidget *ebox;
	GtkWidget *box;
	static struct{
		char nums[8];
		GtkWidget *selected_btn;
	}selection_data;

	const gchar *texts[] = {" 用户信息", " 修改密码", " 权限管理",
	   					   " 同步账户", " 创建账户", " 本地创建"};
	const gchar *icons[] = {"/opt/usermanageclient/user.png", "/opt/usermanageclient/passwd.png", "/opt/usermanageclient/auth.png",
	   						"/opt/usermanageclient/sync.png", "/opt/usermanageclient/create.png", "/opt/usermanageclient/create.png"};
	int i, n;

	ebox = gtk_event_box_new();

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(ebox), box);
	add_class(ebox, "selection");

	n = strcmp("root", loguser) ? 2 : 6;

	for(i = 0; i < n; ++i){
		selection_data.nums[i] = i;
		GtkWidget *btn = gtk_button_new_with_label(texts[i]);
		//add_class(btn, "sbox");
		//g_signal_connect(btn, "clicked", G_CALLBACK(selection_clicked), &selection_data.nums[i]);
		g_signal_connect(btn, "button-press-event", G_CALLBACK(selection_clicked), &selection_data.nums[i]);
		GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size(icons[i], 20, 20, NULL);
		GtkWidget *image = gtk_image_new_from_pixbuf(pixbuf);

		gtk_widget_set_size_request(btn, 0, 40);
		gtk_button_set_image(GTK_BUTTON(btn), image);
		gtk_button_set_always_show_image(GTK_BUTTON(btn), TRUE);
		gtk_box_pack_start(GTK_BOX(box), btn, FALSE, FALSE, 0);
		if(0 == i){
			selection_data.selected_btn = btn;
			add_class(btn, "selection_focused");
		}
	}

	return ebox;
}

gboolean motion_start(GtkWidget *widget, GdkEvent *event, gpointer user_data){
	GtkWidget *window;
	int *crd = user_data;
	
	window = gtk_widget_get_toplevel(widget);
	if(GDK_BUTTON_PRESS == event->type){
		gtk_window_get_position(GTK_WINDOW(window), crd, crd + 1);
		crd[0] -= event->button.x_root;
		crd[1] -= event->button.y_root;
	}
	return FALSE;
}

gboolean motion_changing(GtkWidget *widget, GdkEvent *event, gpointer user_data){
	GtkWidget *window;
	int *crd = user_data;

	window = gtk_widget_get_toplevel(widget);
	gtk_window_move(GTK_WINDOW(window),
		   			(int)(crd[0] + event->button.x_root),
					(int)(crd[1] + event->button.y_root));

	return FALSE;
}

gboolean client_minimize(GtkWidget *widget, gpointer user_data){
	gtk_window_iconify(GTK_WINDOW(gtk_widget_get_toplevel(widget)));
	return FALSE;
};

GtkWidget *um_info_bar(const gchar *text){
	GtkWidget *info_bar;
	GtkWidget *box;
	GtkWidget *close;
	GtkWidget *minimize;
	GtkWidget *label;
	GdkPixbuf *pixbuf;
	static int crd[2];
	
	info_bar = gtk_event_box_new();
	gtk_widget_set_size_request(info_bar, 0, 37);
	add_class(info_bar, "info_bar");
	g_signal_connect(info_bar, "button-press-event", G_CALLBACK(motion_start), crd);
	g_signal_connect(info_bar, "motion-notify-event", G_CALLBACK(motion_changing), crd);

	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

	close = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(close), GTK_RELIEF_NONE);
	gtk_widget_set_size_request(close, 33, 0);
	g_signal_connect(close, "clicked", G_CALLBACK(gtk_main_quit), NULL);

	pixbuf = gdk_pixbuf_new_from_file_at_size("/opt/usermanageclient/close.png", 10, 10, NULL);
	gtk_button_set_image(GTK_BUTTON(close), gtk_image_new_from_pixbuf(pixbuf));

	minimize = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(minimize), GTK_RELIEF_NONE);
	gtk_widget_set_size_request(minimize, 33, 0);
	g_signal_connect(minimize, "clicked", G_CALLBACK(client_minimize), NULL);

	pixbuf = gdk_pixbuf_new_from_file_at_size("/opt/usermanageclient/minimize.png", 10, 10, NULL);
	gtk_button_set_image(GTK_BUTTON(minimize), gtk_image_new_from_pixbuf(pixbuf));
	
	info_label = label = gtk_label_new(text);

	gtk_box_pack_end(GTK_BOX(box), close, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(box), minimize, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);

	gtk_container_add(GTK_CONTAINER(info_bar), box);
	gtk_widget_show_all(info_bar);

	return info_bar;
}

GtkWidget *um_icon_bar(){
	GtkWidget *icon_bar;
	static int crd[2];
	
	icon_bar = gtk_event_box_new();

	gtk_widget_set_size_request(icon_bar, 150, 36);
	add_class(icon_bar, "small_icon");
	g_signal_connect(icon_bar, "button-press-event", G_CALLBACK(motion_start), crd);
	g_signal_connect(icon_bar, "motion-notify-event", G_CALLBACK(motion_changing), crd);

	gtk_widget_show_all(icon_bar);

	return icon_bar;
}

GtkWidget *justify_label(int width, int n, ...){
	int i;
	char *tmp;
	va_list ap;
	int per_width[3];
	GtkWidget *box;

	per_width[0] = per_width[1] = per_width[2] = width / n;
	if((width % n) == 2){
		per_width[0] = ++per_width[2];
	}else if((width % n) == 1){
		if(n == 2){
			per_width[0]++;
		}else{
			per_width[1]++;
		}
	}

	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

	va_start(ap, n);
	for(i = 0; i < n; i++){
		GtkWidget *label = gtk_label_new(va_arg(ap, char*));
		gtk_widget_set_size_request(label, per_width[i], 0);
		if(!i)
			gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
		else if(n == i + 1)
			gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);
		//printf("%d\n", per_width[i]);
		gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
	}
	va_end(ap);
	
	return box;
}

gboolean register_send(GtkWidget *widget, gpointer user_data){
	char prompt_message[32] = {0};
	const gchar *regname, *passwd, *ensurepasswd, *email;
	struct required_widgets *data = user_data;

	regname = gtk_entry_get_text(GTK_ENTRY(data->name));
	passwd = gtk_entry_get_text(GTK_ENTRY(data->pwd1));
	ensurepasswd = gtk_entry_get_text(GTK_ENTRY(data->pwd2));

	if(!*regname){
		add_class(data->name, "red_border");
		return FALSE;
	}
	if(!*passwd){
		add_class(data->pwd1, "red_border");
		return FALSE;
	}
	if(!*ensurepasswd){
		add_class(data->pwd2, "red_border");
		return FALSE;
	}
	if(strcmp(passwd, ensurepasswd)){
		sprintf(prompt_message, "两次输入密码不一致!");
		goto error;
	}
	if(strlen(passwd) < 6){
		sprintf(prompt_message, "密码长度不能小于6位!");
		goto error;
	}

	cJSON *obj = cJSON_CreateObject();
	cJSON *jtype = cJSON_CreateString("3");
	cJSON *logname = cJSON_CreateString(loguser);

	cJSON *jname = cJSON_CreateString(regname);
	cJSON *jpasswd = cJSON_CreateString(passwd);
	cJSON_AddItemToObject(obj, "type", jtype);
	cJSON_AddItemToObject(obj, "uname", logname);
	cJSON_AddItemToObject(obj, "newname", jname);
	cJSON_AddItemToObject(obj, "newpwd", jpasswd);
	remote = 0;
	if(data->mail){
		remote = 1;
		email = gtk_entry_get_text(GTK_ENTRY(data->mail));
		if(!*email){
			add_class(data->mail, "red_border");
			cJSON_Delete(obj);
			goto error;
		}
		cJSON *jemail = cJSON_CreateString(email);
		cJSON_AddItemToObject(obj, "email", jemail);
	}
	if(remote){
		strcpy(req_infos[1].name, regname);
		strcpy(req_infos[1].mail, email);
	}else{
		strcpy(req_infos[2].name, regname);
		strcpy(req_infos[2].mail, "————");
	}

	char * buf = cJSON_Print(obj);
	sendreq(sfd, buf);
	free(buf);

	cJSON_Delete(obj);

	return TRUE;
error:
	if(*prompt_message){
		if(prompt_widget){
			gtk_label_set_text(GTK_LABEL(prompt_label), prompt_message);
			gtk_widget_show(prompt_widget);
		}else{
			GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE, "%s", prompt_message);
			gtk_widget_show(dialog);
			g_signal_connect_swapped(dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);
		}
	}
	return FALSE;
}

gboolean rm_red_border(GtkWidget *widget, GdkEvent *event, gpointer user_data){
	remove_class(widget, "red_border");
	return FALSE;
}

GtkWidget *um_reg_page(gboolean remote){
	struct required_widgets *req_widgets = malloc(sizeof(struct required_widgets));
	GtkWidget *reg_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
	gtk_widget_set_valign(reg_box, GTK_ALIGN_CENTER);
	//gtk_widget_set_margin_top(reg_box, 50);
	bzero(req_widgets, sizeof(struct required_widgets));

	GtkWidget *regname_box = justify_label(64, 3, "用", "户", "名");
	gtk_widget_set_halign(regname_box, GTK_ALIGN_CENTER);
	GtkWidget *regname_entry = gtk_entry_new();
	req_widgets->name = regname_entry;
	gtk_widget_set_size_request(regname_entry, 220, 30);
	gtk_box_pack_start(GTK_BOX(regname_box), regname_entry, FALSE, FALSE, 10);
	
	GtkWidget *regpasswd_box = justify_label(64, 2, "密", "码");
	gtk_widget_set_halign(regpasswd_box, GTK_ALIGN_CENTER);
	GtkWidget *regpasswd_entry = gtk_entry_new();
	req_widgets->pwd1 = regpasswd_entry;
	gtk_entry_set_visibility(GTK_ENTRY(regpasswd_entry), FALSE);
	gtk_widget_set_size_request(regpasswd_entry, 220, 30);
	gtk_box_pack_start(GTK_BOX(regpasswd_box), regpasswd_entry, FALSE, FALSE, 10);

	GtkWidget *ensurepasswd_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_set_halign(ensurepasswd_box, GTK_ALIGN_CENTER);
	GtkWidget *ensurepasswd_label = gtk_label_new("确认密码");
	//gtk_misc_set_alignment(GTK_MISC(ensurepasswd_label), 1, 0.5);
	//gtk_widget_set_size_request(ensurepasswd_label, 64, 0);

	GtkWidget *ensurepasswd_entry = gtk_entry_new();
	req_widgets->pwd2 = ensurepasswd_entry;
	gtk_entry_set_visibility(GTK_ENTRY(ensurepasswd_entry), FALSE);
	gtk_widget_set_size_request(ensurepasswd_entry, 220, 30);
	gtk_entry_set_visibility(GTK_ENTRY(ensurepasswd_entry), FALSE);

	gtk_box_pack_end(GTK_BOX(ensurepasswd_box), ensurepasswd_entry, FALSE, FALSE, 10);
	gtk_box_pack_end(GTK_BOX(ensurepasswd_box), ensurepasswd_label, FALSE, FALSE, 0);

	GtkWidget *regemail_box = NULL;
	GtkWidget *regemail_entry = NULL;
	if(remote){
		regemail_box = justify_label(64, 2, "邮", "箱");
		gtk_widget_set_halign(regemail_box, GTK_ALIGN_CENTER);

		regemail_entry = gtk_entry_new();
		req_widgets->mail = regemail_entry;
		gtk_widget_set_size_request(regemail_entry, 220, 30);

		gtk_box_pack_start(GTK_BOX(regemail_box), regemail_entry, FALSE, FALSE, 10);
		g_signal_connect(req_widgets->mail, "button-press-event", G_CALLBACK(rm_red_border), NULL);
	}
	g_signal_connect(req_widgets->name, "button-press-event", G_CALLBACK(rm_red_border), NULL);
	g_signal_connect(req_widgets->pwd1, "button-press-event", G_CALLBACK(rm_red_border), NULL);
	g_signal_connect(req_widgets->pwd2, "button-press-event", G_CALLBACK(rm_red_border), NULL);

	GtkWidget *regbtn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	GtkWidget *reg_button = gtk_button_new_with_label("注 册");
	add_class(reg_button, "reg_btn");
	g_signal_connect(reg_button, "clicked", G_CALLBACK(register_send), req_widgets);
	gtk_widget_set_halign(regbtn_box, GTK_ALIGN_CENTER);
	gtk_widget_set_margin_left(reg_button, 194);
	gtk_widget_set_margin_right(reg_button, 10);
	gtk_widget_set_size_request(reg_button, 100, 0);
	gtk_box_pack_start(GTK_BOX(regbtn_box), reg_button, FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(reg_box), regname_box, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(reg_box), regpasswd_box, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(reg_box), ensurepasswd_box, FALSE, FALSE, 0);
	if(remote){
		gtk_box_pack_start(GTK_BOX(reg_box), regemail_box, FALSE, FALSE, 0);
	}
	gtk_box_pack_start(GTK_BOX(reg_box), regbtn_box, FALSE, FALSE, 0);
	gtk_widget_show_all(reg_box);

	return reg_box;
}

gboolean sync_cb(GtkWidget *widget, gpointer user_data){
	char *buf;
	cJSON *obj;
	struct required_widgets *data = user_data;
	const gchar *name = gtk_entry_get_text(GTK_ENTRY(data->name));
	const gchar *mail = gtk_entry_get_text(GTK_ENTRY(data->mail));
	const gchar *pwd = gtk_entry_get_text(GTK_ENTRY(data->pwd));

	if(!*name){
		add_class(data->name, "red_border");
		return FALSE;
	}
	if(!*mail){
		add_class(data->mail, "red_border");
		return FALSE;
	}
	if(!*pwd){
		add_class(data->pwd, "red_border");
		return FALSE;
	}
	strcpy(req_infos[0].name, name);
	strcpy(req_infos[0].mail, mail);

	obj = cJSON_CreateObject();
	cJSON *jtype = cJSON_CreateString("5");
	cJSON *juser = cJSON_CreateString(loguser);
	cJSON *jname = cJSON_CreateString(name);
	cJSON *jmail = cJSON_CreateString(mail);
	cJSON *jpwd = cJSON_CreateString(pwd);

	cJSON_AddItemToObject(obj, "type", jtype);
	cJSON_AddItemToObject(obj, "uname", juser);
	cJSON_AddItemToObject(obj, "syncname", jname);
	cJSON_AddItemToObject(obj, "email", jmail);
	cJSON_AddItemToObject(obj, "passwd", jpwd);

	buf = cJSON_Print(obj);
	sendreq(sfd, buf);
	free(buf);

	cJSON_Delete(obj);

	return FALSE;
}

GtkWidget *um_sync_page(){
	GtkWidget *box;
	GtkWidget *name_box;
	GtkWidget *mail_box;
	GtkWidget *pwd_box;
	GtkWidget *btn_box;
	GtkWidget *entry;
	GtkWidget *button;
	static struct required_widgets req_widgets;

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
	gtk_widget_set_valign(box, GTK_ALIGN_CENTER);

	name_box = justify_label(60, 3, "用", "户", "名");
	gtk_widget_set_halign(name_box, GTK_ALIGN_CENTER);
	entry = gtk_entry_new();
	req_widgets.name = entry;
	gtk_widget_set_size_request(entry, 220, 30);
	gtk_box_pack_start(GTK_BOX(name_box), entry, FALSE, FALSE, 10);

	mail_box = justify_label(60, 2, "邮", "箱");
	gtk_widget_set_halign(mail_box, GTK_ALIGN_CENTER);
	entry = gtk_entry_new();
	req_widgets.mail = entry;
	gtk_widget_set_size_request(entry, 220, 30);
	gtk_box_pack_start(GTK_BOX(mail_box), entry, FALSE, FALSE, 10);

	pwd_box = justify_label(60, 2, "密", "码");
	gtk_widget_set_halign(pwd_box, GTK_ALIGN_CENTER);
	entry = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE);
	req_widgets.pwd = entry;
	gtk_widget_set_size_request(entry, 220, 30);
	gtk_box_pack_start(GTK_BOX(pwd_box), entry, FALSE, FALSE, 10);

	g_signal_connect(req_widgets.name, "button-press-event", G_CALLBACK(rm_red_border), NULL);
	g_signal_connect(req_widgets.mail, "button-press-event", G_CALLBACK(rm_red_border), NULL);
	g_signal_connect(req_widgets.pwd, "button-press-event", G_CALLBACK(rm_red_border), NULL);

	btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);	
	button = gtk_button_new_with_label("同 步");
	add_class(button, "reg_btn");
	g_signal_connect(button, "clicked", G_CALLBACK(sync_cb), &req_widgets);
	gtk_widget_set_halign(btn_box, GTK_ALIGN_CENTER);
	gtk_widget_set_margin_left(button, 194);
	gtk_widget_set_margin_right(button, 10);
	gtk_widget_set_size_request(button, 100, 0);
	gtk_box_pack_start(GTK_BOX(btn_box), button, FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(box), name_box, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), mail_box, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), pwd_box, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), btn_box, FALSE, FALSE, 0);
	gtk_widget_show_all(box);

	return box;
}

void edit_head_image(GtkButton *button, gpointer user_data){
	gtk_notebook_set_current_page(GTK_NOTEBOOK(user_data), 1);
	gtk_widget_hide(list_widget);
	//gtk_label_set_text(GTK_LABEL(info_label), "自定义头像");
}

GtkWidget *um_info_page(gpointer user_data){
	GtkWidget *box;
	GtkWidget *head_box;
	GtkWidget *nick_box;
	GtkWidget *name_box;
	GtkWidget *mail_box;
	GtkWidget *button;
	GtkWidget *arrow;
	GtkWidget *separator;
	GdkPixbuf *pixbuf;

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	head_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

	//pixbuf = gdk_pixbuf_new_from_file_at_size("default.png", 75, 75, NULL);
	//info_widgets.head_image = gtk_image_new_from_pixbuf(pixbuf);
	info_widgets.head_image = gtk_image_new();
	gtk_box_pack_start(GTK_BOX(head_box), info_widgets.head_image, FALSE, FALSE, 0);

	pixbuf = gdk_pixbuf_new_from_file("/opt/usermanageclient/arrow.png", NULL);
	arrow = gtk_image_new_from_pixbuf(pixbuf);
	if(strcmp(loguser, "root")){
		button = gtk_button_new_with_label("修  改 ");
		add_class(button, "theme_color");
		g_signal_connect(button, "clicked", G_CALLBACK(edit_head_image), user_data);
		gtk_button_set_image_position(GTK_BUTTON(button), GTK_POS_RIGHT);
		gtk_button_set_image(GTK_BUTTON(button), arrow);
		gtk_button_set_always_show_image(GTK_BUTTON(button), TRUE);
		gtk_box_pack_end(GTK_BOX(head_box), button, FALSE, FALSE, 0);
	}

	nick_box = justify_label(60, 2, "昵", "称");
	info_widgets.nick_label = gtk_label_new("");
	gtk_box_pack_end(GTK_BOX(nick_box), info_widgets.nick_label, FALSE, FALSE, 0);

	name_box = justify_label(60, 3, "用", "户", "名");
	info_widgets.name_label = gtk_label_new("");
	gtk_box_pack_end(GTK_BOX(name_box), info_widgets.name_label, FALSE, FALSE, 0);

	mail_box = justify_label(60, 2, "邮", "箱");
	info_widgets.mail_label = gtk_label_new("");
	gtk_box_pack_end(GTK_BOX(mail_box), info_widgets.mail_label, FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(box), head_box, FALSE, FALSE, 0);
	separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	add_class(separator, "black");
	gtk_widget_set_margin_top(separator, 10);
	gtk_widget_set_margin_bottom(separator, 14);
	gtk_box_pack_start(GTK_BOX(box), separator, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), nick_box, FALSE, FALSE, 0);
	separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	add_class(separator, "black");
	gtk_widget_set_margin_top(separator, 10);
	gtk_widget_set_margin_top(separator, 14);
	gtk_widget_set_margin_bottom(separator, 14);
	gtk_box_pack_start(GTK_BOX(box), separator, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), name_box, FALSE, FALSE, 0);
	separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	add_class(separator, "black");
	gtk_widget_set_margin_top(separator, 10);
	gtk_widget_set_margin_top(separator, 14);
	gtk_widget_set_margin_bottom(separator, 14);
	gtk_box_pack_start(GTK_BOX(box), separator, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), mail_box, FALSE, FALSE, 0);
	separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	add_class(separator, "black");
	gtk_widget_set_margin_top(separator, 10);
	gtk_widget_set_margin_top(separator, 14);
	gtk_box_pack_start(GTK_BOX(box), separator, FALSE, FALSE, 0);
	gtk_widget_show_all(box);
	gtk_widget_set_margin_left(box, 10);
	gtk_widget_set_margin_top(box, 10);
	gtk_widget_set_margin_right(box, 50);

	return box;
}

gboolean head_rec_press(GtkWidget *widget, GdkEvent *event, gpointer user_data){
	struct EditorDrawContext *context = user_data;

	context->x = event->button.x;
	context->y = event->button.y;
	context->start = 1;

	return FALSE;
}

gboolean head_rec_release(GtkWidget *widget, GdkEvent *event, gpointer user_data){
	struct EditorDrawContext *context = user_data;

	context->start = 0;
	context->sizing_start = 0;

	return FALSE;
}

gboolean head_rec_move(GtkWidget *widget, GdkEventMotion *event, gpointer user_data){
	int x, y, w, h, r;
	GdkModifierType state;
	struct EditorDrawContext *context = user_data;

	if(!context->pixbuf) return FALSE;

	gdk_window_get_device_position (event->window, event->device, &x, &y, &state);

	if(!context->start){
		r = x >= (context->rx + context->rw - 4) && x <= (context->rx +context->rw + 4) && y >= (context->ry +context->rh - 4) && y <= (context->ry + context->rh + 4);
		if((r ^ context->in_small)){
			if(!context->in_small){
				gdk_window_set_cursor(gtk_widget_get_window(widget), context->sizing);
				context->in_big = 0;
				context->in_small = 1;
			}else{
				r = x >= context->rx && x <= (context->rx +context->rw) && y >= context->ry && y <= (context->ry + context->rh);
				if(r){
					gdk_window_set_cursor(gtk_widget_get_window(widget), context->fleur);
				}else{
					gdk_window_set_cursor(gtk_widget_get_window(widget), context->left_ptr);
				}
				context->in_big = r;
				context->in_small = 0;
			}
		}else if(!context->in_small){
			r = x >= context->rx && x <= (context->rx +context->rw) && y >= context->ry && y <= (context->ry + context->rh);
			if((r ^ context->in_big)){
				if(r){
					gdk_window_set_cursor(gtk_widget_get_window(widget), context->fleur);
				}else{
					gdk_window_set_cursor(gtk_widget_get_window(widget), context->left_ptr);
				}
				context->in_big = r;
			}
		}
	}

	if((state & GDK_BUTTON1_MASK) && context->start){
		if(context->in_small || context->sizing_start){
			context->sizing_start = 1;
			w = context->cx + context->cw - context->rx;
			h = context->cy + context->ch - context->ry;

			context->rw = x - context->rx;
			context->rh = y - context->ry;

			context->rw = context->rw >= 50 ? context->rw : 50;
			//context->rw = context->rw <= 300 ? context->rw : 300;

			context->rh = context->rh >= 50 ? context->rh : 50;
			//context->rh = context->rh <= 300 ? context->rh : 300;

			context->rh = context->rw = context->rw > context->rh ? context->rw : context->rh;

			context->rw = context->rw <= w ? context->rw : w; 
			context->rh = context->rh <= h ? context->rh : h; 

			context->rh = context->rw = context->rw < context->rh ? context->rw : context->rh;
		}else if(context->in_big){
			w = context->cx + context->cw - context->rw;
			h = context->cy + context->ch - context->rh;

			context->rx += x - context->x;
			context->ry += y - context->y;
			context->x = x;
			context->y = y;

			context->rx = context->rx >= context->cx ? context->rx : context->cx;
			context->rx = context->rx <= w ? context->rx : w;

			context->ry = context->ry >= context->cy ? context->ry : context->cy;
			context->ry = context->ry <= h ? context->ry : h;
		}
		//printf("%d, %d, %d, %d\n", context->rx, context->ry, context->rw, context->rh);
		cairo_surface_destroy(context->preview);
		context->preview = cairo_surface_create_for_rectangle(context->surface,
				   												  (int)((context->rx - context->cx) / context->scale),
																  (int)((context->ry - context->cy) / context->scale),
																  context->rw / context->scale,
																  context->rh / context->scale);
		gtk_widget_queue_draw(widget);
		gtk_widget_queue_draw(context->preview_widget);
	}

	return FALSE;
}

unsigned char *gdk_2_cairo(GdkPixbuf *pixbuf){
	guint len;
	//int w, h, i, j;
	int i, j;
	unsigned char *data, *dest;
	union {
		char a;
		int  b;
	}test = {.b = 0x01020304};
	//test.data = 0x01020304;

	//w = gdk_pixbuf_get_width(pixbuf);
	//h = gdk_pixbuf_get_height(pixbuf);
	//stride = gdk_pixbuf_get_rowstride(pixbuf);
	data = gdk_pixbuf_get_pixels_with_length(pixbuf, &len);

	if(3 == gdk_pixbuf_get_n_channels(pixbuf)){
		dest = malloc(len * 4 / 3);

		if(test.a != 0x01){
			for(i = 0, j = 0; i < len; i += 3, j += 4){
				dest[j] = data[i + 2];
				dest[j + 1] = data[i + 1];
				dest[j + 2] = data[i];
			}
		}else{
			int *a, *b;
			for(i = 0, j = 0; i < len; i += 3, j += 4){
				a = (int*)(data + i);
				b = (int*)(dest + j + 1);
				*b = *a;
				/*
				   dest[j + 1] = data[i];
				   dest[j + 2] = data[i + 1];
				   dest[j + 3] = data[i + 2];
				 */
			}
		}
	}else{
		unsigned char *p, *q;
		unsigned char *end = data + len;

		dest = malloc(len);

		if(test.a != 0x01){
			for(p = data, q = dest; p < end; p += 4, q += 4){
				q[0] = p[2];
				q[1] = p[1];
				q[2] = p[0];
				q[3] = p[3];
			}
		}else{
			for(p = data, q = dest; p < end; p += 4, q += 4){
				q[0] = p[2];
				q[1] = p[1];
				q[2] = p[0];
				q[0] = p[3];
			}
		}
	}

	return dest;
}

gboolean head_configure_cb(GtkWidget *widget, GdkEvent *event, gpointer user_data){
	unsigned char *dest;
	double tmp;
	int w, h, stride;
	GdkPixbuf *pixbuf;
	cairo_format_t format = CAIRO_FORMAT_ARGB32;
	struct EditorDrawContext *context = user_data;

	w = gtk_widget_get_allocated_width(widget);
	h = gtk_widget_get_allocated_height(widget);

	//pixbuf = gdk_pixbuf_new_from_file("/opt/umclient/1.jpeg", NULL);
	if(!(pixbuf = context->pixbuf))
		return FALSE;
	context->w = gdk_pixbuf_get_width(pixbuf);
	context->h = gdk_pixbuf_get_height(pixbuf);
	stride = gdk_pixbuf_get_rowstride(pixbuf);

	if(3 == gdk_pixbuf_get_n_channels(pixbuf)){
		stride = stride * 4 / 3;
		format = CAIRO_FORMAT_RGB24;
	}

	dest = gdk_2_cairo(pixbuf);

	context->scale = 1.0f * w / context->w;
	tmp = 1.0f * h / context->h;
	if(context->scale < tmp){
		context->cw = w;
		context->ch = context->scale * context->h;
		context->cx = 0;
		context->cy = (h - context->ch) / 2;
	}else{
		context->scale = tmp;
		context->ch = h;
		context->cw = context->scale * context->w;
		context->cy = 0;
		context->cx = (w - context->cw) / 2;
	}
	context->rx = context->rx >= context->cx ? context->rx : context->cx;
	context->ry = context->ry >= context->cy ? context->ry : context->cy;

	if(context->surface)
		cairo_surface_destroy(context->surface);
	context->surface = cairo_image_surface_create_for_data(dest, format, context->w, context->h, stride);

	if(context->preview)
		cairo_surface_destroy(context->preview);
	context->preview = cairo_surface_create_for_rectangle(context->surface,
				   												  (context->rx - context->cx) / context->scale,
																  (context->ry - context->cy) / context->scale,
																  context->rw / context->scale,
																  context->rh / context->scale);
	//cairo_surface_write_to_png(context->preview, "preview.png");
	return FALSE;
}

gboolean head_draw_cb(GtkWidget *widget, cairo_t *cr, gpointer user_data){
	int w, h;
	struct EditorDrawContext *context = user_data;

	if(!context->pixbuf){
		cairo_set_source_rgba(cr, 0.9, 0.9, 0.9, 0.8);
		cairo_paint(cr);
		return FALSE;
	}

	w = gtk_widget_get_allocated_width(widget);
	h = gtk_widget_get_allocated_height(widget);

	cairo_save(cr);
	cairo_translate(cr, context->cx, context->cy);
	cairo_scale(cr, context->scale, context->scale);
	cairo_set_source_surface(cr, context->surface, 0, 0);
	cairo_paint(cr);
	cairo_restore(cr);

	cairo_rectangle(cr, 0, 0, w, h);
	cairo_new_sub_path(cr);
	cairo_rectangle(cr, context->rx, context->ry, context->rw, context->rh);
	cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);
	cairo_set_source_rgba(cr, 1, 1, 1, 0.8);
	cairo_fill(cr);

	cairo_set_line_width (cr, 1.0);
	cairo_set_source_rgba(cr, 0, 1, 1, 0.8);
	cairo_rectangle(cr, context->rx + 0.5, context->ry + 0.5, context->rw - 1, context->rh - 1);
	cairo_new_sub_path(cr);
	cairo_rectangle(cr, context->rx + context->rw - 4.5,
					context->ry + context->rh - 4.5, 8, 8);
	cairo_stroke(cr);

	return FALSE;
}

GtkWidget *head_editor(gpointer context){
	GtkWidget *da;

	da = gtk_drawing_area_new();
	gtk_widget_set_size_request(da, 400, 0);
	gtk_widget_add_events(da, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);
	g_signal_connect(da, "draw", G_CALLBACK(head_draw_cb), context);
	g_signal_connect(da, "configure-event", G_CALLBACK(head_configure_cb), context);
	g_signal_connect(da, "button-press-event", G_CALLBACK(head_rec_press), context);
	g_signal_connect(da, "button-release-event", G_CALLBACK(head_rec_release), context);
	g_signal_connect(da, "motion-notify-event", G_CALLBACK(head_rec_move), context);

	return da;
}

gboolean preview_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data){
	int w;
	struct EditorDrawContext *context = user_data;

	w = gtk_widget_get_allocated_width(widget);
	cairo_set_source_rgb(cr, 0.80, 0.80, 0.80);
	cairo_set_line_width(cr, 1);
	cairo_move_to(cr, 0.5, 0);
	cairo_line_to(cr, 0.5, 500);
	cairo_move_to(cr, 0.5, w - 0.5);
	cairo_line_to(cr, w, w - 0.5);
	cairo_stroke(cr);
	cairo_translate(cr, 1, 0);
	w--;

	if(!context->pixbuf){
		cairo_set_source_rgba(cr, 0.85, 0.85, 0.85, 0.8);
		cairo_rectangle(cr, 0, 0, w, w);
		cairo_fill(cr);
		return FALSE;
	}

	cairo_scale(cr, w * context->scale / context->rw, w * context->scale / context->rh);
	cairo_set_source_surface(cr, context->preview, 0, 0);
	cairo_paint(cr);

	return FALSE;
}

void show_preview_image(GtkFileChooser *chooser, gpointer user_data){
	gchar *filename;
	int w, h;
	GdkPixbuf *pixbuf, *tmp;
	GtkWidget *image = gtk_file_chooser_get_preview_widget(chooser);

	filename = gtk_file_chooser_get_filename(chooser);
	if(!filename) return;

	pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
	if(!pixbuf) return;

	w = gdk_pixbuf_get_width(pixbuf);
	if(w > 150){
		h = gdk_pixbuf_get_height(pixbuf);
		tmp = gdk_pixbuf_scale_simple(pixbuf, 150, h * 150 / w, GDK_INTERP_BILINEAR);
		g_object_unref(pixbuf);
		pixbuf = tmp;
	}
	gtk_image_set_from_pixbuf(GTK_IMAGE(image), pixbuf);

	g_free(filename);
}

void choose_image(GtkButton *button, gpointer user_data){
	GtkWidget *dialog;
	GtkFileFilter *filter;
	GtkWidget *image;
	struct EditorDrawContext *context = user_data;

	dialog = gtk_file_chooser_dialog_new ("选择一张图片",
			NULL,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			//GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			"取消", GTK_RESPONSE_CANCEL,
			//GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
			"选择", GTK_RESPONSE_ACCEPT,
			NULL);

	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "图像 *.png,*.jpg,*.jpeg");
	gtk_file_filter_add_pattern(filter, "*.png");
	gtk_file_filter_add_pattern(filter, "*.jpg");
	gtk_file_filter_add_pattern(filter, "*.jpeg");

	image = gtk_image_new();
	gtk_widget_set_size_request(image, 150, 0);

	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(dialog), image);
	gtk_file_chooser_set_use_preview_label(GTK_FILE_CHOOSER(dialog), FALSE);
	g_signal_connect(dialog, "update-preview", G_CALLBACK(show_preview_image), NULL);

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT){
		gboolean ret;
		char *filename;

		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

		if(context->pixbuf) g_object_unref(context->pixbuf);

		context->pixbuf = gdk_pixbuf_new_from_file(filename, NULL);

		if(context->pixbuf){
			g_signal_emit_by_name(context->widget, "configure-event", context, &ret);
		}else{
			gtk_label_set_text(GTK_LABEL(prompt_label), "读取文件失败");
			gtk_widget_show(prompt_widget);
		}
		g_free(filename);
	}

	gtk_widget_destroy(dialog);
}

void save_image(GtkButton *button, gpointer user_data){
	cJSON *obj;
	char path[512];
	gchar *name, *mail;
	struct EditorDrawContext *context = user_data;

	if(!context->pixbuf) return;

	gtk_tree_model_get(GTK_TREE_MODEL(user_model), &cur_iter, COL_NAME, &name,COL_MAIL, &mail, -1);
#if 0
	if(!strcmp("————", mail)){
		sprintf(path, "%s/%s", conf_path, name);
		cairo_surface_write_to_png(context->preview, path);

		Gdkpixbuf *small = pixbuf_from_local(name, NULL, TRUE);
		Gdkpixbuf *big = pixbuf_from_local(name, NULL, FALSE);

		gtk_list_store_set(user_model, &cur_iter, COL_IMAGE_SMALL, small, COL_IMAGE_BIG, big, -1);
		return;
	}
#endif
	sprintf(path, "%s/%s", conf_path, mail);
	cairo_surface_write_to_png(context->preview, path);

	strcpy(req_infos[3].name, name);
	strcpy(req_infos[3].mail, mail);
	
#if 1
	obj = cJSON_CreateObject();
	cJSON *jtype = cJSON_CreateString("7");
	cJSON *jname = cJSON_CreateString(name);
	cJSON *jpath = cJSON_CreateString(path);

	cJSON_AddItemToObject(obj, "type", jtype);
	cJSON_AddItemToObject(obj, "uname", jname);
	cJSON_AddItemToObject(obj, "photopath", jpath);

	char * buf = cJSON_Print(obj);
	sendreq(sfd, buf);
	free(buf);

	cJSON_Delete(obj);
#endif
}

void edit_finish(GtkButton *button, gpointer user_data){
	struct EditorDrawContext *context = user_data;

	if(context->pixbuf){
		g_object_unref(context->pixbuf);
		context->pixbuf = NULL;
	}

	gtk_widget_show_all(list_widget);
	gtk_notebook_set_current_page(GTK_NOTEBOOK(infobook), 0);
}

GtkWidget *um_head_editor(){
	GtkWidget *box;
	GtkWidget *rbox;
	GtkWidget *button;
	GtkWidget *label;
	static struct EditorDrawContext context;
	
	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

	context.rx = 0;
	context.ry = 0;
	context.rw = 200;
	context.rh = 200;
	context.in_big = 0;
	context.in_small = 0;
	context.start = 0;
	context.sizing_start = 0;
	context.surface = NULL;
	context.preview = NULL;
	context.left_ptr = gdk_cursor_new(GDK_LEFT_PTR);
	context.sizing = gdk_cursor_new(GDK_SIZING);
	context.fleur = gdk_cursor_new(GDK_FLEUR);
	context.pixbuf = NULL;
	context.widget = head_editor(&context);
	context.preview_widget = rbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	add_class(rbox, "editor");
	g_signal_connect_after(rbox, "draw", G_CALLBACK(preview_draw), &context);

	button = gtk_button_new_with_label("确定");
	//add_class(button, "editor");
	gtk_box_pack_end(GTK_BOX(rbox), button, FALSE, FALSE, 0);
	g_signal_connect(button, "clicked", G_CALLBACK(save_image), &context);

	button = gtk_button_new_with_label("返回");
	gtk_box_pack_end(GTK_BOX(rbox), button, FALSE, FALSE, 0);
	g_signal_connect(button, "clicked", G_CALLBACK(edit_finish), &context);

	button = gtk_button_new_with_label("上传图片");
	gtk_box_pack_end(GTK_BOX(rbox), button, FALSE, FALSE, 100);
	g_signal_connect(button, "clicked", G_CALLBACK(choose_image), &context);

	label = gtk_label_new("预览");
	gtk_box_pack_end(GTK_BOX(rbox), label, FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(box), context.widget, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), rbox, TRUE, TRUE, 0);
	
	return box;
}

GtkWidget *um_info_book(gpointer user_data){

	infobook = gtk_notebook_new();
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(infobook), FALSE);
	gtk_notebook_set_show_border(GTK_NOTEBOOK(infobook), FALSE);
	gtk_notebook_append_page(GTK_NOTEBOOK(infobook), um_info_page(infobook), NULL);
	gtk_notebook_append_page(GTK_NOTEBOOK(infobook), um_head_editor(), NULL);

	return infobook;
}

gboolean passwd_cb(GtkWidget *widget, gpointer user_data){
	cJSON *obj;
	char *name;
	char *buf, prompt_message[64];
	struct required_widgets *data = user_data;
	const gchar *pwd = gtk_entry_get_text(GTK_ENTRY(data->pwd));
	const gchar *pwd1 = gtk_entry_get_text(GTK_ENTRY(data->pwd1));
	const gchar *pwd2 = gtk_entry_get_text(GTK_ENTRY(data->pwd2));

	gtk_tree_model_get(GTK_TREE_MODEL(user_model), &cur_iter, COL_NAME, &name, -1);

	if(!*pwd){
		add_class(data->pwd, "red_border");
		return FALSE;
	}
	if(!*pwd1){
		add_class(data->pwd1, "red_border");
		return FALSE;
	}
	if(!*pwd2){
		add_class(data->pwd2, "red_border");
		return FALSE;
	}
	if(strcmp(pwd1, pwd2)){
		sprintf(prompt_message, "两次输入密码不一致!");
		goto error;
	}
	if(strlen(pwd1) < 6){
		sprintf(prompt_message, "密码长度不能小于6位!");
		goto error;
	}
	if(!strcmp(pwd, pwd1)){
		sprintf(prompt_message, "新密码不能与原密码相同!");
		goto error;
	}
	/*TODO*///judge passwd ro email is lawye

	obj = cJSON_CreateObject();
	cJSON *jtype = cJSON_CreateString("4");
	cJSON *juser = cJSON_CreateString(loguser);
	cJSON *jname = cJSON_CreateString(name);
	cJSON *jold = cJSON_CreateString(pwd);
	cJSON *jnew = cJSON_CreateString(pwd1);

	cJSON_AddItemToObject(obj, "type", jtype);
	cJSON_AddItemToObject(obj, "uname", juser);
	cJSON_AddItemToObject(obj, "uptname", jname);
	cJSON_AddItemToObject(obj, "oldpwd", jold);
	cJSON_AddItemToObject(obj, "pwd", jnew);

	buf = cJSON_Print(obj);
	//printf("%s\n", buf);
	sendreq(sfd, buf);
	free(buf);

	cJSON_Delete(obj);

	return FALSE;
error:
	gtk_label_set_text(GTK_LABEL(prompt_label), prompt_message);
	gtk_widget_show(prompt_widget);
	return FALSE;
}

GtkWidget *um_passwd_page(){
	GtkWidget *box;
	GtkWidget *old_box;
	GtkWidget *new_box;
	GtkWidget *ens_box;
	GtkWidget *btn_box;
	GtkWidget *entry;
	GtkWidget *button;
	static struct required_widgets req_widgets;

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
	gtk_widget_set_valign(box, GTK_ALIGN_CENTER);

	old_box = justify_label(64, 3, "原", "密", "码");
	gtk_widget_set_halign(old_box, GTK_ALIGN_CENTER);
	entry = gtk_entry_new();
	req_widgets.pwd = entry;
	gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE);
	gtk_widget_set_size_request(entry, 220, 30);
	gtk_box_pack_start(GTK_BOX(old_box), entry, FALSE, FALSE, 10);

	new_box = justify_label(64, 3, "新", "密", "码");
	gtk_widget_set_halign(new_box, GTK_ALIGN_CENTER);
	entry = gtk_entry_new();
	req_widgets.pwd1 = entry;
	gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE);
	gtk_widget_set_size_request(entry, 220, 30);
	gtk_box_pack_start(GTK_BOX(new_box), entry, FALSE, FALSE, 10);

	ens_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_set_halign(ens_box, GTK_ALIGN_CENTER);
	entry = gtk_entry_new();
	req_widgets.pwd2 = entry;
	gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE);
	gtk_widget_set_size_request(entry, 220, 30);
	gtk_box_pack_start(GTK_BOX(ens_box), gtk_label_new("确认密码"), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(ens_box), entry, FALSE, FALSE, 10);

	g_signal_connect(req_widgets.pwd, "button-press-event", G_CALLBACK(rm_red_border), NULL);
	g_signal_connect(req_widgets.pwd1, "button-press-event", G_CALLBACK(rm_red_border), NULL);
	g_signal_connect(req_widgets.pwd2, "button-press-event", G_CALLBACK(rm_red_border), NULL);

	btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);	
	button = gtk_button_new_with_label("修 改");
	add_class(button, "reg_btn");
	g_signal_connect(button, "clicked", G_CALLBACK(passwd_cb), &req_widgets);
	gtk_widget_set_halign(btn_box, GTK_ALIGN_CENTER);
	gtk_widget_set_margin_left(button, 194);
	gtk_widget_set_margin_right(button, 10);
	gtk_widget_set_size_request(button, 100, 0);
	gtk_box_pack_start(GTK_BOX(btn_box), button, FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(box), old_box, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), new_box, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), ens_box, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), btn_box, FALSE, FALSE, 0);
	gtk_widget_show_all(box);

	return box;
}
/*
static void set_cell_image(GtkCellLayout   *cell_layout,
						   GtkCellRenderer *cell,
						   GtkTreeModel    *tree_model,
						   GtkTreeIter     *iter,
						   gpointer         data)
{
	gchar *image;
	GdkPixbuf *pixbuf;

	gtk_tree_model_get(tree_model, iter, COL_IMAGE, &image, -1);

	pixbuf = gdk_pixbuf_new_from_file_at_size(image, 30, 30, NULL);

	g_object_set(cell, "pixbuf", pixbuf, NULL);

	g_object_unref(pixbuf);
}
*/
gint timeout_show(gpointer user_data){
	struct animation *data = user_data;

	if(!data->showing) return FALSE;

	data->frame_count += 2;
	data->ani_margin = data->cur_margin + data->frame_count * data->frame_count * data->frame_count / 80;
	
	if(data->ani_margin >= 120){
		data->showing = 0;
		data->ani_margin = data->cur_margin = 120;
		gtk_widget_set_margin_right(data->animate_widget, 120);
		gtk_widget_queue_resize(data->scrwin);
		return FALSE;
	}

	gtk_widget_set_margin_right(data->animate_widget, data->ani_margin);
	gtk_widget_queue_resize(data->scrwin);
	
	return TRUE;
}

gint timeout_hide(gpointer user_data){
	struct animation *data = user_data;

	if(!data->hiding) return FALSE;

	data->frame_count += 2;
	data->ani_margin = data->cur_margin - data->frame_count * data->frame_count * data->frame_count / 80;
	
	if(data->ani_margin <= 0){
		data->hiding = 0;
		data->ani_margin = data->cur_margin = 0;
		gtk_widget_set_margin_right(data->animate_widget, 0);
		gtk_widget_queue_resize(data->scrwin);
		return FALSE;
	}

	gtk_widget_set_margin_right(data->animate_widget, data->ani_margin);
	gtk_widget_queue_resize(data->scrwin);
	
	return TRUE;
}

static gboolean animation_show(GtkWidget *widget, GdkEvent *event, gpointer user_data){
	GdkEventCrossing *cevent = (GdkEventCrossing*)event;
	struct animation *data = user_data;

	//printf("show %p\n", cevent->window);
	if(cevent->window == gtk_widget_get_window(widget))
	   	return TRUE;

	data->showing = 1;
	data->hiding = 0;
	data->frame_count = 0;
	data->cur_margin = data->ani_margin;
	g_timeout_add(20, timeout_show, user_data);

	return TRUE;
}

static gboolean animation_hide(GtkWidget *widget, GdkEvent *event, gpointer user_data){
	GdkEventCrossing *cevent = (GdkEventCrossing*)event;
	struct animation *data = user_data;

	//printf("hide %p\n", cevent->window);
	if(cevent->window != gtk_widget_get_window(widget))
		return TRUE;

	data->hiding = 1;
	data->showing = 0;
	data->frame_count = 0;
	data->cur_margin = data->ani_margin;
	//usleep(200000);
	g_timeout_add(20, timeout_hide, user_data);

	return TRUE;
}

size_t g_write_stream(char *ptr, size_t size, size_t n, void *data){
	size_t len = size * n;
	GInputStream *stream = data;
	char *p;

	if(g_input_stream_is_closed(stream))
		return len;

	if(!strncmp(ptr + 2, "result", 6)){
		g_input_stream_close(stream, NULL, NULL);
		return len;
	}
	
	p = malloc(len);
	memcpy(p, ptr, len);
	g_memory_input_stream_add_data(G_MEMORY_INPUT_STREAM(stream), p, len, NULL);

	return len;
}

gboolean um_set_pixbuf(gpointer user_data){
	struct StoreInfo *data = user_data;

	gtk_list_store_set(data->store, &data->iter, COL_IMAGE_SMALL, data->small, COL_IMAGE_BIG, data->big, -1);

	if(cur_iter.user_data == data->iter.user_data){
		gtk_image_set_from_pixbuf(GTK_IMAGE(info_widgets.head_image), data->big);
	}
	
	free(data);
	return FALSE;
}

void *curl_set_pixbuf(gpointer user_data){
	int fd, r;
	CURL *curl;
	char fields[256];
	char buf[4096];
	GdkPixbuf *small, *big;
	GInputStream *stream;
	GError *error = NULL;
	struct StoreInfo *data = user_data;
	
	curl = curl_easy_init();

	stream = g_memory_input_stream_new();

	//strcpy(url, "http://www.qqbody.com/uploads/allimg/201407/31-161128_920.jpg");
	sprintf(fields, "&email=%s", data->mail);

	do{
		if(!curl)
			break;
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, fields);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, g_write_stream);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, stream);
		curl_easy_setopt(curl, CURLOPT_URL, server_head_url);

		//printf("OK:%s?%s\n", server_head_url, fields);
		if(CURLE_OK != curl_easy_perform(curl)){
			//printf("%d\n", curl_easy_perform(curl));
			curl_easy_cleanup(curl);
			break;
		}

		if(g_input_stream_is_closed(stream))
			break;

		small = gdk_pixbuf_new_from_stream_at_scale(stream, 30, 30, FALSE, NULL, &error);
		//if(error) printf("%s\n", error->message);

		error = NULL;
		g_seekable_seek(G_SEEKABLE(stream), 0, G_SEEK_SET, NULL, NULL);
		big = gdk_pixbuf_new_from_stream_at_scale(stream, 75, 75, FALSE, NULL, &error);
		//if(error) printf("%s\n",error->message);

		g_seekable_seek(G_SEEKABLE(stream), 0, G_SEEK_SET, NULL, NULL);
		sprintf(fields, "%s/%s", conf_path, data->mail);
		if((fd = open(fields, O_WRONLY|O_CREAT, 0644)) > 0){
			while(1){
				r = g_input_stream_read(G_INPUT_STREAM(stream), buf, sizeof(buf), NULL, NULL);
				if(r <= 0) break;
				r = writen(fd, buf, r);
				if(r <= 0) break;
			}
			close(fd);
			if(r < 0)
				remove(fields);
		}

		data->small = small;
		data->big = big;
		gdk_threads_add_idle(um_set_pixbuf, data);

		curl_easy_cleanup(curl);
	}while(0);

	if(!data->small)
		free(data);

	g_input_stream_close(stream, NULL, NULL);

	return NULL;
}

GdkPixbuf *pixbuf_from_local(const char *name, const char *mail, gboolean preview){
	int size = 30;
	const char *tail;
	char image_path[256];
	GdkPixbuf *pixbuf;
	
	tail = mail ? mail : name;
	sprintf(image_path, "%s/%s", conf_path, tail);

	if(!access(image_path, F_OK)){
		size = preview ? 30 : 75;
		pixbuf = gdk_pixbuf_new_from_file_at_size(image_path, size, size, NULL);
	}else{
		pixbuf = preview ? def_head.small : def_head.big;
	}

	return pixbuf;
}

GtkListStore *create_store(gpointer user_data){
	int i, n;
	cJSON *array;
	cJSON *obj = user_data;
	GtkTreeIter iter;
	GtkListStore *store;
	GdkPixbuf *small, *big;
	const char *name, *mail;

	store = gtk_list_store_new(NUM_COLS, GDK_TYPE_PIXBUF, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING);
	gtk_list_store_clear(store);

	if(array = cJSON_GetObjectItem(obj, "remote")){
		n = cJSON_GetArraySize(array);
		for(i = 0; i < n; i++){
			cJSON *user = cJSON_GetArrayItem(array, i);
			struct StoreInfo *data = malloc(sizeof(struct StoreInfo));
			name = cJSON_GetObjectItem(user, "uname")->valuestring;
			mail = cJSON_GetObjectItem(user, "email")->valuestring;
			small = pixbuf_from_local(name, mail, TRUE);
			big = pixbuf_from_local(name, mail, FALSE);

			gtk_list_store_append(store, &iter);
			gtk_list_store_set(store, &iter,
							   COL_IMAGE_SMALL, small,
							   COL_IMAGE_BIG, big,
				   			   COL_NAME, name,
							   COL_MAIL, mail,
							   -1);

			data->mail = mail;
			data->store = store;
			data->iter = iter;
			data->small = data->big = NULL;
			pool_add(gtk_pool, curl_set_pixbuf, data);
		}
	}

	if(array = cJSON_GetObjectItem(obj, "local")){
		n = cJSON_GetArraySize(array);
		for(i = 0; i < n; i++){
			cJSON *user = cJSON_GetArrayItem(array, i);
			name = cJSON_GetObjectItem(user, "uname")->valuestring;
#if 0
			small = pixbuf_from_local(name, NULL, TRUE);
			big = pixbuf_from_local(name, NULL, FALSE);
#endif

			gtk_list_store_append(store, &iter);
			gtk_list_store_set(store, &iter,
#if 0
							   COL_IMAGE_SMALL, small,
							   COL_IMAGE_BIG, big,
#else
							   COL_IMAGE_SMALL, def_head.small,
							   COL_IMAGE_BIG, def_head.big,
#endif
				   			   COL_NAME, cJSON_GetObjectItem(user, "uname")->valuestring,
							   COL_MAIL, "————",
							   -1);
		}
	}

	return store;
}

void change_cb(GtkIconView *iconview, gpointer user_data){
	GtkTreeIter *iter = user_data;
	GList *list;
	GdkPixbuf *pixbuf;
	const gchar *name, *mail;
	
	list = gtk_icon_view_get_selected_items(iconview);
	if(!list){
		gtk_icon_view_select_path(GTK_ICON_VIEW(iconview),
			   					  gtk_tree_model_get_path(GTK_TREE_MODEL(user_model), iter));
		return;
	}

	gtk_tree_model_get_iter(GTK_TREE_MODEL(user_model), iter, list->data);
	gtk_tree_model_get(GTK_TREE_MODEL(user_model), iter,
		   			   COL_IMAGE_BIG, &pixbuf,
		   			   COL_NAME, &name,
					   COL_MAIL, &mail,
					   -1);

	gtk_image_set_from_pixbuf(GTK_IMAGE(info_widgets.head_image), pixbuf);

	gtk_label_set_text(GTK_LABEL(info_widgets.nick_label), name);
	gtk_label_set_text(GTK_LABEL(info_widgets.name_label), name);
	gtk_label_set_text(GTK_LABEL(info_widgets.mail_label), mail);

	if(!strcmp("root", loguser))
		if(strcmp("————", mail))
			gtk_widget_set_sensitive(gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), 1), FALSE);
		else
			gtk_widget_set_sensitive(gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), 1), TRUE);
	//printf("selection-changed: %p, %p, %p\n", list->data, list->prev, list->next);
}

void delete_cb(GtkWidget *widget, gpointer user_data){
	char *name, *buf;
	cJSON *obj;
	GtkTreeIter *iter = user_data;

	gtk_tree_model_get(GTK_TREE_MODEL(user_model), iter, COL_NAME, &name, -1);
	del_iter = *iter;

	obj = cJSON_CreateObject();
	cJSON *jtype = cJSON_CreateString("6");
	cJSON *juser = cJSON_CreateString(loguser);
	cJSON *jname = cJSON_CreateString(name);
	cJSON_AddItemToObject(obj, "type", jtype);
	cJSON_AddItemToObject(obj, "uname", juser);
	cJSON_AddItemToObject(obj, "fbdname", jname);
	
	buf = cJSON_Print(obj);
	sendreq(sfd, buf);
	free(buf);

	cJSON_Delete(obj);
	//printf("delete\n");
}

gboolean menu_cb(GtkWidget *widget, GdkEvent *event, gpointer user_data){
	GtkTreePath *path;
	static GtkTreeIter iter;
	static GtkWidget *menu = NULL;

	if(event->type == GDK_BUTTON_PRESS && event->button.button == 3){
		path = gtk_icon_view_get_path_at_pos(GTK_ICON_VIEW(widget), event->button.x, event->button.y);
		if(path){
			gtk_tree_model_get_iter(GTK_TREE_MODEL(user_model), &iter, path);
			if(!menu){
				GtkWidget *menuitem = gtk_image_menu_item_new_with_label("删除该用户");
				g_signal_connect(menuitem, "activate", G_CALLBACK(delete_cb), &iter);

				GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size("/opt/usermanageclient/del.png", 20, 20, NULL);
				GtkWidget *image = gtk_image_new_from_pixbuf(pixbuf);
				gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);

				gtk_widget_show(menuitem);
				menu = gtk_menu_new();
				gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
			}
			gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, event->button.button, event->button.time);
		}
	}
	return FALSE;
}

GtkWidget *um_scrwin(gpointer user_data){
	GtkWidget *scrwin;
	GtkWidget *inner_box;
	GtkWidget *icon_view;
	GtkTreeIter iter;
	GtkListStore *store = user_data;
	GtkCellRenderer *renderer;
	GtkWidget *vport;
	static struct animation animation_data;

	inner_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_set_margin_left(inner_box, 5);
	bzero(&animation_data, sizeof(animation_data));
	animation_data.animate_widget = inner_box;

	list_view = icon_view = gtk_icon_view_new_with_model(GTK_TREE_MODEL(store));
	g_signal_connect(icon_view, "selection-changed", G_CALLBACK(change_cb), &cur_iter);
	if(!strcmp(loguser, "root"))
		g_signal_connect(icon_view, "button-press-event", G_CALLBACK(menu_cb), &cur_iter);
	//gtk_widget_add_events(icon_view, GDK_ENTER_NOTIFY_MASK|GDK_LEAVE_NOTIFY_MASK);
	//g_signal_connect(icon_view, "enter-notify-event", G_CALLBACK(animation_show), &animation_data);
	//g_signal_connect(icon_view, "leave-notify-event", G_CALLBACK(animation_hide), &animation_data);
	gtk_icon_view_set_item_width(GTK_ICON_VIEW(icon_view), 153);

	gtk_icon_view_set_selection_mode(GTK_ICON_VIEW(icon_view), GTK_SELECTION_SINGLE);
	gtk_icon_view_set_item_orientation(GTK_ICON_VIEW(icon_view), GTK_ORIENTATION_HORIZONTAL);
	gtk_icon_view_set_columns(GTK_ICON_VIEW(icon_view), 1);
	gtk_icon_view_set_reorderable(GTK_ICON_VIEW(icon_view), FALSE);
	gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(icon_view), COL_IMAGE_SMALL);
	
	//renderer = gtk_cell_renderer_pixbuf_new();
	//gtk_cell_renderer_set_fixed_size(renderer, 38, -1);
	//gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(icon_view), renderer, FALSE);
	//gtk_cell_layout_set_cell_data_func(GTK_CELL_LAYOUT(icon_view), renderer, set_cell_image, NULL, NULL);

	gtk_icon_view_set_row_spacing(GTK_ICON_VIEW(icon_view), 5);
	gtk_icon_view_set_margin(GTK_ICON_VIEW(icon_view), 0);
	gtk_icon_view_set_item_padding(GTK_ICON_VIEW(icon_view), 4);

	renderer = gtk_cell_renderer_text_new();
	//g_object_set(renderer, "max-width-chars", 12, NULL);
	//g_object_set(renderer, "wrap-width", 7, NULL);
	g_object_set(renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_cell_renderer_set_alignment(renderer, 0, 0.5);
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(icon_view), renderer, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(icon_view), renderer, "text", COL_NAME, NULL);

	gtk_box_pack_start(GTK_BOX(inner_box), icon_view, FALSE, FALSE, 0);
	gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter);
	gtk_icon_view_select_path(GTK_ICON_VIEW(icon_view), gtk_tree_model_get_path(GTK_TREE_MODEL(store), &iter));

	vport = gtk_viewport_new(NULL, NULL);
	gtk_widget_add_events(vport, GDK_ENTER_NOTIFY_MASK|GDK_LEAVE_NOTIFY_MASK);
	g_signal_connect(vport, "enter-notify-event", G_CALLBACK(animation_show), &animation_data);
	g_signal_connect(vport, "leave-notify-event", G_CALLBACK(animation_hide), &animation_data);
	gtk_container_add(GTK_CONTAINER(vport), inner_box);

	animation_data.scrwin = list_widget = scrwin = scr_win_new();
	gtk_widget_set_halign(scrwin, GTK_ALIGN_END);
	gtk_container_add(GTK_CONTAINER(scrwin), vport);
	//gtk_container_add(GTK_CONTAINER(scrwin), icon_view);
	gtk_widget_show_all(scrwin);
	//gtk_icon_view_item_activated(GTK_ICON_VIEW(icon_view), gtk_icon_view_get_path_at_pos(GTK_ICON_VIEW(icon_view), 30, 30));
	//gtk_icon_view_selected_foreach(GTK_ICON_VIEW(icon_view), for_each_test, NULL);

	return scrwin;
}

gboolean prompt_cb(GtkWidget *widget, gpointer user_data){
	gtk_widget_hide(prompt_widget);
	return FALSE;
}

GtkWidget *um_prompt(){
	GtkWidget *box1;
	GtkWidget *box2;
	GtkWidget *ebox1;
	GtkWidget *ebox2;
	GtkWidget *button;

	prompt_widget = ebox1 = gtk_event_box_new();
	add_class(ebox1, "prompt1");

	box1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_set_valign(box1, GTK_ALIGN_CENTER);

	box2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

	ebox2 = gtk_event_box_new();
	add_class(ebox2, "prompt2");

	prompt_label = gtk_label_new("");
	button = gtk_button_new_with_label("确定");
	g_signal_connect(button, "clicked", G_CALLBACK(prompt_cb), NULL);
	gtk_widget_set_margin_right(button, 100);
	gtk_widget_set_margin_top(button, 3);
	gtk_widget_set_margin_bottom(button, 3);

	gtk_box_pack_end(GTK_BOX(box2), button, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(box2), prompt_label, TRUE, FALSE, 0);

	gtk_container_add(GTK_CONTAINER(ebox2), box2);
	gtk_box_pack_start(GTK_BOX(box1), ebox2, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(ebox1), box1);
	
	return ebox1;
}

GtkWidget *um_setting_box(gpointer user_data){
	GtkWidget *overlay;
	GtkWidget *info_box;

	overlay = gtk_overlay_new();

	user_model = create_store(user_data);

	notebook = gtk_notebook_new();
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
	gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), FALSE);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), um_info_book(user_model), NULL);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), um_passwd_page(), NULL);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), gtk_box_new(GTK_ORIENTATION_VERTICAL, 0), NULL);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), um_sync_page(), NULL);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), um_reg_page(TRUE), NULL);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), um_reg_page(FALSE), NULL);

	gtk_container_add(GTK_CONTAINER(overlay), notebook);
	gtk_overlay_add_overlay(GTK_OVERLAY(overlay), um_prompt());
	gtk_overlay_add_overlay(GTK_OVERLAY(overlay), um_scrwin(user_model));
	gtk_widget_show_all(overlay);

	gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 0);

	return overlay;
}

GtkWidget *um_main_box(gpointer user_data){
	GtkWidget *box;
	GtkWidget *box1;
	GtkWidget *box2;

	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

	def_head.small = gdk_pixbuf_new_from_file_at_size("/opt/usermanageclient/default.png", 30, 30, NULL);
	def_head.big = gdk_pixbuf_new_from_file_at_size("/opt/usermanageclient/default.png", 75, 75, NULL);

	box1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start(GTK_BOX(box1), um_icon_bar(), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box1), um_selection_button_box(), TRUE, TRUE, 0);

	box2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start(GTK_BOX(box2), um_info_bar("用户信息"), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box2), um_setting_box(user_data), TRUE, TRUE, 0);

	gtk_box_pack_start(GTK_BOX(box), box1, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), box2, TRUE, TRUE, 0);

	gtk_widget_show_all(box);

	return box;
}

/*
 * This function define the login window
 */
GtkWidget *um_main_win(gpointer user_data){
	static GtkWidget *window = NULL;

	if(window) return window;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	gtk_window_set_title(GTK_WINDOW(window), "网络账号客户端");
	gtk_container_set_border_width(GTK_CONTAINER(window), 1);
	gtk_widget_set_size_request(window, 680, 480);
	g_signal_connect_after(window, "draw", G_CALLBACK(draw_border), NULL);
	g_signal_connect_after(window, "set-focus", G_CALLBACK(remove_focus), NULL);

	gtk_container_add(GTK_CONTAINER(window), um_main_box(user_data));
	gtk_widget_show(window);
	gtk_widget_hide(prompt_widget);

	return window;
}

gboolean register_cb(GtkWidget *widget, gpointer user_data){
	struct required_widgets *data = user_data;
	
	if(!register_send(widget, user_data)){
		return FALSE;
	}

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->other))){
		if(!access(regmask, F_OK)){
			remove(regmask);
		}
	}

	gtk_widget_destroy(gtk_widget_get_toplevel(GTK_WIDGET(data->other)));

	cur_win = um_log_win();

	return FALSE;
}

gboolean cancel_cb(GtkWidget *widget, gpointer user_data){
	struct required_widgets *data = user_data;

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->other))){
		if(!access(regmask, F_OK)){
			remove(regmask);
		}
	}

	gtk_widget_destroy(gtk_widget_get_toplevel(GTK_WIDGET(data->other)));

	um_log_win();

	return FALSE;
}

GtkWidget *um_reg_win(){
	static int crd[2];
	static struct required_widgets req_widgets;
	GtkWidget *reg_win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(reg_win), 1);
	g_signal_connect_after(reg_win, "draw", G_CALLBACK(draw_border), NULL);
	gtk_window_set_decorated(GTK_WINDOW(reg_win), FALSE); 
	gtk_window_set_resizable(GTK_WINDOW(reg_win), FALSE);
	gtk_window_set_title(GTK_WINDOW(reg_win), "网络账号注册");
	gtk_widget_set_size_request(reg_win, 450, 0);

	//GtkWidget *out_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 30);

	GtkWidget *reg_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
	gtk_widget_set_valign(reg_box, GTK_ALIGN_CENTER);

	GtkWidget *rt_eventbox = gtk_event_box_new();
	add_class(rt_eventbox, "small_icon");
	g_signal_connect(rt_eventbox, "button-press-event", G_CALLBACK(motion_start), crd);
	g_signal_connect(rt_eventbox, "motion-notify-event", G_CALLBACK(motion_changing), crd);

	GtkWidget *rt_label = gtk_label_new("网络账号注册");
	gtk_container_add(GTK_CONTAINER(rt_eventbox), rt_label);

	GtkWidget *regname_box = justify_label(64, 3, "用", "户", "名");
	gtk_widget_set_halign(regname_box, GTK_ALIGN_CENTER);
	GtkWidget *regname_entry = gtk_entry_new();
	req_widgets.name = regname_entry;
	gtk_widget_set_size_request(regname_entry, 220, 30);
	gtk_box_pack_start(GTK_BOX(regname_box), regname_entry, FALSE, FALSE, 10);
	
	GtkWidget *regpasswd_box = justify_label(64, 2, "密", "码");
	gtk_widget_set_halign(regpasswd_box, GTK_ALIGN_CENTER);
	GtkWidget *regpasswd_entry = gtk_entry_new();
	req_widgets.pwd1 = regpasswd_entry;
	gtk_entry_set_visibility(GTK_ENTRY(regpasswd_entry), FALSE);
	gtk_widget_set_size_request(regpasswd_entry, 220, 30);
	gtk_box_pack_start(GTK_BOX(regpasswd_box), regpasswd_entry, FALSE, FALSE, 10);

	GtkWidget *ensurepasswd_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_set_halign(ensurepasswd_box, GTK_ALIGN_CENTER);

	GtkWidget *ensurepasswd_label = gtk_label_new("确认密码");
	gtk_misc_set_alignment(GTK_MISC(ensurepasswd_label), 1, 0.5);
	gtk_widget_set_size_request(ensurepasswd_label, 64, 0);

	GtkWidget *ensurepasswd_entry = gtk_entry_new();
	req_widgets.pwd2 = ensurepasswd_entry;
	gtk_entry_set_visibility(GTK_ENTRY(ensurepasswd_entry), FALSE);
	gtk_widget_set_size_request(ensurepasswd_entry, 220, 30);
	gtk_entry_set_visibility(GTK_ENTRY(ensurepasswd_entry), FALSE);

	gtk_box_pack_end(GTK_BOX(ensurepasswd_box), ensurepasswd_entry, FALSE, FALSE, 10);
	gtk_box_pack_end(GTK_BOX(ensurepasswd_box), ensurepasswd_label, FALSE, FALSE, 0);

	GtkWidget *regemail_box = justify_label(64, 2, "邮", "箱");
	gtk_widget_set_halign(regemail_box, GTK_ALIGN_CENTER);
	GtkWidget *regemail_entry = gtk_entry_new();
	req_widgets.mail = regemail_entry;
	gtk_widget_set_size_request(regemail_entry, 220, 30);
	gtk_box_pack_start(GTK_BOX(regemail_box), regemail_entry, FALSE, FALSE, 10);

	GtkWidget *regbtn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_set_margin_left(regbtn_box, 10);
	gtk_widget_set_margin_bottom(regbtn_box, 10);
	gtk_widget_set_margin_right(regbtn_box, 82);

	GtkWidget *check_button = gtk_check_button_new_with_label("不再提示");
	req_widgets.other = check_button;

	GtkWidget *reg_button = gtk_button_new_with_label("注 册");
	add_class(reg_button, "reg_btn");
	g_signal_connect(reg_button, "clicked", G_CALLBACK(register_cb), &req_widgets);
	gtk_widget_set_size_request(reg_button, 100, 0);

	GtkWidget *cancel_button = gtk_button_new_with_label("取消");
	add_class(cancel_button, "reg_btn");
	g_signal_connect(cancel_button, "clicked", G_CALLBACK(cancel_cb), &req_widgets);
	gtk_widget_set_size_request(cancel_button, 100, 0);
	gtk_box_pack_start(GTK_BOX(regbtn_box), check_button, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(regbtn_box), reg_button, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(regbtn_box), cancel_button, FALSE, FALSE, 20);

	gtk_box_pack_start(GTK_BOX(reg_box), rt_eventbox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(reg_box), regname_box, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(reg_box), regpasswd_box, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(reg_box), ensurepasswd_box, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(reg_box), regemail_box, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(reg_box), regbtn_box, FALSE, FALSE, 0);


	gtk_container_add(GTK_CONTAINER(reg_win), reg_box);
	gtk_widget_show_all(reg_win);
	gtk_window_set_focus_visible(GTK_WINDOW(reg_win), FALSE);

	return reg_win;
}

gboolean log_cb(GtkWidget *widget, gpointer user_data){
	char *buf;
	cJSON *obj;
	struct required_widgets *data = user_data;
	const gchar *name = gtk_entry_get_text(GTK_ENTRY(data->name));
	const gchar *pwd = gtk_entry_get_text(GTK_ENTRY(data->pwd));

	if(!*name){
		add_class(data->name, "red_border");
		return FALSE;
	}
	if(!*pwd){
		add_class(data->pwd, "red_border");
		return FALSE;
	}
	strcpy(loguser, name);

	obj = cJSON_CreateObject();
	cJSON *jtype = cJSON_CreateString("2");
	cJSON *jname = cJSON_CreateString(name);
	cJSON *jpwd = cJSON_CreateString(pwd);
	cJSON_AddItemToObject(obj, "type", jtype);
	cJSON_AddItemToObject(obj, "uname", jname);
	cJSON_AddItemToObject(obj, "passwd", jpwd);

	buf = cJSON_Print(obj);
	//printf("%s\n", buf);
	sendreq(sfd, buf);
	free(buf);

	cJSON_Delete(obj);

	return FALSE;
}

gboolean bind_enter(GtkWidget *widget, GdkEvent *event, gpointer user_data){
	remove_class(widget, "red_border");

	if(((GdkEventKey*)event)->keyval == 65293)
		log_cb(widget, user_data);
	return FALSE;
}

GtkWidget *um_log_win(){
	static int crd[2];
	GtkWidget *window;
	GtkWidget *box, *ebox;
	GtkWidget *btn_box;
	GtkWidget *event_box;
	GtkWidget *box1, *box2;
	GtkWidget *close, *minimize;
	GtkWidget *name_entry;
	GtkWidget *passwd_entry;
	GtkWidget *label;
	GdkPixbuf *pixbuf;
	GtkWidget *image;
	GtkWidget *button;
	static struct required_widgets req_widgets;
   
	cur_win = window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(window), 1);
	g_signal_connect_after(window, "draw", G_CALLBACK(draw_border), NULL);
	gtk_window_set_decorated(GTK_WINDOW(window), FALSE); 
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	gtk_window_set_title(GTK_WINDOW(window), "网络账号客户端");
	gtk_widget_set_size_request(window, 380, 0);

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	ebox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	event_box = gtk_event_box_new();
	add_class(event_box, "log_ani");
	gtk_widget_set_size_request(event_box, 0, 150);
	g_signal_connect(event_box, "button-press-event", G_CALLBACK(motion_start), crd);
	g_signal_connect(event_box, "motion-notify-event", G_CALLBACK(motion_changing), crd);

	btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	add_class(btn_box, "log_bar");

	close = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(close), GTK_RELIEF_NONE);
	g_signal_connect(close, "clicked", G_CALLBACK(gtk_main_quit), NULL);
	pixbuf = gdk_pixbuf_new_from_file_at_size("/opt/usermanageclient/close.png", 10, 10, NULL);
	image = gtk_image_new_from_pixbuf(pixbuf);
	gtk_button_set_image(GTK_BUTTON(close), image);

	minimize = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(minimize), GTK_RELIEF_NONE);
	g_signal_connect(minimize, "clicked", G_CALLBACK(client_minimize), NULL);
	pixbuf = gdk_pixbuf_new_from_file_at_size("/opt/usermanageclient/minimize.png", 10, 10, NULL);
	image = gtk_image_new_from_pixbuf(pixbuf);
	gtk_button_set_image(GTK_BUTTON(minimize), image);

	gtk_box_pack_end(GTK_BOX(btn_box), close, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(btn_box), minimize, FALSE, FALSE, 0);

	label = gtk_label_new("网 络 账 号\n用 户 管 理");
	
	gtk_box_pack_start(GTK_BOX(ebox), btn_box, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(ebox), label, TRUE, FALSE, 0);

	gtk_container_add(GTK_CONTAINER(event_box), ebox);

	box1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	gtk_widget_set_halign(box1, GTK_ALIGN_CENTER);

	pixbuf = gdk_pixbuf_new_from_file_at_size("/opt/usermanageclient/default.png", 100, 100, NULL);
	image = gtk_image_new_from_pixbuf(pixbuf);

	box2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

	name_entry = gtk_entry_new();
	req_widgets.name = name_entry;
	gtk_entry_set_placeholder_text(GTK_ENTRY(name_entry), "用户名");
	gtk_widget_set_size_request(name_entry, 170, 26);

	passwd_entry = gtk_entry_new();
	req_widgets.pwd = passwd_entry;
	gtk_entry_set_placeholder_text(GTK_ENTRY(passwd_entry), "密    码");
	gtk_entry_set_visibility(GTK_ENTRY(passwd_entry), FALSE);
	gtk_widget_set_size_request(passwd_entry, 170, 26);

	g_signal_connect(req_widgets.name, "button-press-event", G_CALLBACK(rm_red_border), NULL);
	g_signal_connect(req_widgets.pwd, "button-press-event", G_CALLBACK(rm_red_border), NULL);
	g_signal_connect(req_widgets.pwd, "key-press-event", G_CALLBACK(bind_enter), &req_widgets);

	button = gtk_button_new_with_label("登  录");
	add_class(button, "log_btn");
	g_signal_connect(button, "clicked", G_CALLBACK(log_cb), &req_widgets);

	gtk_box_pack_start(GTK_BOX(box2), name_entry, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box2), passwd_entry, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box2), button, FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(box1), image, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box1), box2, FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(box), event_box, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), box1, FALSE, FALSE, 10);

	gtk_container_add(GTK_CONTAINER(window), box);
	gtk_widget_show_all(window);
	gtk_window_set_focus_visible(GTK_WINDOW(window), FALSE);

	return window;
}

void set_conf_dir(){
	int r;
	FILE *stream;
	char *p = NULL, *tmp;

	if(!(stream = fopen("/etc/pam.d/shadow.conf", "r"))){
		fprintf(stderr, "配置文件遗失\n");
		exit(-1);//TODO //dialog?
	}

	while((r = getline(&p, &r, stream)) > 0){
		if(strstr(p, "server_ip:"))
			break;
	}

	if(r <= 0){
		fprintf(stderr, "配置文件损坏\n");
		exit(-1);//TODO //dialog?
	}
	p[r -1] = 0;
	tmp = p + strlen("server_ip:");

	sprintf(server_head_url, "https://%s/userDetial/getUserImage", tmp);
	free(p);
	fclose(stream);

	sprintf(conf_path, "/home/%s/.umclient", getlogin());
	sprintf(regmask, "%s/regmask", conf_path);

	if(access(conf_path, F_OK)){
		mkdir(conf_path, 0700);
		creat(regmask, 0644);
	}
}

void exit_if_exist(){
	char path[1024];
	char ppath[1024];

	sprintf(path, "%s/umclient.pid", conf_path);

	int fd = open(path, O_RDWR|O_CREAT, 0644);

	if(fd < 0){
		sprintf(path, "应用文件读写失败！");;
		goto error;
	}

	if(!lseek(fd, 0, SEEK_END))
		goto done;
	lseek(fd, 0, SEEK_SET);

	int pid;
	int r = read(fd, &pid, 4);

	if(r <= 0){
		sprintf(path, "应用文件读写失败！");
		goto error;
	}

	sprintf(ppath, "/proc/%d", pid);

	if(access(ppath, F_OK)){
		goto done;
	}

	strcat(ppath, "/comm");

	FILE *file = fopen(ppath, "r");

	if(!file){
		sprintf(path, "文件读写失败！");
		goto error;
	}

	char *str = NULL;
	size_t n = 0;

	if(!getline(&str, &n, file)){
		sprintf(path, "文件读写失败！");
		goto error;
	}

	if(strstr(str, "usermanageclie")){
		sprintf(path, "应用程序已经开启！");
		goto error;
	}

done:
	pid = getpid();
	lseek(fd, 0, SEEK_SET);
	r = write(fd, &pid, 4);
	if(r < 0){
		sprintf(path, "应用文件读写失败！");
		goto error;
	}

	fsync(fd);
	close(fd);

	return;
error:
	close(fd);

	GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
			GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE, "%s", path);

	gtk_widget_show(dialog);

	g_signal_connect_swapped(dialog, "response", G_CALLBACK (gtk_main_quit), dialog);

	gtk_main();
	exit(-1);
}

int is_registered(){
	//return 0;
	int len = 0;
	cJSON *obj = cJSON_CreateObject();
	cJSON *jtype = cJSON_CreateString("1");
	cJSON_AddItemToObject(obj, "type", jtype);
	char *buf = cJSON_Print(obj);
	if(sendreq(sfd, buf) <= 0){
		fprintf(stderr, "服务器错误！\n");
		free(buf);
		return -1;
	}
	if(readn(sfd, (char*)&len, 4) <= 0){
		fprintf(stderr, "服务器错误！\n");
		free(buf);
		return -1;
	}
	buf = realloc(buf, len);
	if(readn(sfd, buf, len) <= 0){
		fprintf(stderr, "服务器错误！\n");
		free(buf);
		return -1;
	}
	obj = cJSON_Parse(buf);
	free(buf);
	if(!obj){
		fprintf(stderr, "Json 解析错误:错误的json格式!\n");
		return -1;
	}
	cJSON *tmp = cJSON_GetObjectItem(obj, "type");
	if(!tmp || tmp->valuestring[0] != '1'){
		fprintf(stderr, "错误的消息!\n");
		cJSON_Delete(obj);
		return -1;
	}
	tmp = cJSON_GetObjectItem(obj, "result");
	if(!tmp){
		fprintf(stderr, "错误的消息!\n");
		cJSON_Delete(obj);
		return -1;
	}
	if(tmp->valuestring[0] == '0'){
		cJSON_Delete(obj);
		return 0;
	}else{
		cJSON_Delete(obj);
		return 1;
	}
	fprintf(stderr, "未知消息!\n");
	return -1;
}

char *error_msg_2_gbk(char *msg){
	char *ret;
	static char *msgs[] = {"密码错误!",
							"未知错误!",
							"用户没有权限!",
							"用户已存在!",
							"用户不存在!",
							"连接错误!",
							"用户名或密码错误!",
							"系统错误",
							"程序正在运行",
							"密码格式错误",
							"邮箱格式错误",
							"邮箱已注册"};
	switch(*msg) {
		case '1':
			if(!msg[1])
				ret = msgs[0];
			else if(msg[1] == '0')
				ret = msgs[9];
			else if(msg[1] == '1')
				ret = msgs[10];
			else if(msg[1] == '2')
				ret = msgs[11];
			break;
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			ret = msgs[*msg - '1'];
			break;
		default:
			ret = msgs[1];
			break;
	}

	return ret;
}

void clear_entry(GtkWidget *widget, gpointer data){
	if(GTK_IS_CONTAINER(widget)){
		gtk_container_foreach(GTK_CONTAINER(widget), clear_entry, data);
	}else if(GTK_IS_ENTRY(widget)){
		gtk_entry_set_text(GTK_ENTRY(widget), "");
	}
}

void *message_main(void *arg){
	int len = 0;
	char *buf = NULL;

	while(1){
		if(readn(sfd, (char*)&len, 4) <= 0)
			break;
		buf = realloc(buf, len);
		if(readn(sfd, buf, len) <= 0){
			break;
		}
		process_message(buf);
	}

	if(buf)
		free(buf);

	exit(-1);

	return NULL;
}

int main(int argc, char **argv){
	GtkWidget *log_win;

	gtk_pool = pool_init(4);
	gtk_init(NULL, NULL);
	curl_global_init(CURL_GLOBAL_ALL);

	app_set_theme("/opt/usermanageclient/theme.css");
	gtk_window_set_default_icon(gdk_pixbuf_new_from_file("/opt/usermanageclient/logo.png", NULL));

	set_conf_dir();
	exit_if_exist();

	sfd = con_with_server("127.0.0.1");
	if(sfd <= 0){
		fprintf(stderr, "无法连接服务器\n");
		return 0;
	}

	if(!access(regmask, F_OK)){
		int r = is_registered();
		if(r < 0)
			return -1;
		else if(r == 0)
			cur_win = um_reg_win();
		else
			cur_win = um_log_win();
	}else{
		cur_win = um_log_win();
	}

	pthread_t message_thread;
	pthread_create(&message_thread, NULL, message_main, NULL);

	curl_global_cleanup();
	gtk_main();
	pool_destroy(gtk_pool);

	return 0;
}

gboolean prompt_show(gpointer user_data){
	gtk_label_set_text(GTK_LABEL(prompt_label), (gchar*)user_data);
	gtk_widget_show(prompt_widget);
	return FALSE;
}

gboolean dialog_show(gpointer user_data){
	GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
		   	GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE, "%s", (gchar*)user_data);
	gtk_widget_show(dialog);
	g_signal_connect_swapped(dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);
	return FALSE;
}

gboolean start_main_win(gpointer user_data){
	GtkWidget *tmp;
	tmp = um_main_win(user_data);
	if(tmp != cur_win){
		gtk_widget_destroy(cur_win);
		cur_win = tmp;
	}
	return FALSE;
}

int process_message(char *buf){
	int r = 0;
	GtkTreeIter iter;
	GdkPixbuf *small, *big;
	static char prompt_message[256];
	struct StoreInfo *data;
	bzero(prompt_message, sizeof(prompt_message));

	cJSON *obj = cJSON_Parse(buf);
	if(!obj){
		fprintf(stderr, "消息错误!\n");
		return -1;
	}
	cJSON *tmp = cJSON_GetObjectItem(obj, "type");
	if(!tmp){
		fprintf(stderr, "消息错误!\n");
		cJSON_Delete(obj);
		return -1;
	}
	r = tmp->valuestring[0];

	tmp = cJSON_GetObjectItem(obj, "result");
	if(!tmp){
		sprintf(prompt_message, "错误的消息格式!");
		r = -1;
	}else if(tmp->valuestring[0] == '0'){
		cJSON *msg_obj = cJSON_GetObjectItem(obj, "reason");
		if(msg_obj)
			sprintf(prompt_message, "错误: %s", error_msg_2_gbk(msg_obj->valuestring));
		r = -1;
	}else if(tmp->valuestring[0] == '1'){
		switch(r){
			case '2':
				gdk_threads_add_idle(start_main_win, obj);
				return 0;
				break;
			case '3':
				if(user_model){
					gtk_list_store_append(user_model, &iter);
					if(remote){
						gtk_list_store_set(user_model, &iter,
								COL_IMAGE_SMALL, def_head.small,
								COL_IMAGE_BIG, def_head.big,
								COL_NAME, req_infos[1].name,
								COL_MAIL, req_infos[1].mail,
								-1);
						clear_entry(gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), 4), NULL);
					}else{
						gtk_list_store_set(user_model, &iter,
								COL_IMAGE_SMALL, def_head.small,
								COL_IMAGE_BIG, def_head.big,
								COL_NAME, req_infos[2].name,
								COL_MAIL, req_infos[2].mail,
								-1);
						clear_entry(gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), 5), NULL);
					}
				}
				sprintf(prompt_message, "注册成功!");
				if(!access(regmask, F_OK)){
					remove(regmask);
				}
				break;
			case '4':
				clear_entry(gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), 1), NULL);
				sprintf(prompt_message, "密码修改成功!");
				break;
			case '5':
				data = malloc(sizeof(struct StoreInfo));

				gtk_list_store_append(user_model, &iter);
				gtk_list_store_set(user_model, &iter,
						COL_IMAGE_SMALL, def_head.small,
						COL_IMAGE_BIG, def_head.big,
						COL_NAME, req_infos[0].name,
						COL_MAIL, req_infos[0].mail,
						-1);

				data->store = user_model;
				data->iter = iter;
				data->mail = req_infos[0].mail;
				data->small = data->big = NULL;
				pool_add(gtk_pool, curl_set_pixbuf, data);

				clear_entry(gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), 3), NULL);
				sprintf(prompt_message, "同步成功!");
				break;
			case '6':
				r = 1;
				if(cur_iter.user_data == del_iter.user_data){
					gtk_tree_model_get_iter_first(GTK_TREE_MODEL(user_model), &cur_iter);
					if(cur_iter.user_data == del_iter.user_data){
						if(!gtk_tree_model_iter_next(GTK_TREE_MODEL(user_model), &cur_iter))
							r = 0;
					}
				}
				if(r){
					gtk_icon_view_select_path(GTK_ICON_VIEW(list_view), gtk_tree_model_get_path(GTK_TREE_MODEL(user_model), &cur_iter));
				}else{
					gtk_label_set_text(GTK_LABEL(info_widgets.nick_label), "————");
					gtk_label_set_text(GTK_LABEL(info_widgets.name_label), "————");
					gtk_label_set_text(GTK_LABEL(info_widgets.mail_label), "————");
				}

				gtk_list_store_remove(user_model, &del_iter);
				//sprintf(prompt_message, "删除成功!");
				r = 0;
				break;
			case '7':
				//gtk_list_store_set(user_model, &cur_iter, COL_IMAGE_SMALL, data->small, COL_IMAGE_BIG, data->big, -1);
				small = pixbuf_from_local(req_infos[3].name, req_infos[3].mail, TRUE);
				big = pixbuf_from_local(req_infos[3].name, req_infos[3].mail, FALSE);
				gtk_list_store_set(user_model, &cur_iter, COL_IMAGE_SMALL, small, COL_IMAGE_BIG, big, -1);
				break;
			default:
				sprintf(prompt_message, "未知错误!");
				r = -1;
				break;
		}
	}else{
		sprintf(prompt_message, "错误：未知结果!");
		r = -1;
	}

	cJSON_Delete(obj);
	if(*prompt_message){
		if(prompt_widget){
			gdk_threads_add_idle(prompt_show, prompt_message);
		}else{
			gdk_threads_add_idle(dialog_show, prompt_message);
		}
	}
	return r;
}
