/*#include "model.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <gtk/gtk.h>  // GTK başlık dosyasını ekledik
#include <semaphore.h>

#define BUF_SIZE 4096
extern ShmBuf *shm;

#define errExit(msg)        \
    do {                    \
        perror(msg);        \
        exit(EXIT_FAILURE); \
    } while (0)

// 🔽 GLOBAL KOMUT GEÇMİŞİ BURADA TUTULUYOR
static ProcessInfo process_history[BUF_SIZE / sizeof(ProcessInfo)];
static int history_count = 0;

#include <sys/stat.h>

ShmBuf *buf_init(int terminal_id) {
    const char *shm_name = "/mymsgbuf_global";
    int fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (fd == -1) errExit("shm_open");

    // Only initialize size if we're creating it
    struct stat sb;
    if (fstat(fd, &sb) == -1) errExit("fstat");
    if (sb.st_size == 0) {
        if (ftruncate(fd, sizeof(ShmBuf)) == -1) errExit("ftruncate");
    }

    ShmBuf *shmp = mmap(NULL, sizeof(ShmBuf), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shmp == MAP_FAILED) errExit("mmap");

    // Initialize semaphore only once
    static bool sem_initialized = false;
    if (!sem_initialized) {
        if (sem_init(&shmp->sem, 1, 1) == -1) errExit("sem_init");
        shmp->available = 0;  // Initialize to no message
        sem_initialized = true;
    }

    close(fd);
    return shmp;
}

void execute_command(const char *cmd, ShmBuf *shmp, GtkTextBuffer *text_buffer) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        errExit("pipe failed");
    }

    pid_t pid = fork();
    if (pid == -1) {
        errExit("fork failed");
    } else if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);

        char *args[] = {"/bin/sh", "-c", (char *)cmd, NULL};
        execvp(args[0], args);
        errExit("execvp failed");
    } else {
        close(pipefd[1]);

        int status;
        waitpid(pid, &status, 0);

        // Komut çıktısını GUI’ye yaz
        char buffer[512];
        ssize_t nbytes;
        while ((nbytes = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[nbytes] = '\0';
            gtk_text_buffer_insert_at_cursor(text_buffer, buffer, -1);
        }
        close(pipefd[0]);

        // Geçmişi al ve GUI'ye yaz
        ProcessInfo *history = get_process_history();
        int history_count = get_history_count();
        gtk_text_buffer_insert_at_cursor(text_buffer, "\n--- Komut Geçmişi ---\n", -1);
        for (int i = 0; i < history_count; i++) {
            char history_msg[1024];
            snprintf(history_msg, sizeof(history_msg), "Komut Geçmişi -> PID: %d, Komut: %s, Durum: %d\n",
                     history[i].pid, history[i].command, history[i].status);
            gtk_text_buffer_insert_at_cursor(text_buffer, history_msg, -1);
        }
    }
}

char* model_read_messages() {
    if (shm == NULL) return NULL;
    
    sem_wait(&shm->sem);
    
    // Mesaj yoksa veya zaten okunduysa
    if (shm->available == 0) {
        sem_post(&shm->sem);
        return NULL;
    }
    
    char *msg = strdup(shm->msgbuf);
    shm->available = 0;  // Mesaj okundu olarak işaretle
    clock_gettime(CLOCK_REALTIME, &shm->last_update);
    
    sem_post(&shm->sem);
    return msg;
}

void send_message(ShmBuf *shmp, const char *msg, int sender_id) {
    if (!shmp || !msg) return;

    sem_wait(&shmp->sem);
    strncpy(shmp->msgbuf, msg, MSG_BUF_SIZE - 1);
    shmp->msgbuf[MSG_BUF_SIZE - 1] = '\0';
    shmp->sender_id = sender_id;
    shmp->available = 1;
    shmp->last_displayed = -1; // Reset display tracking
    clock_gettime(CLOCK_REALTIME, &shmp->last_update);
    sem_post(&shmp->sem);
    g_print("DEBUG: [Terminal%d] Gönderilen mesaj: %s\n", sender_id, msg);
}

char* receive_message(ShmBuf *shmp, int receiver_id) {
    if (!shmp) return NULL;

    sem_wait(&shmp->sem);
    char *msg = NULL;

    // Only receive if message is available AND it's either:
    // 1. A message from another terminal, OR
    // 2. Our own message that hasn't been displayed yet
    if (shmp->available && 
        (shmp->sender_id != receiver_id || shmp->last_displayed != receiver_id)) {
        msg = strdup(shmp->msgbuf);
        
        // Mark as displayed for this terminal
        shmp->last_displayed = receiver_id;
        
        // Clear available flag if it's our own message or if it's been shown to all terminals
        if (shmp->sender_id == receiver_id) {
            shmp->available = 0;
        }
        
        g_print("DEBUG: Mesaj alındı (Terminal%d<-Terminal%d): %s\n",
               receiver_id, shmp->sender_id, msg);
    }
    sem_post(&shmp->sem);
    return msg;
}

ProcessInfo *get_process_history() {
    return process_history;
}

int get_history_count() {
    return history_count;
}

int get_sender_id(ShmBuf *shmp) {
    if (shmp == NULL) return -1;
    return shmp->sender_id;
}

extern GtkWidget *create_terminal_window();
extern GtkTextBuffer *global_text_buffer;
*/

#include "model.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/wait.h>
#include <gtk/gtk.h>
#include <semaphore.h>

#define BUF_SIZE 4096
extern ShmBuf *shm;

#define errExit(msg)        \
    do {                    \
        perror(msg);        \
        exit(EXIT_FAILURE); \
    } while (0)

static ProcessInfo process_history[BUF_SIZE / sizeof(ProcessInfo)];
static int history_count = 0;

// Paylaşılan bellek başlatma
ShmBuf *buf_init() {
    int fd = shm_open("mymsgbuf", O_CREAT | O_RDWR, 0600);
    if (fd < 0) {
        errExit("shm_open failed");
    }
    ftruncate(fd, sizeof(ShmBuf));
    ShmBuf *shmp = mmap(NULL, sizeof(ShmBuf), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shmp == MAP_FAILED) {
        errExit("mmap failed");
    }
    sem_init(&shmp->sem, 1, 1);
    
    shmp->cnt = 0;
    return shmp;
}

// Komutları çalıştırma
void execute_command(const char *cmd, ShmBuf *shmp, GtkTextBuffer *text_buffer) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        errExit("pipe failed");
    }

    pid_t pid = fork();
    if (pid == -1) {
        errExit("fork failed");
    } else if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);

        char *args[] = {"/bin/sh", "-c", (char *)cmd, NULL};
        execvp(args[0], args);
        errExit("execvp failed");
    } else {
        close(pipefd[1]);

        int status;
        waitpid(pid, &status, 0);

        // Komut çıktısını GUI’ye yaz
        char buffer[512];
        ssize_t nbytes;
        while ((nbytes = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[nbytes] = '\0';
            gtk_text_buffer_insert_at_cursor(text_buffer, buffer, -1);
        }
        close(pipefd[0]);

        // Geçmişi al ve GUI'ye yaz
        ProcessInfo *history = get_process_history();
        int history_count = get_history_count();
        gtk_text_buffer_insert_at_cursor(text_buffer, "\n--- Komut Geçmişi ---\n", -1);
        for (int i = 0; i < history_count; i++) {
            char history_msg[1024];
            snprintf(history_msg, sizeof(history_msg), "Komut Geçmişi -> PID: %d, Komut: %s, Durum: %d\n",
                     history[i].pid, history[i].command, history[i].status);
            gtk_text_buffer_insert_at_cursor(text_buffer, history_msg, -1);
        }
    }
}

// Mesajları okuma (terminal numarasına göre)
char* model_read_messages(int terminal_id) {
    if (shm->cnt > 0) {
        // Eğer mesaj bu terminalden gelmişse, okuma işlemini gerçekleştir
        if (shm->sender_id != terminal_id) {
            return shm->msgbuf;
        }
    }
    return NULL;
}

// Mesaj gönderme (hedef terminal numarasını kontrol et)
void send_message(ShmBuf *shmp, const char *msg, int sender_id, int receiver_id) {
    sem_wait(&shmp->sem);
    // Eğer hedef terminal numarası doğruysa, mesajı yaz
    if (receiver_id == -1 || receiver_id == sender_id) {
        strncpy(shmp->msgbuf, msg, MSG_BUF_SIZE);
        shmp->sender_id = sender_id;
        shmp->receiver_id = receiver_id;  // Hedef terminali belirt
        shmp->cnt = strlen(msg);
        sem_post(&shmp->sem);
        g_print("DEBUG: [Terminal%d] Gönderilen mesaj: %s\n", sender_id, msg);
    }
    sem_post(&shmp->sem);
}

// Mesaj almayı sıfırlama (veya işaretleme)
void receive_message(ShmBuf *shmp, int terminal_id) {
    sem_wait(&shmp->sem);
    if (shmp->receiver_id == terminal_id) {
        shmp->cnt = 0;  // Mesaj alındıktan sonra sıfırla
    }
    sem_post(&shmp->sem);
}

// Komut geçmişini almak
ProcessInfo *get_process_history() {
    return process_history;
}

// Geçmiş sayısını almak
int get_history_count() {
    return history_count;
}




