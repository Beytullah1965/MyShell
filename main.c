#include <gtk/gtk.h>
#include <sys/mman.h>
#include <semaphore.h>
#include "model.h"
#include "controller.h"
#include "view.h"

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);  // GTK başlat

    // Paylaşılan bellek başlat
    buf_init();

    // İlk terminal penceresini oluştur
    GtkWidget *window = create_terminal_window();
    gtk_widget_show_all(window);

    // GTK olay döngüsünü başlat
    gtk_main();

    // Uygulama sonlandığında paylaşılan belleği temizle
    shm_unlink("mymsgbuf");

    return 0;
}
