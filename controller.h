/*#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <gtk/gtk.h>

// Controller başlatma
void init_controller(int termianl_id);

// Komut işleme
void handle_command(const char *input);

#endif
*/

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <gtk/gtk.h>

// Controller başlatma
void init_controller();

// Komut işleme
void handle_command(const char *input);

#endif


