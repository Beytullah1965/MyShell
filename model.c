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
ShmBuf *shm = NULL;

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

void execute_command(const char *cmd, ShmBuf *shmp, GtkTextBuffer *text_buffer) {
    if (!GTK_IS_TEXT_BUFFER(text_buffer)) return;

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        return;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        return;
    } else if (pid == 0) {
        // Child process
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);

        char *args[] = {"/bin/sh", "-c", (char *)cmd, NULL};
        execvp(args[0], args);

        // Eğer exec başarısızsa
        perror("exec failed");
        exit(1);
    } else {
        // Parent process
        close(pipefd[1]);

        int status;
        waitpid(pid, &status, 0);

        // Komut çıktısını oku ve ekrana yaz
        char buffer[512];
        ssize_t nbytes;
        while ((nbytes = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[nbytes] = '\0';
            gtk_text_buffer_insert_at_cursor(text_buffer, buffer, -1);
        }
        close(pipefd[0]);

        // Durum kodunu yorumla
        int exit_status = (WIFEXITED(status)) ? WEXITSTATUS(status) : 1;

        // PID ve durumu shared memory'ye yaz
        sem_wait(&shmp->sem);
        if (shmp->history_count > 0) {
            shmp->history[shmp->history_count - 1].pid = pid;
            shmp->history[shmp->history_count - 1].status = exit_status;
        }
        sem_post(&shmp->sem);
    }
}



// Mesajları okuma (yalnızca hedef terminal ise)
char* model_read_messages(int terminal_id) {
    if (shm->cnt > 0 && shm->receiver_id == terminal_id) {
        return shm->msgbuf;
    }
    return NULL;
}

// Mesaj gönderme
void send_message(ShmBuf *shmp, const char *msg, int sender_id, int receiver_id) {
    sem_wait(&shmp->sem);

    strncpy(shmp->msgbuf, msg, MSG_BUF_SIZE - 1);
    shmp->msgbuf[MSG_BUF_SIZE - 1] = '\0';  // null-terminate
    shmp->sender_id = sender_id;
    shmp->receiver_id = receiver_id;  // -1: broadcast, >=0: belirli terminal
    shmp->cnt = strlen(msg);

    sem_post(&shmp->sem);
    g_print("DEBUG: [Terminal%d] Gönderilen mesaj -> %s\n", sender_id, msg);
}

// Mesajı alındı olarak işaretle
void receive_message(ShmBuf *shmp, int terminal_id) {
    sem_wait(&shmp->sem);
    if (shmp->receiver_id == terminal_id) {
        shmp->cnt = 0;  // mesaj okunmuş say
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

