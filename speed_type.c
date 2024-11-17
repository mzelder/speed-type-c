#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "words.h"

static int countdown = 60;
static int elapsed_seconds = 0;

static char *sentence;

static char** words;
static int words_length; 
static char *current_word;
static int word_length;
static int word_counter = 0;
static int *previous_input;

static GtkWidget *window;
static GtkWidget *timer;
static GtkWidget *entry; 
static GtkWidget *sentence_label;
static GtkWidget *race_label;
static GtkWidget *wpm_label; 
static GtkWidget *progress_bar;
static GtkWidget *rocket_image; 
static GtkWidget *rocket_container;

static void show_statistics() {
    // Calculate statistics
    int wpm = (elapsed_seconds > 0) ? (word_counter * 60 / elapsed_seconds) : 0;

    // Create a new layout for the statistics (GtkBox with vertical orientation)
    GtkWidget *stats_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);  // Adjust spacing between elements
    gtk_widget_set_margin_top(stats_box, 50);
    gtk_widget_set_margin_bottom(stats_box, 50);
    gtk_widget_set_margin_start(stats_box, 50);
    gtk_widget_set_margin_end(stats_box, 50);

    // Create a detailed statistics message
    GString *details = g_string_new("");
    g_string_append_printf(details, "<span font='36' weight='bold' foreground='#4CAF50'>Typing Statistics</span>\n");  // Increase font size for the title and change color
    g_string_append_printf(details, "<span font='28'>Words Per Minute (WPM): <b>%d</b></span>\n", wpm);
    g_string_append_printf(details, "<span font='28'>Time Elapsed: <b>%d seconds</b></span>", elapsed_seconds);

    // Create a label to show the detailed statistics message
    GtkWidget *stats_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(stats_label), details->str);

    // Add the label to the box (container)
    gtk_box_append(GTK_BOX(stats_box), stats_label);

    // Add a "Close" button to exit the program
    GtkWidget *close_button = gtk_button_new_with_label("Close");
    gtk_widget_set_size_request(close_button, 150, 50);  // Adjust button size
    gtk_widget_set_margin_top(close_button, 20);  // Add some space above the button
    gtk_widget_set_margin_bottom(close_button, 10);  // Add some space below the button

    // Connect the button to close the window
    g_signal_connect(close_button, "clicked", G_CALLBACK(gtk_window_close), window);
    
    // Add the button to the box
    gtk_box_append(GTK_BOX(stats_box), close_button);

    // Clear the current content of the window (if any)
    GtkWidget *current_child = gtk_window_get_child(GTK_WINDOW(window));
    if (current_child) {
        gtk_window_set_child(GTK_WINDOW(window), NULL); // Remove the current child
    }

    // Set the new layout as the window content
    gtk_window_set_child(GTK_WINDOW(window), stats_box);

    // Free the GString memory
    g_string_free(details, TRUE);
}

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
        show_statistics();
        return G_SOURCE_REMOVE;
    }
    return G_SOURCE_CONTINUE;
}

static void format_sentence(int first_incorrect_indx, int current_length) {
    GString *markup_string = g_string_new("");

    for (int i = 0; i < word_counter; i++) {
        g_string_append_printf(markup_string, "<span font='24' foreground='green'>%s</span>", words[i]);
    }

    for (int i = 0; i < current_length; i++) {
        if (current_word[i] == ' ') continue;
        char *color = (i < first_incorrect_indx || first_incorrect_indx == -1) ? "green" : "red";
        g_string_append_printf(markup_string, "<span font='24' underline='single' foreground='%s'>%c</span>", color, current_word[i]);
    }
    for (int i = current_length; i < word_length; i++) { 
        g_string_append_printf(markup_string, "<span font='24'>%c</span>", current_word[i]);
    }
    g_string_append_printf(markup_string, "<span font='24'> </span>");

    for (int i = word_counter + 1; i < words_length; i++) {
        g_string_append_printf(markup_string, "<span font='24'>%s </span>", words[i]);
    }
    
    gtk_label_set_markup(GTK_LABEL(sentence_label), markup_string->str);
    g_free(markup_string);
}

static void another_word() {    
    word_counter++;
    
    if (word_counter >= words_length) {
        show_statistics();
        return;
    }
    
    current_word = words[word_counter];
    word_length = strlen(current_word);
    if (word_counter != words_length) {
        current_word[word_length] = ' ';
    }
}

static void move_rocket() {
    double fraction = (double)word_counter / words_length;
    int x_position = (int)(fraction * 700);
    gtk_widget_set_margin_start(rocket_image, x_position);
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

    bool check = input_text[input_length-1] == ' ';
    if (check && is_correct) {
        gtk_entry_buffer_set_text(buffer, "", -1);
        gtk_entry_set_buffer(GTK_ENTRY(widget), buffer);
        another_word();
        input_length = 0;
    }
    
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), (double)word_counter / words_length);
    move_rocket();
    format_sentence(first_incorrect_indx, input_length);
}

static void activate(GtkApplication *app, gpointer user_data) {
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "SpeedType");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, -1);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

    GtkCssProvider *css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(css_provider, "style.css");
    gtk_style_context_add_provider_for_display(gdk_display_get_default(), GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *top_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 500);
    GtkWidget *mid_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *bottom_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *progress_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    gtk_window_set_child(GTK_WINDOW(window), main_box);
    gtk_widget_set_margin_start(main_box, 20);  
    gtk_widget_set_margin_end(main_box, 20);    
    gtk_widget_set_margin_top(main_box, 20);    
    gtk_widget_set_margin_bottom(main_box, 20); 

    race_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(race_label), "<span font='30' weight='bold'>The race is on! Type the text below:</span>");
    gtk_box_append(GTK_BOX(top_box), race_label);

    timer = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(timer), "<span font='30' weight='bold'>60</span>");
    gtk_box_append(GTK_BOX(top_box), timer);

    rocket_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    rocket_image = gtk_image_new_from_file("rocket.png");
    gtk_widget_set_size_request(GTK_WIDGET(rocket_image), 150, 150);
    gtk_box_append(GTK_BOX(rocket_container), rocket_image);
    gtk_box_append(GTK_BOX(progress_box), rocket_container);

    progress_bar = gtk_progress_bar_new();
    gtk_widget_set_size_request(GTK_WIDGET(progress_bar), 800, -1);
    gtk_box_append(GTK_BOX(progress_box), progress_bar);
    gtk_box_append(GTK_BOX(mid_box), progress_box);

    wpm_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(wpm_label), "<span font='24' weight='bold'>WPM: 0</span>");
    gtk_widget_set_margin_top(wpm_label, 10);
    gtk_widget_set_margin_bottom(wpm_label, 10);
    gtk_widget_set_margin_start(wpm_label, 5);
    gtk_widget_set_margin_end(wpm_label, 5);
    gtk_box_append(GTK_BOX(mid_box), wpm_label);

    words = get_words();
    words_length = get_words_length();
    current_word = words[word_counter];
    word_length = strlen(current_word);
    sentence = get_sentence(words);
    current_word[word_length] = ' ';
    
    sentence_label = gtk_label_new(sentence);
    gtk_label_set_wrap(GTK_LABEL(sentence_label), TRUE); 
    gtk_label_set_wrap_mode(GTK_LABEL(sentence_label), PANGO_WRAP_WORD);
    gtk_label_set_xalign(GTK_LABEL(sentence_label), 0.0);
    const char *format = "<span font='24'>%s</span>";
    char * markup = g_markup_printf_escaped(format, sentence);
    gtk_label_set_markup(GTK_LABEL(sentence_label), markup);
    gtk_box_append(GTK_BOX(bottom_box), sentence_label);

    entry = gtk_entry_new();
    GtkEntryBuffer *buffer = gtk_entry_buffer_new("", -1);
    gtk_box_append(GTK_BOX(bottom_box), entry); 

    gtk_widget_set_name(race_label, "race_label");
    gtk_widget_set_name(entry, "entry");
    gtk_widget_set_name(progress_bar, "progress_bar");
    gtk_widget_set_name(sentence_label, "sentence_label");
    gtk_widget_set_name(wpm_label, "wpm_label");
    gtk_widget_set_name(timer, "timer");

    gtk_box_append(GTK_BOX(main_box), top_box);
    gtk_box_append(GTK_BOX(main_box), mid_box);
    gtk_box_append(GTK_BOX(main_box), bottom_box);

    g_signal_connect(entry, "changed", G_CALLBACK(on_entry_changed), NULL);

    g_timeout_add(1000, update_timer, NULL);

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new(NULL, G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    status = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref(app);

    return status;
}
