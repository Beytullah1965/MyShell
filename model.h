#ifndef MODEL_H
#define MODEL_H

#include <gtk/gtk.h>
#include <semaphore.h>
#include <sys/types.h>
#include <stddef.h>

#define MSG_BUF_SIZE 1024
#define MAX_COMMAND_LENGTH 512
#define MAX_HISTORY 100
#define MAX_TERMINAL 10  // Eklendi

typedef struct {
    pid_t pid;
    char command[MAX_COMMAND_LENGTH];
    int status;
} ProcessInfo;

typedef struct {
    char msgbuf[MSG_BUF_SIZE];
    int sender_id;
    int receiver_id;
    int cnt;
    sem_t sem;

    // Eklenenler
    int terminal_active[MAX_TERMINAL];  // Her terminal için aktiflik durumu
    ProcessInfo history[MAX_HISTORY];
    int history_count;
} ShmBuf;

extern ShmBuf *shm;

ShmBuf *buf_init();
char* model_read_messages(int terminal_id);
void send_message(ShmBuf *shmp, const char *msg, int sender_id, int receiver_id);
void receive_message(ShmBuf *shmp, int terminal_id);
void execute_command(const char *cmd, ShmBuf *shmp, GtkTextBuffer *text_buffer);
ProcessInfo *get_process_history();
int get_history_count();

#endif // MODEL_H

