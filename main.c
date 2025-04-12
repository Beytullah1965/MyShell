/*#include <gtk/gtk.h>
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