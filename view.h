/*#ifndef VIEW_H
#define VIEW_H

#include <gtk/gtk.h>
#include "model.h"

// Global text buffer, model.c içindeki main fonksiyonundan kullanılacak.
extern GtkTextBuffer *global_text_buffer;

// Komut girişini işleyen callback fonksiyonu
void on_command_entry_activate(GtkEntry *entry, gpointer user_data);

// Mesaj gönderme işlemi (girişten shared memory'e yaz)
void on_send_message(GtkButton *button, gpointer user_data);

// Mesaj alma işlemi (shared memory'den okuyup ekrana yaz)
void on_receive_message(GtkButton *button, gpointer user_data);

void on_window_destroy(GtkWidget *window, gpointer user_data);

// GUI'yi oluşturan fonksiyon
GtkWidget* create_terminal_window();

#endif // VIEW_H
*/

#ifndef VIEW_H
#define VIEW_H

#include <gtk/gtk.h>
#include "model.h"

// Global text buffer, model.c içindeki main fonksiyonundan kullanılacak.
extern GtkTextBuffer *global_text_buffer;

// Komut girişini işleyen callback fonksiyonu
void on_command_entry_activate(GtkEntry *entry, gpointer user_data);

// Mesaj gönderme işlemi (girişten shared memory'e yaz)
void on_send_message(GtkButton *button, gpointer user_data);

// Mesaj alma işlemi (shared memory'den okuyup ekrana yaz)
void on_receive_message(GtkButton *button, gpointer user_data);

void on_window_destroy(GtkWidget *window, gpointer user_data);

// GUI'yi oluşturan fonksiyon
GtkWidget* create_terminal_window();

#endif // VIEW_H

