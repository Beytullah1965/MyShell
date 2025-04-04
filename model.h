#ifndef MODEL_H
#define MODEL_H

#include <gtk/gtk.h>
#include <semaphore.h>
#include <sys/types.h>
#include <stddef.h>

// Paylaşılan bellek yapısı
#define MSG_BUF_SIZE 4096
#define MAX_COMMAND_LENGTH 512

typedef struct {
    sem_t sem;
    int fd;
    char msgbuf[MSG_BUF_SIZE];
    size_t cnt;
} ShmBuf;

typedef struct {
    pid_t pid;
    char command[MAX_COMMAND_LENGTH];
    int status;
} ProcessInfo;

// Fonksiyon prototipleri
ShmBuf *buf_init();
void execute_command(const char *cmd, ShmBuf *shmp, GtkTextBuffer *text_buffer);

void send_message(ShmBuf *shmp, const char *msg);
void receive_message(ShmBuf *shmp);

#endif // MODEL_H





