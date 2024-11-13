#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "words.h"

static int countdown = 50;

static char *sentence;

static char** words;
static int words_length; 
static char *current_word;
static int word_length;
static int word_counter = 0;

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

static void format_sentence(int first_incorrect_indx, int current_length) {
    GString *markup_string = g_string_new("");
    g_print("WORD COUNTER: %d\n", word_counter);
    g_print("WORDS_LENGTH: %d\n", words_length);
    g_print("CURRENT_WORD: %s\n", current_word);

    // Fill done_words with completed words
    for (int i = 0; i < word_counter; i++) {
        g_string_append_printf(markup_string, "<span font='24' foreground='green'>%s</span>", words[i]);
    }

    // Already typed letters 
    for (int i = 0; i < current_length; i++) {
        if (current_word[i] == ' ') continue;
        else if (i < first_incorrect_indx || first_incorrect_indx == -1) {
            g_string_append_printf(markup_string, "<span font='24' foreground='green'>%c</span>", current_word[i]);
        } else {
            g_string_append_printf(markup_string, "<span font='24' foreground='red'>%c</span>", current_word[i]);
        }
    }
    // Rest of the world 
    for (int i = current_length; i < word_length; i++) { 
        g_string_append_printf(markup_string, "<span font='24'>%c</span>", current_word[i]);
    }
    g_string_append_printf(markup_string, "<span font='24'> </span>");

    // Add rest of the sentence to rest_words with correct indexing
    for (int i = word_counter + 1; i < words_length; i++) {
        g_string_append_printf(markup_string, "<span font='24'>%s </span>", words[i]);
    }
    
    // Display the markup in the label
    gtk_label_set_markup(GTK_LABEL(sentence_label), markup_string->str);

    // Free dynamically allocated memory
    g_free(markup_string);
}

static void another_word() {    
    word_counter++;
    current_word = words[word_counter];
    word_length = strlen(current_word);
    current_word[word_length] = ' ';
}

static void on_entry_changed(GtkWidget *widget, gpointer user_data) {
    GtkEntryBuffer *buffer = gtk_entry_get_buffer(GTK_ENTRY(entry));
    const gchar *input_text = gtk_entry_buffer_get_text(buffer);
    int input_length = strlen(input_text);
    bool is_correct= true;
    int first_incorrect_indx = -1;

    g_print("INPUT TEXT: %s\n", input_text);
    g_print("INPUT LENGTH: %i\n", input_length);
    
    for (int i = 0; i < input_length; i++) {
        if (input_text[i] == current_word[i]) {
            continue;
        } else {
            is_correct = false;
            if (first_incorrect_indx == -1) {
                first_incorrect_indx = i;
            }
        }
    }

    // Space pressed at the end of the word
    bool check = input_text[input_length-1] == ' ';
    g_print("IS correct: %i\n", is_correct);
    g_print("IS length good: %i\n", check);
    if (check && is_correct) {
        gtk_entry_buffer_set_text(buffer, "", -1);
        gtk_entry_set_buffer(GTK_ENTRY(widget), buffer);
        another_word();
        g_print("DONE! DONE!\n");
        input_length = 0;
    }
    
    format_sentence(first_incorrect_indx, input_length);
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
    words_length = get_words_length();
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
