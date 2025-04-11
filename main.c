/*#include <gtk/gtk.h>
#include <sys/mman.h>
#include <semaphore.h>
#include "model.h"
#include "controller.h"
#include "view.h"

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);  // Parametresiz olarak çağrılır

    static int terminal_counter = 0;
    int terminal_id = ++terminal_counter;

    // Terminal penceresini oluştur
    GtkWidget *window = create_terminal_window(terminal_id);
    gtk_widget_show_all(window);  // window'u göster

    // Ana GTK olay döngüsünü başlat
    gtk_main();
    
    return 0;

}*/

#include <gtk/gtk.h>
#include <sys/mman.h>
#include <semaphore.h>
#include "model.h"
#include "controller.h"
#include "view.h"

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);  // Parametresiz olarak çağrılır

    // Terminal penceresini oluştur
    GtkWidget *window = create_terminal_window();
    gtk_widget_show_all(window);  // window'u göster

    buf_init();  // Paylaşılan bellek başlatılır

    // Ana GTK olay döngüsünü başlat
    gtk_main();

    // Program sonlandığında paylaşılan belleği temizle
    shm_unlink("mymsgbuf");

    return 0;
}