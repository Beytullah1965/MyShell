#include "controller.h"
#include "model.h"
#include "view.h"
#include <string.h>
#include <gtk/gtk.h>
#include <stdbool.h>

#define MESSAGE_POLL_INTERVAL 300

typedef struct {
    GtkTextBuffer *buffer;
    int terminal_id;
} TerminalData;

static gboolean poll_messages(gpointer user_data) {
    TerminalData *tdata = (TerminalData *)user_data;
    GtkTextBuffer *buffer = tdata->buffer;
    int terminal_id = tdata->terminal_id;

    sem_wait(&shm->sem);
    if (shm->cnt > 0 && (shm->receiver_id == -1 || shm->receiver_id == terminal_id)) {
        GtkTextIter end;
        gtk_text_buffer_get_end_iter(buffer, &end);

        gtk_text_buffer_insert(buffer, &end, "[MSG] ", -1);
        gtk_text_buffer_insert(buffer, &end, shm->msgbuf, -1);
        gtk_text_buffer_insert(buffer, &end, "\n", -1);

        shm->cnt = 0;
        shm->receiver_id = -2;
    }
    sem_post(&shm->sem);

    return G_SOURCE_CONTINUE;
}

void init_controller(GtkTextBuffer *buffer, int terminal_id) {
    shm = buf_init();

    TerminalData *tdata = g_malloc(sizeof(TerminalData));
    tdata->buffer = buffer;
    tdata->terminal_id = terminal_id;

    g_timeout_add(MESSAGE_POLL_INTERVAL, poll_messages, tdata);
}

void handle_command(const char *input, int sender_id, GtkTextBuffer *text_buffer) {
    if (input == NULL || strlen(input) == 0) return;
    if (!GTK_IS_TEXT_BUFFER(text_buffer)) return;

    sem_wait(&shm->sem);
    {
        if (shm->history_count >= MAX_HISTORY) {
            memmove(&shm->history[0], &shm->history[1],
                    (MAX_HISTORY - 1) * sizeof(ProcessInfo));
            shm->history_count--;
        }

        ProcessInfo *info = &shm->history[shm->history_count];
        strncpy(info->command, input, MAX_COMMAND_LENGTH - 1);
        info->command[MAX_COMMAND_LENGTH - 1] = '\0';
        info->pid = getpid();
        info->status = 0;  // Artık kullanılmıyor ama yapıyı bozmayalım
        shm->history_count++;
    }
    sem_post(&shm->sem);

    // Private messaging: @msg <terminal_id> <message>
    if (strncmp(input, "@msg ", 5) == 0) {
        const char *msg_start = input + 5;
        int target_id = -1;
        const char *space = strchr(msg_start, ' ');

        if (space != NULL) {
            char id_str[16] = {0};
            size_t id_len = space - msg_start;
            if (id_len > 0 && id_len < sizeof(id_str)) {
                strncpy(id_str, msg_start, id_len);
                target_id = atoi(id_str);  // Terminal ID'yi al
            }
            msg_start = space + 1;  // Mesaj kısmı
        }

        if (target_id != -1 && strlen(msg_start) > 0) {
            sem_wait(&shm->sem);
            strncpy(shm->msgbuf, msg_start, MSG_BUF_SIZE - 1);
            shm->msgbuf[MSG_BUF_SIZE - 1] = '\0';
            shm->cnt = strlen(shm->msgbuf);
            shm->receiver_id = target_id;  // Hedef terminalin ID'si
            sem_post(&shm->sem);

            // Mesaj gönderildiğine dair bir bilgi eklemek
            GtkTextIter iter;
            gtk_text_buffer_get_end_iter(text_buffer, &iter);
            char info_msg[256];
            snprintf(info_msg, sizeof(info_msg), "Message sent to Terminal %d\n", target_id);
            gtk_text_buffer_insert(text_buffer, &iter, info_msg, -1);
        }
        else {
            // Geçersiz mesaj durumu
            GtkTextIter iter;
            gtk_text_buffer_get_end_iter(text_buffer, &iter);
            gtk_text_buffer_insert(text_buffer, &iter, "Invalid message format.\n", -1);
        }
    }
    else if (strcmp(input, "history") == 0) {
        sem_wait(&shm->sem);
        GtkTextIter iter;
        gtk_text_buffer_get_end_iter(text_buffer, &iter);
        gtk_text_buffer_insert(text_buffer, &iter, "--- Command History ---\n", -1);

        for (int i = 0; i < shm->history_count; i++) {
            char line[512];
            const char *cmd = shm->history[i].command;
            char truncated_cmd[100];

            strncpy(truncated_cmd, cmd, sizeof(truncated_cmd) - 1);
            truncated_cmd[sizeof(truncated_cmd) - 1] = '\0';
            if (strlen(cmd) > sizeof(truncated_cmd) - 1) {
                strcpy(truncated_cmd + sizeof(truncated_cmd) - 4, "...");
            }

            snprintf(line, sizeof(line), "%d: %s (PID: %d)\n",
                     i + 1,
                     truncated_cmd,
                     shm->history[i].pid);

            gtk_text_buffer_insert(text_buffer, &iter, line, -1);
        }
        sem_post(&shm->sem);
    }
    else if (strcmp(input, "clear") == 0) {
        gtk_text_buffer_set_text(text_buffer, "", -1);
    }
    else if (strncmp(input, "cd ", 3) == 0) {
        const char *path = input + 3;
        if (chdir(path) != 0) {
            GtkTextIter iter;
            gtk_text_buffer_get_end_iter(text_buffer, &iter);
            char err_msg[256];
            snprintf(err_msg, sizeof(err_msg), "cd: %s\n", strerror(errno));
            gtk_text_buffer_insert(text_buffer, &iter, err_msg, -1);
        }
    }
    else {
        execute_command(input, shm, text_buffer);
    }
}


