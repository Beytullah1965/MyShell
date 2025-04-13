<<<<<<< HEAD
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "model.h"  // model.h dahil ediliyor

#define MAX_COMMAND_LEN 100

// Global text buffer; model.c içindeki main fonksiyonundan kullanılacak.
GtkTextBuffer *global_text_buffer = NULL;

// Komut girişini işleyen callback fonksiyonu
void on_command_entry_activate(GtkEntry *entry, gpointer user_data) {
    const char *command = gtk_entry_get_text(entry);
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(user_data);
    char output[512];

    if (strcmp(command, "exit") == 0) {
        gtk_main_quit();
    } else {
        ProcessInfo pinfo;
        ShmBuf *shmp = buf_init();
        int history_count = 0;


        // Girilen komutun çıktısını ilgili text buffer'a ekliyoruz
        execute_command(command, shmp, buffer);

        // Komut uzunluğunu sınırlıyoruz ve çıktıyı snprintf ile yazdırıyoruz
        snprintf(output, sizeof(output), "Çalıştırılan komut: PID=%d, CMD=%.*s, Durum: %d\n", 
                 pinfo.pid, MAX_COMMAND_LEN, pinfo.command, pinfo.status);
        gtk_text_buffer_insert_at_cursor(buffer, output, -1);
    }
    gtk_entry_set_text(entry, "");
}


// Mesaj gönderme işlemi (girişten shared memory'e yaz)
void on_send_message(GtkButton *button, gpointer user_data) {
    GtkEntry *entry = GTK_ENTRY(user_data);
    const char *text = gtk_entry_get_text(entry);

    if (text && strlen(text) > 0) {
        ShmBuf *shmp = buf_init();
        send_message(shmp, text);
        gtk_entry_set_text(entry, ""); // Gönderdikten sonra temizle
    }
}

// Mesaj alma işlemi (shared memory'den okuyup ekrana yaz)
void on_receive_message(GtkButton *button, gpointer user_data) {
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(user_data);
    ShmBuf *shmp = buf_init();

    char *msg = shmp->msgbuf;

    if (!g_utf8_validate(msg, -1, NULL)) {
        char clean_msg[MSG_BUF_SIZE];
        int j = 0;
        for (int i = 0; msg[i] != '\0' && j < MSG_BUF_SIZE - 1; i++) {
            if ((unsigned char)msg[i] >= 32 && (unsigned char)msg[i] <= 126) {
                clean_msg[j++] = msg[i];
            }
        }
        clean_msg[j] = '\0';
        gtk_text_buffer_insert_at_cursor(buffer, "Mesaj (temizlenmiş): ", -1);
        gtk_text_buffer_insert_at_cursor(buffer, clean_msg, -1);
        gtk_text_buffer_insert_at_cursor(buffer, "\n", -1);
    } else {
        gtk_text_buffer_insert_at_cursor(buffer, "Mesaj: ", -1);
        gtk_text_buffer_insert_at_cursor(buffer, msg, -1);
        gtk_text_buffer_insert_at_cursor(buffer, "\n", -1);
    }
}

// GUI'yi oluşturan fonksiyon
GtkWidget* create_terminal_window() {
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *scrolled_window;
    GtkWidget *text_view;
    GtkWidget *command_entry;
    GtkWidget *message_entry;
    GtkWidget *message_send_button;
    GtkWidget *message_receive_button;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Terminal Emulator");
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

    global_text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

    command_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(command_entry), "Komut girin (örn: ls)");
    gtk_box_pack_start(GTK_BOX(vbox), command_entry, FALSE, FALSE, 0);
    g_signal_connect(command_entry, "activate", G_CALLBACK(on_command_entry_activate), global_text_buffer);

    message_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(message_entry), "Gönderilecek mesaj");
    gtk_box_pack_start(GTK_BOX(vbox), message_entry, FALSE, FALSE, 0);

    message_send_button = gtk_button_new_with_label("Mesaj Gönder");
    gtk_box_pack_start(GTK_BOX(vbox), message_send_button, FALSE, FALSE, 0);
    g_signal_connect(message_send_button, "clicked", G_CALLBACK(on_send_message), message_entry);

    message_receive_button = gtk_button_new_with_label("Mesaj Al");
    gtk_box_pack_start(GTK_BOX(vbox), message_receive_button, FALSE, FALSE, 0);
    g_signal_connect(message_receive_button, "clicked", G_CALLBACK(on_receive_message), global_text_buffer);

    return window;
}












=======
#include "controller.h"
#include "model.h"
#include "view.h"
#include <string.h>
#include <gtk/gtk.h>
#include <stdbool.h>
#include <gdk-pixbuf/gdk-pixbuf.h> 
#define MESSAGE_POLL_INTERVAL 300
#define PS_PROMPT "PS C:\\Users\\> "  // Yeni prompt formatı

typedef struct {
    GtkTextBuffer *buffer;
    int terminal_id;
} TerminalData;

// Timer callback function to poll messages
static gboolean poll_messages(gpointer user_data) {
    TerminalData *tdata = (TerminalData *)user_data;
    GtkTextBuffer *buffer = tdata->buffer;
    int terminal_id = tdata->terminal_id;

    // Read messages from shared memory
    sem_wait(&shm->sem);
    if (shm->cnt > 0 && (shm->receiver_id == -1 || shm->receiver_id == terminal_id)) {
        GtkTextIter end;
        gtk_text_buffer_get_end_iter(buffer, &end);

        // Apply green color for messages
        GtkTextTag *tag = gtk_text_buffer_create_tag(buffer, "green_foreground",
                                                   "foreground", "#00FF00", NULL);
        gtk_text_buffer_insert_with_tags(buffer, &end, "[MSG] ", -1, tag, NULL);
        gtk_text_buffer_insert(buffer, &end, shm->msgbuf, -1);
        gtk_text_buffer_insert(buffer, &end, "\n", -1);

        shm->cnt = 0; // Message processed
        shm->receiver_id = -2; // Mark as processed
    }
    sem_post(&shm->sem);

    return G_SOURCE_CONTINUE;
}

void apply_common_css(GtkWidget *widget) {
    GtkCssProvider *css_provider = gtk_css_provider_new();
    const gchar *css_data =
        "textview { "
        "   background-color: #000000; "  // Siyah arka plan (sadece terminal çıktısı için)
        "   color:rgb(0, 0, 0); "            // Beyaz metin (sadece terminal çıktısı için)
        "   border: none; "
        "   font-family: Consolas, monospace; "
        "} "
        
        // Diğer widget'lar için orijinal stiller (değişmeden kalacak)
        "window, .terminal { "
        "   background-color:rgb(1, 36, 86); "  // Orijinal mavi arka plan
        "   color:rgb(255, 0, 0); "            // Orijinal kırmızı metin
        "   font-family: Consolas, monospace; "
        "   font-size: 12pt; "
        "} "
        "entry { "
        "   background-color: #012456; "
        "   color:rgb(255, 0, 0); "
        "   border: 1px solid #1E90FF; "
        "   border-radius: 0px; "
        "   padding: 5px; "
        "   font-family: Consolas, monospace; "
        "} "
        "button { "
        "   background-color:rgb(250, 0, 0); "
        "   color:rgb(250, 0, 0); "
        "   border: 1px solidrgb(255, 0, 0); "
        "   border-radius: 0px; "
        "   padding: 5px 10px; "
        "   font-family: Consolas, monospace; "
        "   margin-top: 5px; "
        "} "
        "button:hover { "
        "   background-color:rgb(255, 30, 30); "
        "   color:rgb(255, 30, 30); "
        "} "
        "scrollbar { "
        "   background-color: #011C3D; "
        "} "
        "scrollbar slider { "
        "   background-color: #1E90FF; "
        "} "
        ".ps-prompt { "
        "   color:rgb(255, 255, 255); "
        "   background-color: #012456; "
        "   border: 1px solid #1E90FF; "
        "   border-radius: 0px; "
        "}";

    gtk_css_provider_load_from_data(css_provider, css_data, -1, NULL);

    GtkStyleContext *context = gtk_widget_get_style_context(widget);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(css_provider), 
                                 GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_style_context_add_class(context, "terminal");

    if (GTK_IS_ENTRY(widget)) {
        const gchar *placeholder = gtk_entry_get_placeholder_text(GTK_ENTRY(widget));
        if (placeholder && strstr(placeholder, PS_PROMPT) != NULL) {
            gtk_style_context_add_class(context, "ps-prompt");
        }
    }

    g_object_unref(css_provider);
}

// Create a new terminal window and handle UI components
void create_new_terminal(int terminal_id, const char *title) {
    GtkWidget *window, *vbox, *scrolled_window, *text_view;
    GtkWidget *command_entry, *message_entry, *message_send_button;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), title);
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), NULL);
    set_window_icon(window);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(text_view), TRUE);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    
    // Set default text color to white
    GtkTextTagTable *tag_table = gtk_text_buffer_get_tag_table(buffer);
    GtkTextTag *default_tag = gtk_text_tag_new("default_white");
    g_object_set(default_tag, "foreground", "#FFFFFF", NULL);
    gtk_text_tag_table_add(tag_table, default_tag);

    // Store terminal-specific data
    TerminalData *tdata = g_new(TerminalData, 1);
    tdata->buffer = buffer;
    tdata->terminal_id = terminal_id;
    g_object_set_data_full(G_OBJECT(window), "terminal_data", tdata, g_free);

    g_object_set_data(G_OBJECT(window), "text_buffer", buffer);
    g_object_set_data(G_OBJECT(window), "terminal_id", GINT_TO_POINTER(terminal_id));
    g_object_set_data(G_OBJECT(window), "text_view", text_view);

    // PowerShell-like command prompt
    command_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(command_entry), PS_PROMPT);
    gtk_box_pack_start(GTK_BOX(vbox), command_entry, FALSE, FALSE, 0);
    g_signal_connect(command_entry, "activate", G_CALLBACK(on_command_entry_activate), NULL);

    // Message entry and send button
    message_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(message_entry), 
                                 "Enter message (@msg ...) - Private Messaging");
    gtk_box_pack_start(GTK_BOX(vbox), message_entry, FALSE, FALSE, 0);

    message_send_button = gtk_button_new_with_label("Send Message");
    gtk_box_pack_start(GTK_BOX(vbox), message_send_button, FALSE, FALSE, 0);
    g_signal_connect(message_send_button, "clicked", G_CALLBACK(on_send_message), message_entry);

    // Initialize controller with this terminal's buffer
    init_controller(buffer, terminal_id);

    // Set up message polling
    guint timer_id = g_timeout_add(MESSAGE_POLL_INTERVAL, poll_messages, tdata);
    g_object_set_data(G_OBJECT(window), "timer_id", GUINT_TO_POINTER(timer_id));

    // Apply common CSS styles to all widgets
    apply_common_css(window);
    apply_common_css(command_entry);
    apply_common_css(text_view);
    apply_common_css(message_entry);
    apply_common_css(message_send_button);
    apply_common_css(scrolled_window);

    gtk_widget_show_all(window);
}


// Handle window destruction (cleanup)
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

// Add a new terminal on button click
void on_add_terminal_button_clicked(GtkButton *button, gpointer user_data) {
    static int terminal_counter = 1;  // Make this static to maintain count

    char terminal_title[50];
    snprintf(terminal_title, sizeof(terminal_title), "Terminal %d", terminal_counter);

    // Create new terminal with unique ID
    create_new_terminal(terminal_counter, terminal_title);
    terminal_counter++;
}
void set_window_icon(GtkWidget *window) {
    const gchar *icon_path = "/mnt/c/Users/beytu/Downloads/shellicon.png";
    
    // PNG dosyasını yükle
    GError *error = NULL;
    GdkPixbuf *icon = gdk_pixbuf_new_from_file(icon_path, &error);
    
    if (error != NULL) {
        g_printerr("Icon yüklenemedi: %s\n", error->message);
        g_error_free(error);
        return;
    }
    
    if (icon != NULL) {
        gtk_window_set_icon(GTK_WINDOW(window), icon);
        g_object_unref(icon); // Belleği temizle
    }
}




// Create the initial terminal window
GtkWidget* create_terminal_window() {
    GtkWidget *window, *vbox, *scrolled_window, *text_view;
    GtkWidget *command_entry, *message_entry, *message_send_button, *add_terminal_button;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Terminal");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    set_window_icon(window);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(text_view), TRUE);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    
    // Set default text color to white
    GtkTextTagTable *tag_table = gtk_text_buffer_get_tag_table(buffer);
    GtkTextTag *default_tag = gtk_text_tag_new("default_white");
    g_object_set(default_tag, "foreground", "#FFFFFF", NULL);
    gtk_text_tag_table_add(tag_table, default_tag);

    // Store terminal data (ID 0 for the main terminal)
    TerminalData *tdata = g_new(TerminalData, 1);
    tdata->buffer = buffer;
    tdata->terminal_id = 0;  // Main terminal ID is set to 0
    g_object_set_data_full(G_OBJECT(window), "terminal_data", tdata, g_free);

    g_object_set_data(G_OBJECT(window), "text_buffer", buffer);
    g_object_set_data(G_OBJECT(window), "terminal_id", GINT_TO_POINTER(0));  // ID 0 for the main terminal
    g_object_set_data(G_OBJECT(window), "text_view", text_view);

    // PowerShell-like prompt
    command_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(command_entry), PS_PROMPT);
    gtk_box_pack_start(GTK_BOX(vbox), command_entry, FALSE, FALSE, 0);
    g_signal_connect(command_entry, "activate", G_CALLBACK(on_command_entry_activate), NULL);

    // Message entry and send button
    message_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(message_entry), 
                                 "Enter message (@msg ...) - Private Messaging");
    gtk_box_pack_start(GTK_BOX(vbox), message_entry, FALSE, FALSE, 0);

    message_send_button = gtk_button_new_with_label("Send Message");
    gtk_box_pack_start(GTK_BOX(vbox), message_send_button, FALSE, FALSE, 0);
    g_signal_connect(message_send_button, "clicked", G_CALLBACK(on_send_message), message_entry);

    // Add terminal button (for creating new terminals)
    add_terminal_button = gtk_button_new_with_label("Add Terminal");
    gtk_box_pack_start(GTK_BOX(vbox), add_terminal_button, FALSE, FALSE, 0);
    g_signal_connect(add_terminal_button, "clicked", G_CALLBACK(on_add_terminal_button_clicked), NULL);

    // Apply common CSS for all widgets
    apply_common_css(window);
    apply_common_css(command_entry);
    apply_common_css(text_view);
    apply_common_css(message_entry);
    apply_common_css(message_send_button);
    apply_common_css(add_terminal_button);
    apply_common_css(scrolled_window);

    // Initialize controller with this terminal's buffer
    init_controller(buffer, 0);  // ID 0 for the main terminal

    // Set up message polling
    guint timer_id = g_timeout_add(MESSAGE_POLL_INTERVAL, poll_messages, tdata);
    g_object_set_data(G_OBJECT(window), "timer_id", GUINT_TO_POINTER(timer_id));

    gtk_widget_show_all(window);

    return window;
}


void on_command_entry_activate(GtkEntry *entry, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(entry));
    GtkTextBuffer *buffer = g_object_get_data(G_OBJECT(window), "text_buffer");
    int terminal_id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(window), "terminal_id"));

    const gchar *input = gtk_entry_get_text(entry);
    
    // Insert the command into the text view with prompt
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_insert(buffer, &end, PS_PROMPT, -1);
    gtk_text_buffer_insert(buffer, &end, input, -1);
    gtk_text_buffer_insert(buffer, &end, "\n", -1);
    
    handle_command(input, terminal_id, buffer);

    gtk_entry_set_text(entry, "");
}

void on_send_message(GtkButton *button, gpointer user_data) {
    GtkWidget *entry = GTK_WIDGET(user_data);
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(entry));
    GtkTextBuffer *buffer = g_object_get_data(G_OBJECT(window), "text_buffer");
    int terminal_id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(window), "terminal_id"));

    const gchar *input = gtk_entry_get_text(GTK_ENTRY(entry));
    handle_command(input, terminal_id, buffer);

    gtk_entry_set_text(GTK_ENTRY(entry), "");
}



>>>>>>> master
