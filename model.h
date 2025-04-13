#ifndef MODEL_H
#define MODEL_H

#include <gtk/gtk.h>
#include <semaphore.h>
#include <sys/types.h>
#include <stddef.h>

// Paylaşılan bellek yapısı
#define MSG_BUF_SIZE 1024
#define MAX_COMMAND_LENGTH 512
#define MAX_HISTORY 100  // Yeni eklenen sabit

typedef struct {
    pid_t pid;
    char command[MAX_COMMAND_LENGTH];
    int status;
} ProcessInfo;

typedef struct {
    char msgbuf[MSG_BUF_SIZE];   // Mesaj içeriği
    int sender_id;               // Gönderen terminalin kimliği
    int receiver_id;             // Alıcı terminalin kimliği
    int cnt;                     // Mesaj uzunluğu
    sem_t sem;                   // Semaphore
    
    // Yeni eklenen alanlar
    ProcessInfo history[MAX_HISTORY];  // Komut geçmişi
    int history_count;                 // Geçmişteki komut sayısı
} ShmBuf;

extern ShmBuf *shm;

// Fonksiyon prototipleri
ShmBuf *buf_init();
char* model_read_messages(int terminal_id);
void send_message(ShmBuf *shmp, const char *msg, int sender_id, int receiver_id);
void receive_message(ShmBuf *shmp, int terminal_id);
void execute_command(const char *cmd, ShmBuf *shmp, GtkTextBuffer *text_buffer);
ProcessInfo *get_process_history();
int get_history_count();

#endif // MODEL_H