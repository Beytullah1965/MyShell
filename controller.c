#include "controller.h"
#include "model.h"
#include "view.h"
#include <string.h>
#include <gtk/gtk.h>
#include <stdbool.h>

// Mesaj kontrol aralığı (ms)
#define MESSAGE_POLL_INTERVAL 300

// Paylaşılan bellek işaretçisi
ShmBuf *shm = NULL;

// Timer callback fonksiyonu
static gboolean poll_messages(gpointer user_data) {
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(user_data);
    
    // Paylaşılan bellekten mesajları oku
    sem_wait(&shm->sem);
    if (shm->cnt > 0) {
        GtkTextIter end;
        gtk_text_buffer_get_end_iter(buffer, &end);
        
        // Mesajı formatlı şekilde ekle
        gtk_text_buffer_insert(buffer, &end, "[MSG] ", -1);
        gtk_text_buffer_insert(buffer, &end, shm->msgbuf, -1);
        gtk_text_buffer_insert(buffer, &end, "\n", -1);
        
        shm->cnt = 0; // Mesajı işlendi olarak işaretle
    }
    sem_post(&shm->sem);
    
    return G_SOURCE_CONTINUE;
}

void init_controller() {
    // Paylaşılan belleği başlat
    shm = buf_init();
    
    // Mesaj polling timer'ını başlat
    g_timeout_add(MESSAGE_POLL_INTERVAL, poll_messages, global_text_buffer);
}

void handle_command(const char *input) {
    if (input == NULL || strlen(input) == 0) return;

    // Komut tipini ayır
    if (strncmp(input, "@msg ", 5) == 0) {
        // Mesaj gönder
        const char *message = input + 5;
        if (strlen(message) > 0) {
            sem_wait(&shm->sem);
            strncpy(shm->msgbuf, message, MSG_BUF_SIZE);
            shm->cnt = strlen(message);
            sem_post(&shm->sem);
        }
    } else {
        // Shell komutu çalıştır
        execute_command(input, shm, global_text_buffer); 
    }
}
