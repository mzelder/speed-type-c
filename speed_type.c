#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "words.h"

static int countdown = 50;
static GtkWidget *window;
static GtkWidget *timer;
static GtkWidget *entry;  // Declare the entry widget globally if needed for future use

static gboolean update_timer(gpointer user_data) {
    char time[20];
    if (countdown >= 0) {
        sprintf(time, "%d", countdown);
        gtk_label_set_text(GTK_LABEL(timer), time);
        countdown--;
    } else {
        return G_SOURCE_REMOVE;
    }
    return G_SOURCE_CONTINUE;
}

static void activate(GtkApplication *app, gpointer user_data) {
    // Create the main window
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "SpeedType");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);

    // Create a vertical box to hold multiple widgets
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_window_set_child(GTK_WINDOW(window), vbox);

    // Create a timer label
    timer = gtk_label_new("50");
    gtk_box_append(GTK_BOX(vbox), timer); // Add timer label to vbox

    // Create an input entry field
    entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Start typing here...");  // Set placeholder text
    gtk_box_append(GTK_BOX(vbox), entry);  // Add entry to vbox

    // Start the countdown timer
    g_timeout_add(1000, update_timer, NULL);

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    char **words = get_words(); 
    int length = sizeof(words) / sizeof(words[0]);
    for (int i = 0; i < length; i++){
        printf("%s\n", words[i]);
    }

    // Create a new GtkApplication
    app = gtk_application_new(NULL, G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    // Run the application
    status = g_application_run(G_APPLICATION(app), argc, argv);

    // Cleanup
    g_object_unref(app);

    return status;
}
