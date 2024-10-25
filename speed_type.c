#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "words.h"
//1876254864
#define DELETE_BUTTON 1836786512

static int countdown = 50;
static char *sentence;
static int sentence_counter = 0;
static char** words;
static int word_counter = 0;
static int global_input_length;
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

static void update_sentence_color(bool accurate) {
    char *rest_of_sentence;
    GString *markup_string = g_string_new("");

    for (int i = 0; i < sentence_counter; i++) { 
        char current_letter[2];
        current_letter[0] = sentence[i];
        current_letter[1] = '\0';

        const char *color = accurate ? "green" : "red";
        g_string_append_printf(markup_string, "<span font='24' foreground='%s'>%s</span>", color, current_letter);

    }
    
    rest_of_sentence = strdup(sentence + sentence_counter);
    g_string_append_printf(markup_string, "<span font='24'>%s</span>", rest_of_sentence);

    // g_print("%s", rest_of_sentence);
    gtk_label_set_markup(GTK_LABEL(sentence_label), markup_string->str);
    g_free(markup_string);
}

static void button_pressed(int input_length) {
    g_print("GLOBA:: %d\n", global_input_length);
    g_print("LOCAL: %d\n", input_length);

    if (global_input_length > input_length) {
        g_print("GLOBAL > LOCAL\n");
        sentence_counter--;
    } else { 
        g_print("GLOBAL < LOCAL\n");
        sentence_counter++;
    }
    g_print("%d", sentence_counter);
    global_input_length = input_length;
}

static void on_entry_changed(GtkWidget *widget, gpointer user_data) {
    GtkEntryBuffer *buffer = gtk_entry_get_buffer(GTK_ENTRY(entry));
    const gchar *input_text = gtk_entry_buffer_get_text(buffer);
    
    int input_length = strlen(input_text);
    
    button_pressed(input_length);

    char* curent_word = words[word_counter];
    int word_length = strlen(curent_word);
    
    for (int i = 0; i < input_length; i++) {
        if (input_text[i] == ' ' && input_length == word_length + 1) {
            gtk_entry_buffer_set_text(buffer, "", -1);
            gtk_entry_set_buffer(GTK_ENTRY(widget), buffer);
            update_sentence_color(true);
            word_counter++;
            break;
        }

        if (input_text[i] == curent_word[i]) {
            update_sentence_color(true);
        } else {
            update_sentence_color(false);
            break;
        }
    }
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
    const char *format = "<span font='24'>%s</span>";
    char * markup = g_markup_printf_escaped(format, sentence);
    gtk_label_set_markup(GTK_LABEL(sentence_label), markup);
    gtk_box_append(GTK_BOX(vbox), sentence_label);

    // Create a timer label
    timer = gtk_label_new("50");
    gtk_box_append(GTK_BOX(vbox), timer); // Add timer label to vbox

    // Create an input entry field
    entry = gtk_entry_new();
    GtkEntryBuffer *buffer = gtk_entry_buffer_new("", -1);
    gtk_box_append(GTK_BOX(vbox), entry); 

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
