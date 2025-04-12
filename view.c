/*#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "model.h"
#include "view.h"
#include "controller.h"

#define MAX_COMMAND_LEN 100
#define MSG_BUF_SIZE 1024

// Global widgets
GtkTextBuffer *global_text_buffer = NULL;
GtkWidget *global_text_view = NULL;

static int terminal_counter = 0;
static GtkWidget *parent_terminal_window = NULL;
static GtkWidget *child_terminal_window = NULL;

// Güvenli çıktıyı text buffer'a yazan fonksiyon
static void append_output(GtkTextBuffer *buffer, const char *text) {
    if (!GTK_IS_TEXT_BUFFER(buffer)) {
        fprintf(stderr, "[ERROR] append_output: Invalid buffer!\n");
        return;
    }

    GtkTextIter end;
    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_insert(buffer, &end, text, -1);

    if (GTK_IS_TEXT_VIEW(global_text_view)) {
        GtkTextMark *mark = gtk_text_buffer_create_mark(buffer, "end", &end, FALSE);
        gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(global_text_view), mark, 0.0, FALSE, 0.0, 0.0);
    }
}

void on_command_entry_activate(GtkEntry *entry, gpointer user_data) {
    const char *command = gtk_entry_get_text(entry);

    if (strcmp(command, "exit") == 0) {
        gtk_main_quit();
    } else {
        int terminal_id = 1;
        handle_command(command, terminal_id);
    }
    gtk_entry_set_text(entry, "");
}

void on_send_message(GtkButton *button, gpointer user_data) {
    GtkEntry *entry = GTK_ENTRY(user_data);
    const char *text = gtk_entry_get_text(entry);

    if (text && strlen(text) > 0) {
        char formatted_msg[MSG_BUF_SIZE];
        snprintf(formatted_msg, sizeof(formatted_msg), "@msg %s", text);
        
        int terminal_id = 1;
        handle_command(formatted_msg, terminal_id);
        gtk_entry_set_text(entry, "");
    }
}

static gboolean update_messages(gpointer user_data) {
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(user_data);
    if (!GTK_IS_TEXT_BUFFER(buffer)) {
        fprintf(stderr, "[ERROR] update_messages: Invalid text buffer\n");
        return G_SOURCE_REMOVE;
    }

    int terminal_id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(buffer), "terminal_id"));
    char *msg = model_read_messages(terminal_id);

    if (msg != NULL) {
        char formatted_msg[MSG_BUF_SIZE + 64];
        snprintf(formatted_msg, sizeof(formatted_msg), "[Message] %s\n", msg);
        append_output(buffer, formatted_msg);
        free(msg);
    }
    return G_SOURCE_CONTINUE;
}

void create_new_terminal(int terminal_id, const char *title, gboolean is_parent) {
    GtkWidget *window, *vbox, *scrolled_window, *text_view;
    GtkWidget *command_entry, *message_entry, *message_send_button;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), title);
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), NULL);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    g_object_set_data_full(G_OBJECT(buffer), "terminal_id", GINT_TO_POINTER(terminal_id), NULL);
    g_object_set_data(G_OBJECT(window), "text_buffer", buffer);

    command_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(command_entry), "Enter command (e.g., ls)");
    gtk_box_pack_start(GTK_BOX(vbox), command_entry, FALSE, FALSE, 0);
    g_signal_connect(command_entry, "activate", G_CALLBACK(on_command_entry_activate), NULL);

    message_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(message_entry), "Enter message (@msg ...) - Private Messaging");
    gtk_box_pack_start(GTK_BOX(vbox), message_entry, FALSE, FALSE, 0);

    message_send_button = gtk_button_new_with_label("Send Message");
    gtk_box_pack_start(GTK_BOX(vbox), message_send_button, FALSE, FALSE, 0);
    g_signal_connect(message_send_button, "clicked", G_CALLBACK(on_send_message), message_entry);

    guint timer_id = g_timeout_add(300, update_messages, buffer);
    g_object_set_data(G_OBJECT(window), "timer_id", GUINT_TO_POINTER(timer_id));

    if (is_parent) {
        parent_terminal_window = window;
        global_text_buffer = buffer;
        global_text_view = text_view;
    } else {
        child_terminal_window = window;
    }

    gtk_widget_show_all(window);
}

void on_window_destroy(GtkWidget *window, gpointer user_data) {
    guint timer_id = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(window), "timer_id"));
    if (timer_id > 0) {
        g_source_remove(timer_id);
    }

    if (parent_terminal_window == window) {
        parent_terminal_window = NULL;
        global_text_buffer = NULL;
        global_text_view = NULL;
    } else if (child_terminal_window == window) {
        child_terminal_window = NULL;
    }

    gtk_widget_destroy(window);
}

void on_add_terminal_button_clicked(GtkButton *button, gpointer user_data) {
    int terminal_id = ++terminal_counter;
    char terminal_title[50];

    if (parent_terminal_window == NULL) {
        snprintf(terminal_title, sizeof(terminal_title), "Child Terminal 1");
        create_new_terminal(terminal_id, terminal_title, TRUE);
    } else {
        snprintf(terminal_title, sizeof(terminal_title), "Child Terminal %d", terminal_counter);
        create_new_terminal(terminal_id, terminal_title, FALSE);
    }
}

GtkWidget* create_terminal_window() {
    GtkWidget *window, *vbox, *scrolled_window, *text_view;
    GtkWidget *command_entry, *message_entry, *message_send_button, *add_terminal_button;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Parent Terminal");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);

    global_text_view = text_view;
    global_text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

    command_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(command_entry), "Komut girin (örn: ls)");
    gtk_box_pack_start(GTK_BOX(vbox), command_entry, FALSE, FALSE, 0);
    g_signal_connect(command_entry, "activate", G_CALLBACK(on_command_entry_activate), NULL);

    message_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(message_entry), "Mesaj girin (@msg ...) - Private Messaging");
    gtk_box_pack_start(GTK_BOX(vbox), message_entry, FALSE, FALSE, 0);

    message_send_button = gtk_button_new_with_label("Mesaj Gönder");
    gtk_box_pack_start(GTK_BOX(vbox), message_send_button, FALSE, FALSE, 0);
    g_signal_connect(message_send_button, "clicked", G_CALLBACK(on_send_message), message_entry);

    add_terminal_button = gtk_button_new_with_label("Yeni Shell");
    gtk_box_pack_start(GTK_BOX(vbox), add_terminal_button, FALSE, FALSE, 0);
    g_signal_connect(add_terminal_button, "clicked", G_CALLBACK(on_add_terminal_button_clicked), NULL);

    init_controller();

    return window;
}
*/

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "model.h"
#include "view.h"
#include "controller.h"

#define MAX_COMMAND_LEN 100
#define MSG_BUF_SIZE 1024

// Global widgets
GtkTextBuffer *global_text_buffer = NULL;
GtkWidget *global_text_view = NULL;

//static int terminal_counter = 0;

// Güvenli çıktıyı text buffer'a yazan fonksiyon
static void append_output(GtkTextBuffer *buffer, const char *text) {
    if (!GTK_IS_TEXT_BUFFER(buffer)) {
        fprintf(stderr, "[ERROR] append_output: Invalid buffer!\n");
        return;
    }

    GtkTextIter end;
    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_insert(buffer, &end, text, -1);

    if (GTK_IS_TEXT_VIEW(global_text_view)) {
        GtkTextMark *mark = gtk_text_buffer_create_mark(buffer, "end", &end, FALSE);
        gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(global_text_view), mark, 0.0, FALSE, 0.0, 0.0);
    }
}

void on_command_entry_activate(GtkEntry *entry, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(entry));
    GtkTextBuffer *buffer = g_object_get_data(G_OBJECT(window), "text_buffer");
    
    const char *command = gtk_entry_get_text(entry);
    if (strcmp(command, "exit") == 0) {
        gtk_main_quit();
    } else {
        int terminal_id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(window), "terminal_id"));
        handle_command(command, terminal_id, buffer);
    }
    gtk_entry_set_text(entry, "");
}

void on_send_message(GtkButton *button, gpointer user_data) {
    GtkEntry *entry = GTK_ENTRY(user_data);
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(entry));
    GtkTextBuffer *buffer = g_object_get_data(G_OBJECT(window), "text_buffer");
    int terminal_id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(window), "terminal_id"));
    
    const char *text = gtk_entry_get_text(entry);
    if (text && strlen(text) > 0) {
        char formatted_msg[MSG_BUF_SIZE];
        snprintf(formatted_msg, sizeof(formatted_msg), "@msg %s", text);
        
        handle_command(formatted_msg, terminal_id, buffer);  // Added buffer parameter
        gtk_entry_set_text(entry, "");
    }
}

static gboolean update_messages(gpointer user_data) {
    // Check if the buffer still exists and is valid
    if (user_data == NULL || !GTK_IS_TEXT_BUFFER(user_data)) {
        return G_SOURCE_REMOVE; // Stop the timer if the buffer is gone
    }

    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(user_data);
    int terminal_id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(buffer), "terminal_id"));
    
    char *msg = model_read_messages(terminal_id);
    if (msg != NULL) {
        char formatted_msg[MSG_BUF_SIZE + 64];
        snprintf(formatted_msg, sizeof(formatted_msg), "[Message] %s\n", msg);
        append_output(buffer, formatted_msg);
        free(msg);
    }
    return G_SOURCE_CONTINUE;
}

void create_new_terminal(int terminal_id, const char *title) {
    GtkWidget *window, *vbox, *scrolled_window, *text_view;
    GtkWidget *command_entry, *message_entry, *message_send_button;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), title);
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), NULL);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    
    // Store terminal-specific data
    g_object_set_data(G_OBJECT(window), "text_buffer", buffer);
    g_object_set_data(G_OBJECT(window), "terminal_id", GINT_TO_POINTER(terminal_id));
    g_object_set_data(G_OBJECT(window), "text_view", text_view);

    command_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(command_entry), "Enter command (e.g., ls)");
    gtk_box_pack_start(GTK_BOX(vbox), command_entry, FALSE, FALSE, 0);
    g_signal_connect(command_entry, "activate", G_CALLBACK(on_command_entry_activate), NULL);

    message_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(message_entry), "Enter message (@msg ...) - Private Messaging");
    gtk_box_pack_start(GTK_BOX(vbox), message_entry, FALSE, FALSE, 0);

    message_send_button = gtk_button_new_with_label("Send Message");
    gtk_box_pack_start(GTK_BOX(vbox), message_send_button, FALSE, FALSE, 0);
    g_signal_connect(message_send_button, "clicked", G_CALLBACK(on_send_message), message_entry);

    // Initialize controller with this terminal's buffer
    init_controller(buffer);
    
    // Set up message polling
    guint timer_id = g_timeout_add(300, update_messages, buffer);
g_object_set_data(G_OBJECT(window), "timer_id", GUINT_TO_POINTER(timer_id));

    gtk_widget_show_all(window);
}

void on_window_destroy(GtkWidget *window, gpointer user_data) {
    guint timer_id = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(window), "timer_id"));
    if (timer_id > 0) {
        g_source_remove(timer_id);
    }
    
    // Also clear the buffer reference to prevent dangling pointers
    GtkTextBuffer *buffer = g_object_get_data(G_OBJECT(window), "text_buffer");
    if (buffer) {
        g_object_set_data(G_OBJECT(buffer), "terminal_id", NULL);
    }
}

void on_add_terminal_button_clicked(GtkButton *button, gpointer user_data) {
    static int terminal_counter = 1;  // Make this static to maintain count
    
    char terminal_title[50];
    snprintf(terminal_title, sizeof(terminal_title), "Terminal %d", terminal_counter);
    
    // Create new terminal with unique ID
    create_new_terminal(terminal_counter, terminal_title);
    terminal_counter++;
}

GtkWidget* create_terminal_window() {
    GtkWidget *window, *vbox, *scrolled_window, *text_view;
    GtkWidget *command_entry, *message_entry, *message_send_button, *add_terminal_button;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Terminal");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    
    // Store terminal data (ID 1 for first terminal)
    g_object_set_data(G_OBJECT(window), "text_buffer", buffer);
    g_object_set_data(G_OBJECT(window), "terminal_id", GINT_TO_POINTER(1));
    g_object_set_data(G_OBJECT(window), "text_view", text_view);

    command_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(command_entry), "Enter command (e.g., ls)");
    gtk_box_pack_start(GTK_BOX(vbox), command_entry, FALSE, FALSE, 0);
    g_signal_connect(command_entry, "activate", G_CALLBACK(on_command_entry_activate), NULL);

    message_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(message_entry), "Enter message (@msg ...) - Private Messaging");
    gtk_box_pack_start(GTK_BOX(vbox), message_entry, FALSE, FALSE, 0);

    message_send_button = gtk_button_new_with_label("Send Message");
    gtk_box_pack_start(GTK_BOX(vbox), message_send_button, FALSE, FALSE, 0);
    g_signal_connect(message_send_button, "clicked", G_CALLBACK(on_send_message), message_entry);

    add_terminal_button = gtk_button_new_with_label("New Terminal");
    gtk_box_pack_start(GTK_BOX(vbox), add_terminal_button, FALSE, FALSE, 0);
    g_signal_connect(add_terminal_button, "clicked", G_CALLBACK(on_add_terminal_button_clicked), NULL);

    // Initialize controller
    init_controller(buffer);

    return window;
}







