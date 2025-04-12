/*#include "controller.h"
#include "model.h"
#include "view.h"
#include <string.h>
#include <gtk/gtk.h>
#include <stdbool.h>

// Mesaj kontrol aralığı (ms)
#define MESSAGE_POLL_INTERVAL 300

// Global shm pointer'ı model.c içindekiyle ortak kullanılmalı
extern ShmBuf *shm;

// View'deki global text buffer (her terminale özel olabilir)
extern GtkTextBuffer *global_text_buffer;

// Timer ile mesajları kontrol et
static gboolean poll_messages(gpointer user_data) {
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(user_data);
    
    if (!GTK_IS_TEXT_BUFFER(buffer)) {
        g_warning("Geçersiz GtkTextBuffer referansı");
        return G_SOURCE_CONTINUE;
    }

    char *msg = model_read_messages(shm->receiver_id);  // terminal_id ile karşılaştırma burada yapılır
    if (msg && strlen(msg) > 0) {
        GtkTextIter end;
        gtk_text_buffer_get_end_iter(buffer, &end);
        gtk_text_buffer_insert(buffer, &end, "[MSG] ", -1);
        gtk_text_buffer_insert(buffer, &end, msg, -1);
        gtk_text_buffer_insert(buffer, &end, "\n", -1);

        // Mesaj işlendi olarak işaretle
        receive_message(shm, shm->receiver_id);
    }
    return G_SOURCE_CONTINUE;
}

// Başlatıcı fonksiyon
void init_controller(int terminal_id) {
    // Shared memory başlat (ilk terminal başlatıyorsa oluşturur)
    shm = buf_init();

    // Her terminal için kendi receiver_id'sini ayarla
    shm->receiver_id = terminal_id;

    // View tarafındaki text buffer’a erişim
    if (global_text_buffer && GTK_IS_TEXT_BUFFER(global_text_buffer)) {
        g_timeout_add(MESSAGE_POLL_INTERVAL, poll_messages, global_text_buffer);
    } else {
        g_warning("global_text_buffer geçersiz.");
    }
}

// Kullanıcıdan gelen input'u işle
void handle_command(const char *input, int sender_id) {
    if (input == NULL || strlen(input) == 0) return;

    if (strncmp(input, "@msg ", 5) == 0) {
        const char *message = input + 5;
        if (strlen(message) > 0) {
            // Broadcast (-1), ya da tek kullanıcıya mesaj gönderilebilir
            send_message(shm, message, sender_id, -1);
        }
    } else {
        // Normal shell komutu
        if (global_text_buffer && GTK_IS_TEXT_BUFFER(global_text_buffer)) {
            execute_command(input, shm, global_text_buffer);
        } else {
            g_warning("Komut için geçersiz text buffer.");
        }
    }
}
*/

#include "controller.h"
#include "model.h"
#include "view.h"
#include <string.h>
#include <gtk/gtk.h>
#include <stdbool.h>

#define MESSAGE_POLL_INTERVAL 300

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

void init_controller(GtkTextBuffer *buffer) {
    // Paylaşılan belleği başlat
    shm = buf_init();
    
    // Mesaj polling timer'ını başlat
    g_timeout_add(MESSAGE_POLL_INTERVAL, poll_messages, buffer);
}

void handle_command(const char *input, int sender_id, GtkTextBuffer *text_buffer) {
    if (input == NULL || strlen(input) == 0) return;
    if (!GTK_IS_TEXT_BUFFER(text_buffer)) return;

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
    } else if (strncmp(input, "history", 7) == 0) {
        // History komutu çalıştırıldığında geçmişi göster
        ProcessInfo *history = get_process_history();
        int count = get_history_count();
        
        // Geçmişi View'de göster
        gtk_text_buffer_insert_at_cursor(text_buffer, "--- Komut Geçmişi ---\n", -1);
        for (int i = 0; i < count; i++) {
            char history_msg[1024];
            snprintf(history_msg, sizeof(history_msg), "Komut Geçmişi -> PID: %d, Komut: %s, Durum: %d\n",
                     history[i].pid, history[i].command, history[i].status);
            gtk_text_buffer_insert_at_cursor(text_buffer, history_msg, -1);
        }
    } else if (strncmp(input, "cd ", 3) == 0) {
        // cd komutunu işle
        const char *path = input + 3;
        if (chdir(path) != 0) {
            perror("chdir failed");
        }
    } else if (strncmp(input, "clear", 5) == 0) {
        // Clear the text buffer instead of the terminal
        gtk_text_buffer_set_text(text_buffer, "", -1);
    } else {
        // Shell komutunu çalıştır
        execute_command(input, shm, text_buffer); 
    }
}

