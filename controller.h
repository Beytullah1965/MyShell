#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <gtk/gtk.h>
#include "model.h"

extern ShmBuf *shm;

// Controller başlatma
void init_controller(GtkTextBuffer *buffer, int terminal_id);

// Komut işleme
void handle_command(const char *input, int sender_id, GtkTextBuffer *text_buffer);

#endif
