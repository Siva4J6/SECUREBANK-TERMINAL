/**
 * @file      pc_main.c
 * @brief     SecureBank PC Server Daemon
 * @author    Siva Reddy
 * @date      November 2026
 * 
 * @note      Handles UART request/reply protocol, flat-file database 
 *            lookups (users.txt), and transaction logging (transactions.txt).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include "uart.h"

#define MAX_USERS   10
#define MSG_BUF_LEN 64
#define BAUD_RATE   9600

typedef struct {
    char  tag_id[20];
    char  pin[10];
    char  name[50];
    float balance;
    char  status[10];
    int   failed_attempts;
} User;

static User users[MAX_USERS];
static int  user_count = 0;
static char logged_in_tag[20] = "";
static volatile int keep_running = 1;

/* Signal handler for safe shutdown on Ctrl+C */
void intHandler(int dummy) {
    keep_running = 0;
}

/* Loads the flat-file database into memory */
static int load_users(void) {
    FILE *fp = fopen("users.txt", "r");
    if (fp == NULL) {
        printf("Warning: users.txt not found, starting with 0 users\n");
        return 0;
    }

    int count = 0;
    char line[256];

    while (count < MAX_USERS && fgets(line, sizeof line, fp) != NULL) {
        int fields = sscanf(line, "%19[^|]|%9[^|]|%49[^|]|%f|%9[^|]|%d",
                            users[count].tag_id, users[count].pin,
                            users[count].name, &users[count].balance,
                            users[count].status, &users[count].failed_attempts);
        if (fields >= 4) {
            if (fields < 5) strcpy(users[count].status, "ACTIVE");
            if (fields < 6) users[count].failed_attempts = 0;
            count++;
        }
    }
    fclose(fp);
    return count;
}

/* Flushes the memory database back to the text file */
static void save_users(void) {
    FILE *fp = fopen("users.txt", "w");
    if (fp == NULL) {
        printf("Error: could not save users.txt\n");
        return;
    }
    for (int i = 0; i < user_count; i++) {
        fprintf(fp, "%s|%s|%s|%.2f|%s|%d\n",
                users[i].tag_id, users[i].pin, users[i].name,
                users[i].balance, users[i].status, users[i].failed_attempts);
    }
    fclose(fp);
}

/* Appends a new financial event to the ledger */
static void log_transaction(const char *tag_id, const char *type, float amount) {
    FILE *fp = fopen("transactions.txt", "a");
    if (fp == NULL) return;
    fprintf(fp, "%s|%s|%.2f\n", tag_id, type, amount);
    fclose(fp);
}

static User* find_user(const char *tag_id) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].tag_id, tag_id) == 0)
            return &users[i];
    }
    return NULL;
}

/* Compiles the 3 most recent transactions into a single protocol response */
static void send_mini_statement(int fd, char *reply, size_t reply_size) {
    struct MiniTxn {
        char type[10];
        float amount;
    } recent[3];

    int count = 0, i;
    FILE *fp;
    char tag[20], type[10];
    float amount;

    if (logged_in_tag[0] == '\0' || find_user(logged_in_tag) == NULL) {
        snprintf(reply, reply_size, "@ERR#NO_USER$");
        serialPutString(fd, reply);
        return;
    }

    fp = fopen("transactions.txt", "r");
    if (fp == NULL) {
        snprintf(reply, reply_size, "@OK\nNo transactions$");
        serialPutString(fd, reply);
        return;
    }

    while (fscanf(fp, "%19[^|]|%9[^|]|%f", tag, type, &amount) == 3) {
        int ch;
        while ((ch = fgetc(fp)) != '\n' && ch != EOF); 
        
        if (strcmp(tag, logged_in_tag) != 0) continue;

        if (count < 3) {
            strncpy(recent[count].type, type, sizeof(recent[count].type) - 1);
            recent[count].type[sizeof(recent[count].type) - 1] = '\0';
            recent[count].amount = amount;
            count++;
        } else {
            recent[0] = recent[1];
            recent[1] = recent[2];
            strncpy(recent[2].type, type, sizeof(recent[2].type) - 1);
            recent[2].type[sizeof(recent[2].type) - 1] = '\0';
            recent[2].amount = amount;
        }
    }
    fclose(fp);

    strcpy(reply, "@OK");
    if (count == 0) {
        strncat(reply, "\nNo transactions", reply_size - strlen(reply) - 1);
    } else {
        for (i = count - 1; i >= 0; i--) {
            char line[24];
            if (strcmp(recent[i].type, "Deposit") == 0)
                snprintf(line, sizeof line, "\nDep:%.2f", recent[i].amount);
            else if (strcmp(recent[i].type, "Withdraw") == 0)
                snprintf(line, sizeof line, "\nWdr:%.2f", recent[i].amount);
            else
                snprintf(line, sizeof line, "\n%.7s:%.2f", recent[i].type, recent[i].amount);

            if (strlen(reply) + strlen(line) + 2 >= reply_size) break;
            strcat(reply, line);
        }
    }

    if (strlen(reply) + 2 <= reply_size) {
        strcat(reply, "$");
    } else {
        snprintf(reply, reply_size, "@ERR#STATEMENT_TOO_LONG$");
    }

    printf("TX mini statement: %s\n", reply);
    serialPutString(fd, reply);
}

/* Parses incoming UART packets and dispatches logic */
static void handle_message(int fd, const char *msg) {
    char reply[MSG_BUF_LEN], tag[20], pin[10], new_pin[5];
    float amount;

    if (sscanf(msg, "#PIN:CHANGE#NEW:%4[0-9]#CARD:%19[^$]", new_pin, tag) == 2 && strlen(new_pin) == 4) {
        User *u = find_user(tag);
        if (u != NULL && logged_in_tag[0] != '\0' && strcmp(logged_in_tag, tag) == 0) {
            strcpy(u->pin, new_pin);
            save_users();
            snprintf(reply, sizeof reply, "@OK#PIN_CHANGED$");
        } else {
            snprintf(reply, sizeof reply, "@ERR#PIN_CHANGE_DENIED$");
        }
    }
    else if (strncmp(msg, "#TXN:MINISTAT#REQ", 17) == 0) {
        send_mini_statement(fd, reply, sizeof reply);
        return; 
    }
    else if (sscanf(msg, "#CARD:%19[^#]#PIN:%9[^$]", tag, pin) == 2) {
        User *u = find_user(tag);
        if (u == NULL) {
            snprintf(reply, sizeof reply, "@ERR#INVALID_CARD$");
        } else if (strcmp(u->status, "BLOCKED") == 0) {
            snprintf(reply, sizeof reply, "@ERR#USER_BLOCKED$");
        } else if (strcmp(u->pin, pin) == 0) {
            u->failed_attempts = 0;
            strcpy(u->status, "ACTIVE");
            strcpy(logged_in_tag, tag);
            save_users();
            snprintf(reply, sizeof reply, "@OK#LOGIN_SUCCESS$");
        } else {
            u->failed_attempts++;
            if (u->failed_attempts >= 3) {
                strcpy(u->status, "BLOCKED");
                save_users();
                logged_in_tag[0] = '\0';
                snprintf(reply, sizeof reply, "@ERR#USER_BLOCKED$");
            } else {
                save_users();
                snprintf(reply, sizeof reply, "@ERR#INVALID_PIN$");
            }
        }
    }
    else if (sscanf(msg, "#CARD:%19[^$]", tag) == 1) {
        User *u = find_user(tag);
        if (u == NULL)
            snprintf(reply, sizeof reply, "@ERR#INVALID_CARD$");
        else if (strcmp(u->status, "BLOCKED") == 0)
            snprintf(reply, sizeof reply, "@ERR#USER_BLOCKED$");
        else
            snprintf(reply, sizeof reply, "@OK#VALID_CARD$");
    }
    else if (strncmp(msg, "#TXN:BALANCE#REQ", 16) == 0) {
        User *u = find_user(logged_in_tag);
        if (u != NULL) snprintf(reply, sizeof reply, "@OK#BALANCE:%.2f$", u->balance);
        else snprintf(reply, sizeof reply, "@ERR#NO_USER$");
    }
    else if (sscanf(msg, "#TXN:DEPOSIT#AMOUNT:%f", &amount) == 1) {
        User *u = find_user(logged_in_tag);
        if (u == NULL) {
            snprintf(reply, sizeof reply, "@ERR#NO_USER$");
        } else if (amount <= 0.0f || amount != (float)((int)amount) || ((int)amount % 100) != 0) {
            snprintf(reply, sizeof reply, "@ERR#INVALID_AMOUNT$");
        } else {
            u->balance += amount;
            save_users();
            log_transaction(u->tag_id, "Deposit", amount);
            snprintf(reply, sizeof reply, "@OK#BALANCE:%.2f$", u->balance);
        }
    }
    else if (sscanf(msg, "#TXN:WITHDRAW#AMOUNT:%f", &amount) == 1) {
        User *u = find_user(logged_in_tag);
        if (u == NULL) {
            snprintf(reply, sizeof reply, "@ERR#NO_USER$");
        } else if (amount <= 0.0f || amount != (float)((int)amount) || ((int)amount % 100) != 0) {
            snprintf(reply, sizeof reply, "@ERR#INVALID_AMOUNT$");
        } else if (amount > u->balance) {
            snprintf(reply, sizeof reply, "@ERR#INSUFFICIENT_BALANCE$");
        } else {
            u->balance -= amount;
            save_users();
            log_transaction(u->tag_id, "Withdraw", amount);
            snprintf(reply, sizeof reply, "@OK#BALANCE:%.2f$", u->balance);
        }
    }
    else {
        snprintf(reply, sizeof reply, "@ERR#UNKNOWN_CMD$");
    }

    printf("RX: %s   TX: %s\n", msg, reply);
    serialPutString(fd, reply);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <serial-device>\n", argv[0]);
        fprintf(stderr, "Example: %s /dev/ttyUSB0\n", argv[0]);
        return 1;
    }

    signal(SIGINT, intHandler);

    user_count = load_users();
    printf("Loaded %d users from users.txt\n", user_count);

    int fd = serialOpen(argv[1], BAUD_RATE);
    if (fd < 0) {
        fprintf(stderr, "Unable to open serial device %s: %s\n", argv[1], strerror(errno));
        return 1;
    }
    printf("Serial port %s open at %d baud. Waiting for MCU...\n", argv[1], BAUD_RATE);

    char msg_buf[MSG_BUF_LEN];

    while (keep_running) {
        int len = serialGetString(fd, msg_buf, sizeof msg_buf);
        if (len <= 0) continue;
        handle_message(fd, msg_buf);
    }

    printf("\nShutting down server safely...\n");
    save_users();
    serialClose(fd);
    return 0;
}
