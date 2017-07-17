#include <stdio.h>
#include <gio/gio.h>
#include <gio/gunixsocketaddress.h>
#include <stdlib.h>

static GMainLoop *loop;

void mount_done_cb (GObject *object, GAsyncResult *res, gpointer user_data){
    gboolean succeeded;
    GError *error = NULL;

    succeeded = g_file_mount_enclosing_volume_finish(G_FILE(object), res, &error);
    if(succeeded) printf("mount succeed!\n");
    else printf("mount failed: %s!\n", error->message);

    //g_main_loop_quit(loop);
}

void ask_password_cb (GMountOperation *op,
        const char      *message,
        const char      *default_user,
        const char      *default_domain,
        GAskPasswordFlags flags){
#if 1
    if ((flags & G_ASK_PASSWORD_ANONYMOUS_SUPPORTED)){
        g_mount_operation_reply(op, G_MOUNT_OPERATION_HANDLED);
    }else{
        g_mount_operation_reply(op, G_MOUNT_OPERATION_ABORTED);
    }
#endif
}

void changed(GFileMonitor     *monitor,
             GFile            *file,
             GFile            *other_file,
             GFileMonitorEvent event_type,
             gpointer          user_data){
    printf("%s:%s\n", event_type == G_FILE_MONITOR_EVENT_CREATED ? "Create" : "Delete", g_file_get_parse_name(file));

    printf("event_type: %d\n", event_type);
    if(event_type == G_FILE_MONITOR_EVENT_DELETED){
        GMountOperation *op = g_mount_operation_new();
        {
            g_mount_operation_set_anonymous (op, TRUE);
            //g_mount_operation_set_user(op, "someone");
            //g_mount_operation_set_password(op, "123456");
        }
        g_signal_connect(op, "ask_password", G_CALLBACK(ask_password_cb), NULL);
        g_file_mount_enclosing_volume(file, 0, op, NULL, mount_done_cb, NULL);
    }
}

int main(int argc, char **argv){
#if 0
    GError *error = NULL;
    GDBusConnection *con = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
    if(!con){
        printf("%s\n", error->message);
        return -1;
    }
    GSocketConnection *scon = (GSocketConnection*)g_dbus_connection_get_stream(con);
    GSocketAddress *saddr = g_socket_connection_get_remote_address(scon, NULL);
    const gchar *path = g_unix_socket_address_get_path((GUnixSocketAddress*)saddr);
    const gchar *guid = g_dbus_connection_get_guid(con);
    char address[256];
    sprintf(address, "unix:abstract=%s,guid=%s", path, guid);
    printf("%s\n", address);

    char *env = getenv("DBUS_SESSION_BUS_ADDRESS");
    if(env) printf("B:\t%s\n", env);
    setenv("DBUS_SESSION_BUS_ADDRESS", address, 1);
    env = getenv("DBUS_SESSION_BUS_ADDRESS");
    //printf("%s\n", getenv("DBUS_SESSION_BUS_ADDRESS"));
    if(env) printf("A:\t%s\n", env);
#endif

    loop = g_main_loop_new(NULL, FALSE);
#if 1
    GFile *gFile = g_file_new_for_commandline_arg(argv[1]);

    if(!gFile){
        perror("g_file_new_for_commandline_arg");
        return -1;
    }

    GMountOperation *op = g_mount_operation_new();
    {
        g_mount_operation_set_anonymous (op, TRUE);
        //g_mount_operation_set_user(op, "someone");
        //g_mount_operation_set_password(op, "123456");
    }
    g_signal_connect(op, "ask_password", G_CALLBACK(ask_password_cb), NULL);

    GFileMonitor *monitor = g_file_monitor(gFile, G_FILE_MONITOR_WATCH_MOUNTS | G_FILE_MONITOR_SEND_MOVED, NULL, NULL);
    g_signal_connect(monitor, "changed", G_CALLBACK(changed), NULL);

    g_file_mount_enclosing_volume(gFile, 0, op, NULL, mount_done_cb, NULL);

#endif
    g_main_loop_run(loop);
    g_object_unref(gFile);

    return 0;
}
