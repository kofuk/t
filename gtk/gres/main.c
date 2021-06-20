#include <gtk/gtk.h>

void activate(GtkApplication *app, G_GNUC_UNUSED gpointer user_data) {
    GtkBuilder *builder = gtk_builder_new();
    gtk_builder_add_from_resource(builder, "/org/kofuk/s/Gres/ui/gres.ui",
                                  NULL);

    GObject *window = gtk_builder_get_object(builder, "window");
    gtk_window_set_application(GTK_WINDOW(window), app);

    gtk_widget_show(GTK_WIDGET(window));

    g_object_unref(builder);
}

int main(int argc, char **argv) {
    GtkApplication *app =
        gtk_application_new("org.kofuk.s.Gres", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
