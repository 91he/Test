#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "cJSON.h"
#include "scrwin.h"

GtkWidget *notebook;
GtkWidget *list_view;
GtkWidget *info_label;
GtkWidget *prompt_widget;
GtkWidget *prompt_label;
GtkWidget *cur_win;
GtkWidget *list_widget;
GtkTreeIter cur_iter;
GtkTreeIter del_iter;
GtkListStore *user_model = NULL;
static int sfd = 0;
static int remote = 0;
char loguser[64] = "root";
char conf_path[512];
char opt_path[512];
char regmask[512];
struct{
	GtkWidget *head_image;
	GtkWidget *nick_label;
	GtkWidget *name_label;
	GtkWidget *mail_label;
}info_widgets;
struct{
	char name[64];
	char mail[64];
}req_infos[4];


enum{
	COL_IMAGE,
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
		add_class(btn, "sbox");
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
		   			crd[0] + event->button.x_root,
					crd[1] + event->button.y_root);
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
	button = gtk_button_new_with_label("修  改 ");
	add_class(button, "theme_color");
	gtk_button_set_image_position(GTK_BUTTON(button), GTK_POS_RIGHT);
	gtk_button_set_image(GTK_BUTTON(button), arrow);
	gtk_button_set_always_show_image(GTK_BUTTON(button), TRUE);
	gtk_box_pack_end(GTK_BOX(head_box), button, FALSE, FALSE, 0);

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

	obj = cJSON_CreateObject();
	cJSON *jtype = cJSON_CreateString("4");
	cJSON *juser = cJSON_CreateString(loguser);
	/*TODO*/
	cJSON *jname = cJSON_CreateString(name);
	cJSON *jold = cJSON_CreateString(pwd);
	cJSON *jnew = cJSON_CreateString(pwd1);

	cJSON_AddItemToObject(obj, "type", jtype);
	cJSON_AddItemToObject(obj, "uname", juser);
	cJSON_AddItemToObject(obj, "uptname", jname);
	cJSON_AddItemToObject(obj, "oldpwd", jold);
	cJSON_AddItemToObject(obj, "pwd", jnew);

	buf = cJSON_Print(obj);
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

GtkListStore *create_store(gpointer user_data){
	int i, n;
	cJSON *array;
	cJSON *obj = user_data;
	GtkTreeIter iter;
	GtkListStore *store;

	store = gtk_list_store_new(NUM_COLS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	gtk_list_store_clear(store);

	if(array = cJSON_GetObjectItem(obj, "remote")){
		n = cJSON_GetArraySize(array);
		for(i = 0; i < n; i++){
			cJSON *user = cJSON_GetArrayItem(array, i);
			gtk_list_store_append(store, &iter);
			gtk_list_store_set(store, &iter,
				   			   COL_IMAGE, "/opt/usermanageclient/default.png",
				   			   COL_NAME, cJSON_GetObjectItem(user, "uname")->valuestring,
							   COL_MAIL, cJSON_GetObjectItem(user, "email")->valuestring,
							   -1);
		}
	}

	if(array = cJSON_GetObjectItem(obj, "local")){
		n = cJSON_GetArraySize(array);
		for(i = 0; i < n; i++){
			cJSON *user = cJSON_GetArrayItem(array, i);
			gtk_list_store_append(store, &iter);
			gtk_list_store_set(store, &iter,
				   			   COL_IMAGE, "/opt/usermanageclient/default.png",
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
	const gchar *name, *mail, *image;
	
	list = gtk_icon_view_get_selected_items(iconview);
	if(!list){
		gtk_icon_view_select_path(GTK_ICON_VIEW(iconview),
			   					  gtk_tree_model_get_path(GTK_TREE_MODEL(user_model), iter));
		return;
	}

	gtk_tree_model_get_iter(GTK_TREE_MODEL(user_model), iter, list->data);
	gtk_tree_model_get(GTK_TREE_MODEL(user_model), iter,
		   			   COL_IMAGE, &image,
		   			   COL_NAME, &name,
					   COL_MAIL, &mail,
					   -1);

	pixbuf = gdk_pixbuf_new_from_file_at_size(image, 75, 75, NULL);
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
	
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_cell_renderer_set_fixed_size(renderer, 38, -1);
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(icon_view), renderer, FALSE);
	gtk_cell_layout_set_cell_data_func(GTK_CELL_LAYOUT(icon_view), renderer, set_cell_image, NULL, NULL);

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
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), um_info_page(user_model), NULL);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), um_passwd_page(), NULL);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), gtk_box_new(GTK_ORIENTATION_VERTICAL, 0), NULL);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), um_sync_page(), NULL);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), um_reg_page(TRUE), NULL);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), um_reg_page(FALSE), NULL);

	gtk_container_add(GTK_CONTAINER(overlay), notebook);
	gtk_overlay_add_overlay(GTK_OVERLAY(overlay), um_prompt());
	gtk_overlay_add_overlay(GTK_OVERLAY(overlay), um_scrwin(user_model));
	gtk_widget_show_all(overlay);

	return overlay;
}

GtkWidget *um_main_box(gpointer user_data){
	GtkWidget *box;
	GtkWidget *box1;
	GtkWidget *box2;

	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

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
	GtkWidget *window;//= NULL;

	//if(window) return window;

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
	sprintf(opt_path, "/opt/umclient");
	sprintf(conf_path, "/home/%s/.umclient", getlogin());

	if(access(conf_path, F_OK)){
		mkdir(conf_path, 0700);
		sprintf(regmask, "%s/regmask", conf_path);
		creat(regmask, 0644);
	}
	sprintf(regmask, "%s/regmask", conf_path);
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
	static char *msgs[7] = {"密码错误!",
							"未知错误!",
							"用户没有权限!",
							"用户已存在!",
							"用户不存在!",
							"连接错误!",
							"用户名或密码错误!"};
	switch(*msg) {
		case '1':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
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

	gtk_init(NULL, NULL);
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

	gtk_main();

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
	gtk_widget_destroy(cur_win);
	cur_win = um_main_win(user_data);
	return FALSE;
}

int process_message(char *buf){
	int r = 0;
	GtkTreeIter iter;
	static char prompt_message[256];
	//bzero(prompt_message, sizeof(prompt_message));

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
		sprintf(prompt_message, "错误: %s", error_msg_2_gbk(cJSON_GetObjectItem(obj, "reason")->valuestring));
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
								COL_IMAGE, "/opt/usermanageclient/default.png",
								COL_NAME, req_infos[1].name,
								COL_MAIL, req_infos[1].mail,
								-1);
						clear_entry(gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), 4), NULL);
					}else{
						gtk_list_store_set(user_model, &iter,
								COL_IMAGE, "/opt/usermanageclient/default.png",
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
				gtk_list_store_append(user_model, &iter);
				gtk_list_store_set(user_model, &iter,
						COL_IMAGE, "/opt/usermanageclient/default.png",
						COL_NAME, req_infos[0].name,
						COL_MAIL, req_infos[0].mail,
						-1);
				clear_entry(gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), 3), NULL);
				sprintf(prompt_message, "同步成功!");
				break;
			case '6':
				/*TODO*/
				//remove user from liststore
				//printf("%d %p\n", cur_iter.stamp, cur_iter.user_data);
				//printf("%d %p\n", del_iter.stamp, del_iter.user_data);
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
				sprintf(prompt_message, "删除成功!");
				return 0;
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
