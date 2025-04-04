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












