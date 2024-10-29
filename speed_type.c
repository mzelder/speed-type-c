#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "words.h"

static int countdown = 50;
static bool first_iter = true;

static char *sentence;
static int sentence_counter = 0;
static int start_of_the_bad_letter;

static char** words;
static int word_counter = 0;
static char *current_word;
static int word_length;

static int previous_length;

static GtkWidget *window;
static GtkWidget *timer;
static GtkWidget *entry; 
static GtkWidget *sentence_label; 
static GtkWidget *markup_string;

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

static void format_sentence(bool correctness) {
    char *already_done_sentence; 
    char *rest_of_sentence;
    GString *markup_string = g_string_new("");

    if (sentence_counter == -1) {
        rest_of_sentence = strdup(sentence + sentence_counter + 1);
        g_string_append_printf(markup_string, "<span font='24'>%s</span>", rest_of_sentence);
    } else {
        g_print("Sen counter: %i\n", sentence_counter);
        // Add word that already done
        already_done_sentence = strndup(sentence, sentence_counter);
        g_print("DONE: %s\n", already_done_sentence);
        g_string_append_printf(markup_string, "<span font='24' foreground='green'>%s</span>", already_done_sentence);

        // Add current word 

        char current_letter[2];
        current_letter[0] = sentence[sentence_counter];
        current_letter[1] = '\0';
        const char *color = correctness ? "green" : "red";
        g_print("CURRENT LETTER: %s\n", current_letter);
        g_string_append_printf(markup_string, "<span font='24' foreground='%s'>%s</span>", color, current_letter);
        
        // Add rest of the sentence
        rest_of_sentence = strdup(sentence + sentence_counter+1);
        g_string_append_printf(markup_string, "<span font='24'>%s</span>", rest_of_sentence);
        g_print("REST: %s\n", rest_of_sentence);
    }
    

    gtk_label_set_markup(GTK_LABEL(sentence_label), markup_string->str);
    g_free(markup_string);
}

static void button_pressed(int current_length) {
    if (previous_length > current_length) sentence_counter--;
    else sentence_counter++;
    previous_length = current_length;
}       

static void another_word() {    
    word_counter++;
    sentence_counter++;
    current_word = words[word_counter];
    word_length = strlen(current_word);
    current_word[word_length] = ' ';
}

static void on_entry_changed(GtkWidget *widget, gpointer user_data) {
    GtkEntryBuffer *buffer = gtk_entry_get_buffer(GTK_ENTRY(entry));
    const gchar *input_text = gtk_entry_buffer_get_text(buffer);
    int input_length = strlen(input_text);
    bool is_correct= true; 
    
    if (!first_iter) button_pressed(input_length);
    if (input_length == 0) format_sentence(true);
    for (int i = 0; i < input_length; i++) {
        if (input_text[i] == current_word[i]) {
            format_sentence(true);
        } else {
            is_correct = false;
            format_sentence(false);
        }
    }
    
    // Space pressed at the end of the word
    if (input_text[input_length-1] == ' ' && is_correct) {
        gtk_entry_buffer_set_text(buffer, "", -1);
        gtk_entry_set_buffer(GTK_ENTRY(widget), buffer);
        another_word();
    }

    first_iter = false;
}

static void activate(GtkApplication *app, gpointer user_data) {
    // Create the main window
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "SpeedType");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

    // Create a vertical box to hold multiple widgets
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_window_set_child(GTK_WINDOW(window), vbox);

    // Create sentence label
    words = get_words();
    current_word = words[word_counter];
    word_length = strlen(current_word);
    sentence = get_sentence(words);
    current_word[word_length] = ' ';
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
