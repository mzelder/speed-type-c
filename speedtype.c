#include <gtk/gtk.h>

static void print_hello(GtkWidget *widget, gpointer data) {
    g_print("Hello World\n");
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *button;

    // Create a new window
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Hello");
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);

    // Create a button with the label "Hello World"
    button = gtk_button_new_with_label("Hello World");

    // Connect the "clicked" signal of the button to the print_hello callback
    g_signal_connect(button, "clicked", G_CALLBACK(print_hello), NULL);

    // Set the button as the child of the window
    gtk_window_set_child(GTK_WINDOW(window), button);

    // Present the window
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    // Create a new GtkApplication
    app = gtk_application_new(NULL, G_APPLICATION_DEFAULT_FLAGS);

    // Connect the "activate" signal to the activate function
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    // Run the application
    status = g_application_run(G_APPLICATION(app), argc, argv);

    // Cleanup
    g_object_unref(app);

    return status;
}
