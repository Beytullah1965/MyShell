/*#ifndef MODEL_H
#define MODEL_H

#include <gtk/gtk.h>
#include <semaphore.h>
#include <sys/types.h>
#include <stddef.h>

// Paylaşılan bellek yapısı
#define MSG_BUF_SIZE 1024
#define MAX_COMMAND_LENGTH 512

typedef struct {
    char msgbuf[MSG_BUF_SIZE];   // Mesaj içeriği
    int sender_id;               // Gönderen terminalin kimliği
    int receiver_id;             // Alıcı terminalin kimliği
    int cnt;                     // Mesaj uzunluğu
    sem_t sem;                   // Semaphore
} ShmBuf;

extern ShmBuf *shm;

typedef struct {
    pid_t pid;
    char command[MAX_COMMAND_LENGTH];
    int status;
} ProcessInfo;

// Fonksiyon prototipleri
ShmBuf *buf_init();
char* model_read_messages(int terminal_id);
void send_message(ShmBuf *shmp, const char *msg, int sender_id, int receiver_id);
void receive_message(ShmBuf *shmp, int terminal_id);
void execute_command(const char *cmd, ShmBuf *shmp, GtkTextBuffer *text_buffer);
ProcessInfo *get_process_history();  // Geçmişi almak için getter
int get_history_count();  // Geçmiş sayısını almak için

#endif // MODEL_H
*/

#ifndef MODEL_H
#define MODEL_H

#include <gtk/gtk.h>
#include <semaphore.h>
#include <sys/types.h>
#include <stddef.h>

// Paylaşılan bellek yapısı
#define MSG_BUF_SIZE 1024
#define MAX_COMMAND_LENGTH 512

typedef struct {
    char msgbuf[MSG_BUF_SIZE];   // Mesaj içeriği
    int sender_id;               // Gönderen terminalin kimliği
    int receiver_id;             // Alıcı terminalin kimliği
    int cnt;                     // Mesaj uzunluğu
    sem_t sem;                   // Semaphore
} ShmBuf;

extern ShmBuf *shm;

typedef struct {
    pid_t pid;
    char command[MAX_COMMAND_LENGTH];
    int status;
} ProcessInfo;

// Fonksiyon prototipleri
ShmBuf *buf_init();
char* model_read_messages(int terminal_id);
void send_message(ShmBuf *shmp, const char *msg, int sender_id, int receiver_id);
void receive_message(ShmBuf *shmp, int terminal_id);
void execute_command(const char *cmd, ShmBuf *shmp, GtkTextBuffer *text_buffer);
ProcessInfo *get_process_history();  // Geçmişi almak için getter
int get_history_count();  // Geçmiş sayısını almak için

#endif // MODEL_H

