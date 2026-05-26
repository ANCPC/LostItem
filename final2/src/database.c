#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define USER_FILE "users.txt"
#define ITEM_FILE "items.txt"
#define CLAIM_FILE "claims.txt"
#define MSG_FILE "messages.txt"
#define LOG_FILE "logs.txt"

// Forward declarations for functions used inside database.c
int generateID();

/* ================= USERS ================= */
int userExists(char *username) {
    FILE *fp = fopen(USER_FILE, "r");
    char user[50], pass[50], role[10];
    int points;
    if (!fp) return 0;
    while (fscanf(fp, "%49[^,],%49[^,],%9[^,],%d\n", user, pass, role, &points) == 4) {
        if (strcmp(user, username) == 0) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

int saveUser(char *username, char *password) {
    FILE *fp = fopen(USER_FILE, "a");
    if (!fp) return 0;
    fprintf(fp, "%s,%s,USER,0\n", username, password);
    fclose(fp);
    return 1;
}

int verifyUser(char *username, char *password) {
    FILE *fp = fopen(USER_FILE, "r");
    char user[50], pass[50], role[10];
    int points;
    if (!fp) return 0;
    while (fscanf(fp, "%49[^,],%49[^,],%9[^,],%d\n", user, pass, role, &points) == 4) {
        if (strcmp(user, username) == 0 && strcmp(pass, password) == 0) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

int getUserPoints(char *username) {
    FILE *fp = fopen(USER_FILE, "r");
    char user[50], pass[50], role[10];
    int points;
    if (!fp) return 0;
    while (fscanf(fp, "%49[^,],%49[^,],%9[^,],%d\n", user, pass, role, &points) == 4) {
        if (strcmp(user, username) == 0) {
            fclose(fp);
            return points;
        }
    }
    fclose(fp);
    return 0;
}

int updateUserPoints(char *username, int delta) {
    FILE *fp = fopen(USER_FILE, "r");
    FILE *tmp = fopen("users.tmp", "w");
    if (!fp || !tmp) return 0;
    char user[50], pass[50], role[10];
    int points;
    int found = 0;
    while (fscanf(fp, "%49[^,],%49[^,],%9[^,],%d\n", user, pass, role, &points) == 4) {
        if (strcmp(user, username) == 0) {
            points += delta;
            found = 1;
        }
        fprintf(tmp, "%s,%s,%s,%d\n", user, pass, role, points);
    }
    fclose(fp);
    fclose(tmp);
    remove(USER_FILE);
    rename("users.tmp", USER_FILE);
    return found;
}

/* ================= ITEMS ================= */
int saveItem(int id, char *owner, char *type, char *name, char *category, char *desc, int bounty) {
    FILE *fp = fopen(ITEM_FILE, "a");
    if (!fp) return 0;
    fprintf(fp, "%d|%s|%s|%s|%s|%s|%d\n", id, owner, type, name, category, desc, bounty);
    fclose(fp);
    return 1;
}

void listItems() {
    FILE *fp = fopen(ITEM_FILE, "r");
    char line[400];
    if (!fp) { printf("No items.\n"); return; }
    printf("\n===== ITEMS =====\n");
    while (fgets(line, sizeof(line), fp))
        printf("%s", line);
    fclose(fp);
}

int getItemBounty(int itemId) {
    FILE *fp = fopen(ITEM_FILE, "r");
    if (!fp) return 0;
    int id, bounty;
    char owner[50], type[10], name[100], cat[100], desc[200];
    while (fscanf(fp, "%d|%49[^|]|%9[^|]|%99[^|]|%99[^|]|%199[^|]|%d\n", 
                  &id, owner, type, name, cat, desc, &bounty) == 7) {
        if (id == itemId) {
            fclose(fp);
            return bounty;
        }
    }
    fclose(fp);
    return 0;
}

char* getItemOwner(int itemId) {
    static char owner[50];
    FILE *fp = fopen(ITEM_FILE, "r");
    if (!fp) return NULL;
    int id, bounty;
    char type[10], name[100], cat[100], desc[200];
    while (fscanf(fp, "%d|%49[^|]|%9[^|]|%99[^|]|%99[^|]|%199[^|]|%d\n", 
                  &id, owner, type, name, cat, desc, &bounty) == 7) {
        if (id == itemId) {
            fclose(fp);
            return owner;
        }
    }
    fclose(fp);
    return NULL;
}

/* ================= CLAIMS ================= */
int saveClaim(int claimId, int itemId, char *username, char *status) {
    FILE *fp = fopen(CLAIM_FILE, "a");
    if (!fp) return 0;
    fprintf(fp, "%d|%d|%s|%s|%ld\n", claimId, itemId, username, status, time(NULL));
    fclose(fp);
    return 1;
}

void listClaims() {
    FILE *fp = fopen(CLAIM_FILE, "r");
    char line[300];
    if (!fp) { printf("No claims.\n"); return; }
    printf("\n===== CLAIMS =====\n");
    while (fgets(line, sizeof(line), fp))
        printf("%s", line);
    fclose(fp);
}

int updateClaimStatus(int claimId, char *newStatus) {
    FILE *fp = fopen(CLAIM_FILE, "r");
    FILE *tmp = fopen("claims.tmp", "w");
    if (!fp || !tmp) return 0;
    int id, itemId;
    char claimant[50], status[20];
    long ts;
    int found = 0;
    while (fscanf(fp, "%d|%d|%49[^|]|%19[^|]|%ld\n", &id, &itemId, claimant, status, &ts) == 5) {
        if (id == claimId) {
            strcpy(status, newStatus);
            found = 1;
        }
        fprintf(tmp, "%d|%d|%s|%s|%ld\n", id, itemId, claimant, status, ts);
    }
    fclose(fp);
    fclose(tmp);
    remove(CLAIM_FILE);
    rename("claims.tmp", CLAIM_FILE);
    return found;
}

/* ================= MESSAGES ================= */
int saveMessage(int id, char *fromUser, char *toUser, char *message, char *type) {
    FILE *fp = fopen(MSG_FILE, "a");
    if (!fp) return 0;
    fprintf(fp, "%d|%s|%s|%s|%ld|%s\n", id, fromUser, toUser, message, time(NULL), type);
    fclose(fp);
    return 1;
}

void showInbox(char *username) {
    FILE *fp = fopen(MSG_FILE, "r");
    if (!fp) { printf("No messages.\n"); return; }
    int id;
    char from[50], to[50], msg[300], type[30];
    long ts;
    printf("\n===== INBOX for %s =====\n", username);
    while (fscanf(fp, "%d|%49[^|]|%49[^|]|%299[^|]|%ld|%29[^\n]\n", 
                  &id, from, to, msg, &ts, type) == 6) {
        if (strcmp(to, username) == 0) {
            printf("[%s] From %s: %s\n", type, from, msg);
        }
    }
    fclose(fp);
}

void showPointRequests() {
    FILE *fp = fopen(MSG_FILE, "r");
    if (!fp) return;
    int id;
    char from[50], to[50], msg[300], type[30];
    long ts;
    printf("\n===== PENDING POINT REQUESTS =====\n");
    while (fscanf(fp, "%d|%49[^|]|%49[^|]|%299[^|]|%ld|%29[^\n]\n", 
                  &id, from, to, msg, &ts, type) == 6) {
        if (strcmp(to, "admin") == 0 && strcmp(type, "POINTS_REQUEST") == 0) {
            printf("ID:%d From:%s -> %s\n", id, from, msg);
        }
    }
    fclose(fp);
}

/* ================= LOGS ================= */
void writeLog(char *action) {
    FILE *fp = fopen(LOG_FILE, "a");
    if (!fp) return;
    fprintf(fp, "%ld - %s\n", time(NULL), action);
    fclose(fp);
}