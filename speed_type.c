#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "words.h"

static int countdown = 180;
static int elapsed_seconds = 0;

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
static GtkWidget *race_label;
static GtkWidget *wpm_label; 
static GtkWidget *markup_string;
static GtkWidget *progress_bar;

static gboolean update_timer(gpointer user_data) {
    char time[20];
    char markup[100];
    
    if (countdown >= 0) {
        sprintf(time, "%d", countdown);
        sprintf(markup, "<span font='30' weight='bold'>%s</span>", time);
        gtk_label_set_markup(GTK_LABEL(timer), markup);
        countdown--;
        elapsed_seconds++;

        int wpm = (elapsed_seconds > 0) ? (word_counter * 60 / elapsed_seconds) : 0;
        sprintf(markup, "<span font='30' weight='bold'>WPM: %d</span>", wpm);
        gtk_label_set_markup(GTK_LABEL(wpm_label), markup);
    } else {
        return G_SOURCE_REMOVE;
    }
    return G_SOURCE_CONTINUE;
}

static void format_sentence(int first_incorrect_indx, int current_length) {
    GString *markup_string = g_string_new("");

    // Already written words
    for (int i = 0; i < word_counter; i++) {
        g_string_append_printf(markup_string, "<span font='24' foreground='green'>%s</span>", words[i]);
    }

    // Current word  
    for (int i = 0; i < current_length; i++) {
        if (current_word[i] == ' ') continue;
        char *color = (i < first_incorrect_indx || first_incorrect_indx == -1) ? "green" : "red";
        g_string_append_printf(markup_string, "<span font='24' underline='single' foreground='%s'>%c</span>", color, current_word[i]);
    }
    for (int i = current_length; i < word_length; i++) { 
        g_string_append_printf(markup_string, "<span font='24'>%c</span>", current_word[i]);
    }
    g_string_append_printf(markup_string, "<span font='24'> </span>");

    // Words that wasnt typed 
    for (int i = word_counter + 1; i < words_length; i++) {
        g_string_append_printf(markup_string, "<span font='24'>%s </span>", words[i]);
    }
    
    gtk_label_set_markup(GTK_LABEL(sentence_label), markup_string->str);
    g_free(markup_string);
}

static void another_word() {    
    word_counter++;
    current_word = words[word_counter];
    word_length = strlen(current_word);
    if (word_counter != words_length) {
        current_word[word_length] = ' ';
    }
}

static void on_entry_changed(GtkWidget *widget, gpointer user_data) {
    GtkEntryBuffer *buffer = gtk_entry_get_buffer(GTK_ENTRY(entry));
    const gchar *input_text = gtk_entry_buffer_get_text(buffer);
    int input_length = strlen(input_text);
    bool is_correct= true;
    int first_incorrect_indx = -1;

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
    if (check && is_correct) {
        gtk_entry_buffer_set_text(buffer, "", -1);
        gtk_entry_set_buffer(GTK_ENTRY(widget), buffer);
        another_word();
        input_length = 0;
    }
    
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), (double)word_counter / words_length);
    format_sentence(first_incorrect_indx, input_length);
}

static void activate(GtkApplication *app, gpointer user_data) {
    // Create the main window
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "SpeedType");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 300);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

    // Load CSS
    GtkCssProvider *css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(css_provider, "style.css");
    gtk_style_context_add_provider_for_display(gdk_display_get_default(), GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);

    // Create a vertical box to hold multiple widgets
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 50);
    GtkWidget *top_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 500);
    GtkWidget *mid_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 500);
    GtkWidget *bottom_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    gtk_window_set_child(GTK_WINDOW(window), main_box);
    gtk_widget_set_margin_start(main_box, 20);  
    gtk_widget_set_margin_end(main_box, 20);    
    gtk_widget_set_margin_top(main_box, 20);    
    gtk_widget_set_margin_bottom(main_box, 20); 

    race_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(race_label), "<span font='30' weight='bold'>The race is on! Type the text below:</span>");
    gtk_box_append(GTK_BOX(top_box), race_label);

    timer = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(timer), "<span font='30' weight='bold'>180</span>");
    gtk_box_append(GTK_BOX(top_box), timer);

    progress_bar = gtk_progress_bar_new();
    gtk_widget_set_size_request(GTK_WIDGET(progress_bar), 500, 30);
    gtk_box_append(GTK_BOX(mid_box), progress_bar);

    wpm_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(wpm_label), "<span font='30' weight='bold'>WPM: 0</span>");
    gtk_widget_set_margin_top(wpm_label, 10);
    gtk_widget_set_margin_bottom(wpm_label, 10);
    gtk_widget_set_margin_start(wpm_label, 5);
    gtk_widget_set_margin_end(wpm_label, 5);
    gtk_box_append(GTK_BOX(mid_box), wpm_label);

    // Adding all boxes to main one 
    gtk_box_append(GTK_BOX(main_box), top_box);
    gtk_box_append(GTK_BOX(main_box), mid_box);
    gtk_box_append(GTK_BOX(main_box), bottom_box);

    // // Create sentence label
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
    gtk_box_append(GTK_BOX(bottom_box), sentence_label);

    // // Create an input entry field
    entry = gtk_entry_new();
    GtkEntryBuffer *buffer = gtk_entry_buffer_new("", -1);
    gtk_box_append(GTK_BOX(bottom_box), entry); 

    // Set classes for elements
    gtk_widget_set_name(race_label, "race_label");
    gtk_widget_set_name(entry, "entry");
    gtk_widget_set_name(progress_bar, "progress_bar");

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
