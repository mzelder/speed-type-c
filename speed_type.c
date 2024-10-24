#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "words.h"

static int countdown = 50;
static char *sentence;
static char** words;
static GtkWidget *window;
static GtkWidget *timer;
static GtkWidget *entry; 
static GtkWidget *sentence_label; 

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

static void on_entry_changed(GtkWidget *widget, gpointer user_data) {
    GtkEntryBuffer *buffer = gtk_entry_get_buffer(GTK_ENTRY(entry));
    const gchar *input_text = gtk_entry_buffer_get_text(buffer);
    int input_length  = strlen(input_text);
    if (input_length == 0) return;

    // for (int i = 0; i < input_length; i++) { 
    //     if (input_text[i] == sentence[i]) {
    //         g_print("%s")
    //     }
    // }
    // if (flag)
}

static void activate(GtkApplication *app, gpointer user_data) {
    // Create the main window
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "SpeedType");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);

    // Create a vertical box to hold multiple widgets
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_window_set_child(GTK_WINDOW(window), vbox);

    // Create sentence label
    words = get_words();
    sentence = get_sentence(words);
    sentence_label = gtk_label_new(sentence);
    gtk_box_append(GTK_BOX(vbox), sentence_label);

    // Create a timer label
    timer = gtk_label_new("50");
    gtk_box_append(GTK_BOX(vbox), timer); // Add timer label to vbox

    // Create an input entry field
    entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Start typing here...");  // Set placeholder text
    gtk_box_append(GTK_BOX(vbox), entry);  // Add entry to vbox

    g_signal_connect(entry, "changed", G_CALLBACK(on_entry_changed), NULL);

    // Start the countdown timer
    g_timeout_add(1000, update_timer, NULL);

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    // Create a new GtkApplication
    app = gtk_application_new(NULL, G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    // Run the application
    status = g_application_run(G_APPLICATION(app), argc, argv);

    // Cleanup
    g_object_unref(app);

    return status;
}
