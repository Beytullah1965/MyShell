/*#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "model.h"
#include "controller.h"
#include "view.h"

#define MAX_COMMAND_LEN 100

// Global değişken tanımları
GtkTextBuffer *global_text_buffer = NULL;
GtkWidget *global_text_view = NULL;
ShmBuf *shm = NULL;
int terminal_id = 0;

// Komut çıktısını text buffer'a yazdıran yardımcı fonksiyon
void append_output(GtkTextBuffer *buffer, const char *text) {
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_insert(buffer, &end, text, -1);
    
    // Otomatik scroll
    if (global_text_view) {
        gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(global_text_view),
                                gtk_text_buffer_get_insert(buffer),
                                0.0, FALSE, 0.0, 0.0);
    }
}

// Komut girişi callback fonksiyonu
void on_command_entry_activate(GtkEntry *entry, gpointer user_data) {
    const char *command = gtk_entry_get_text(entry);
    
    if (strcmp(command, "exit") == 0) {
        gtk_main_quit();
    } else {
        handle_command(command);
    }
    gtk_entry_set_text(entry, "");
}

// Mesaj gönderme işlemi
void on_send_message(GtkButton *button, gpointer user_data) {
    GtkEntry *entry = GTK_ENTRY(user_data);
    const char *text = gtk_entry_get_text(entry);
    
    if (text && strlen(text) > 0) {
        char formatted_msg[MSG_BUF_SIZE];
        snprintf(formatted_msg, sizeof(formatted_msg), "@msg %s", text);
        handle_command(formatted_msg);
        gtk_entry_set_text(entry, "");
    }
}

// Mesajları otomatik güncelleyen timer fonksiyonu
static gboolean update_messages(gpointer user_data) {
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(user_data);
    
    if (shm == NULL) {
        g_print("HATA: shm NULL!\n");
        return G_SOURCE_CONTINUE;
    }
    
    char *msg = receive_message(shm, terminal_id);
    if (msg) {
        GtkTextIter end;
        gtk_text_buffer_get_end_iter(buffer, &end);
        
        char formatted_msg[MSG_BUF_SIZE + 50];
        snprintf(formatted_msg, sizeof(formatted_msg), 
                "[Terminal%d] %s\n", get_sender_id(shm), msg);
        
        // Doğrudan ekleme yapın
        gtk_text_buffer_insert(buffer, &end, formatted_msg, -1);
        
        // Otomatik scroll
        if (global_text_view) {
            gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(global_text_view),
            gtk_text_buffer_get_insert(buffer),0.0, FALSE, 0.0, 0.0);
        }
        
        free(msg);
    }
    return G_SOURCE_CONTINUE;
}

void on_window_destroy(GtkWidget *widget, gpointer data) {
    // Paylaşımlı bellek ve semafor temizliği burada da yapılabilir
    gtk_main_quit();  // GTK döngüsünü sonlandır
}

// GUI'yi oluşturan fonksiyon
GtkWidget* create_terminal_window(int id) {
    terminal_id = id;  // Terminal ID'sini kaydet
    shm = buf_init(terminal_id);  // Paylaşılan belleği başlat
    
    GtkWidget *window, *vbox, *scrolled_window, *text_view;
    GtkWidget *command_entry, *message_entry, *message_send_button;

    // Ana pencere oluşturma
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Terminal Emulator");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), NULL);

    // Düzen bileşenleri
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Komut çıktı alanı
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);

    global_text_view = text_view;
    global_text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

    // Komut girişi
    command_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(command_entry), "Komut girin (örn: ls)");
    gtk_box_pack_start(GTK_BOX(vbox), command_entry, FALSE, FALSE, 0);
    g_signal_connect(command_entry, "activate", G_CALLBACK(on_command_entry_activate), NULL);

    // Mesajlaşma arayüzü
    message_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(message_entry), "Mesaj girin (@msg ...)");
    gtk_box_pack_start(GTK_BOX(vbox), message_entry, FALSE, FALSE, 0);

    message_send_button = gtk_button_new_with_label("Mesaj Gönder");
    gtk_box_pack_start(GTK_BOX(vbox), message_send_button, FALSE, FALSE, 0);
    g_signal_connect(message_send_button, "clicked", G_CALLBACK(on_send_message), message_entry);

    // Controller'ı başlat
    init_controller(terminal_id);
    
    // Mesaj güncelleme timer'ını başlat
    g_timeout_add(300, update_messages, global_text_buffer);

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
#define MSG_BUF_SIZE 1024  // Mesaj boyutunu tanımlıyoruz

// Global widgets
GtkTextBuffer *global_text_buffer = NULL;
GtkWidget *global_text_view = NULL;

// Terminal ID sayacı ve parent terminal
static int terminal_counter = 0;
static GtkWidget *parent_terminal_window = NULL;
static GtkWidget *child_terminal_window = NULL; // Child terminal pointer

// Komut çıktısını text buffer'a yazdıran yardımcı fonksiyon
static void append_output(GtkTextBuffer *buffer, const char *text) {
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_insert(buffer, &end, text, -1);
    
    // Otomatik scroll
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(global_text_view),
                            gtk_text_buffer_create_mark(buffer, "end", &end, FALSE),
                            0.0, FALSE, 0.0, 0.0);
}

// Komut girişi callback fonksiyonu
void on_command_entry_activate(GtkEntry *entry, gpointer user_data) {
    const char *command = gtk_entry_get_text(entry);
    
    if (strcmp(command, "exit") == 0) {
        gtk_main_quit();
    } else {
        handle_command(command);
    }
    gtk_entry_set_text(entry, "");
}

// Mesaj gönderme işlemi
void on_send_message(GtkButton *button, gpointer user_data) {
    GtkEntry *entry = GTK_ENTRY(user_data);
    const char *text = gtk_entry_get_text(entry);
    
    if (text && strlen(text) > 0) {
        char formatted_msg[MSG_BUF_SIZE];
        snprintf(formatted_msg, sizeof(formatted_msg), "@msg %s", text);
        handle_command(formatted_msg);
        gtk_entry_set_text(entry, "");
    }
}

// Mesajları otomatik güncelleyen timer fonksiyonu
static gboolean update_messages(gpointer user_data) {
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(user_data);
    
    // First validate the buffer
    if (!GTK_IS_TEXT_BUFFER(buffer)) {
        return G_SOURCE_REMOVE; // Stop the timer if buffer is invalid
    }
    
    int terminal_id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(buffer), "terminal_id"));
    char *msg = model_read_messages(terminal_id);
    
    if (msg != NULL) {
        char formatted_msg[MSG_BUF_SIZE + 20];
        snprintf(formatted_msg, sizeof(formatted_msg), "[Message] %s\n", msg);
        append_output(buffer, formatted_msg);
        free(msg);
    }
    return G_SOURCE_CONTINUE;
}

// Yeni terminali oluşturma fonksiyonu
void create_new_terminal(int terminal_id, const char *title, gboolean is_parent) {
    GtkWidget *window, *vbox, *scrolled_window, *text_view;
    GtkWidget *command_entry, *message_entry, *message_send_button;

    // Create new terminal window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), title);
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
    
    // Connect destroy signal with proper cleanup
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), NULL);

    // Layout components
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Command output area
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);

    // Store terminal ID in the buffer
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    g_object_set_data_full(G_OBJECT(buffer), 
                         "terminal_id", 
                         GINT_TO_POINTER(terminal_id),
                         NULL);
    
    // Store buffer reference in the window
    g_object_set_data(G_OBJECT(window), "text_buffer", buffer);

    // Command entry
    command_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(command_entry), "Enter command (e.g., ls)");
    gtk_box_pack_start(GTK_BOX(vbox), command_entry, FALSE, FALSE, 0);
    g_signal_connect(command_entry, "activate", G_CALLBACK(on_command_entry_activate), NULL);

    // Messaging interface
    message_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(message_entry), 
                                 "Enter message (@msg ...) - Private Messaging");
    gtk_box_pack_start(GTK_BOX(vbox), message_entry, FALSE, FALSE, 0);

    message_send_button = gtk_button_new_with_label("Send Message");
    gtk_box_pack_start(GTK_BOX(vbox), message_send_button, FALSE, FALSE, 0);
    g_signal_connect(message_send_button, "clicked", G_CALLBACK(on_send_message), message_entry);

    // Start message updates with the buffer
    guint timer_id = g_timeout_add(300, update_messages, buffer);
    
    // Store timer ID in the window for cleanup
    g_object_set_data(G_OBJECT(window), "timer_id", GUINT_TO_POINTER(timer_id));

    // Set as parent or child terminal
    if (is_parent) {
        parent_terminal_window = window;
        // Store the buffer globally if needed
        global_text_buffer = buffer;
        global_text_view = text_view;
    } else {
        child_terminal_window = window;
    }

    // Show the window
    gtk_widget_show_all(window);
}

// Add this cleanup function
static void on_window_destroy(GtkWidget *window, gpointer user_data) {
    // Remove the timer
    guint timer_id = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(window), "timer_id"));
    if (timer_id > 0) {
        g_source_remove(timer_id);
    }
    
    // Clear references
    if (parent_terminal_window == window) {
        parent_terminal_window = NULL;
        global_text_buffer = NULL;
        global_text_view = NULL;
    } else if (child_terminal_window == window) {
        child_terminal_window = NULL;
    }
    
    gtk_widget_destroy(window);
}

// Yeni terminal açma butonunu işleyen callback fonksiyonu
void on_add_terminal_button_clicked(GtkButton *button, gpointer user_data) {
    // Yeni terminal ID'si
    int terminal_id = ++terminal_counter;
    char terminal_title[50];

    // İlk terminali parent olarak oluştur
    if (parent_terminal_window == NULL) {
        snprintf(terminal_title, sizeof(terminal_title), "Child Terminal 1");
        create_new_terminal(terminal_id, terminal_title, TRUE);
    } else {
        snprintf(terminal_title, sizeof(terminal_title), "Child Terminal %d", terminal_counter);
        create_new_terminal(terminal_id, terminal_title, FALSE);
    }
}

// GUI'yi oluşturan ana fonksiyon
GtkWidget* create_terminal_window() {
    GtkWidget *window, *vbox, *scrolled_window, *text_view;
    GtkWidget *command_entry, *message_entry, *message_send_button, *add_terminal_button;

    // Ana pencere oluşturma
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Parent Terminal");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Düzen bileşenleri
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Komut çıktı alanı
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);

    global_text_view = text_view;
    global_text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

    // Komut girişi
    command_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(command_entry), "Komut girin (örn: ls)");
    gtk_box_pack_start(GTK_BOX(vbox), command_entry, FALSE, FALSE, 0);
    g_signal_connect(command_entry, "activate", G_CALLBACK(on_command_entry_activate), NULL);

    // Mesajlaşma arayüzü
    message_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(message_entry), "Mesaj girin (@msg ...) - Private Messaging");
    gtk_box_pack_start(GTK_BOX(vbox), message_entry, FALSE, FALSE, 0);

    message_send_button = gtk_button_new_with_label("Mesaj Gönder");
    gtk_box_pack_start(GTK_BOX(vbox), message_send_button, FALSE, FALSE, 0);
    g_signal_connect(message_send_button, "clicked", G_CALLBACK(on_send_message), message_entry);

    // Yeni terminal açma butonunu oluştur
    add_terminal_button = gtk_button_new_with_label("Yeni Shell");
    gtk_box_pack_start(GTK_BOX(vbox), add_terminal_button, FALSE, FALSE, 0);
    g_signal_connect(add_terminal_button, "clicked", G_CALLBACK(on_add_terminal_button_clicked), NULL);

    // Controller'ı başlat
    init_controller();

    return window;
}






