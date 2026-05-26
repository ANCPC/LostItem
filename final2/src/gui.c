#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Declarations from other modules
int userExists(char *username);
int saveUser(char *username, char *password);
int verifyUser(char *username, char *password);
void writeLog(char *action);
int isValidPassword(char *pass);
void searchItems();
void listItems();
void listClaims();
void showInbox(char *username);
int getUserPoints(char *username);
int createItem(char *owner, char *type, char *name, char *category, char *desc, int bounty);
int createClaim(char *username, int itemId);
int sendMessageCore(char *fromUser, char *toUser, char *message, char *type);
int requestPoints(char *username, int points, char *reason);
int approveClaimWithPoints(int claimId);
int updateClaimStatus(int claimId, char *status);
void showPointRequests();
void showInbox(char *username);
char* getItemOwner(int itemId);

/* ================= AUTH ================= */
int signupUser() {
    char username[50], password[50];
    printf("\n===== SIGNUP =====\nUsername: ");
    scanf("%49s", username);
    if (userExists(username)) {
        printf("User already exists.\n");
        return 0;
    }
    printf("Password: ");
    scanf("%49s", password);
    if (!isValidPassword(password)) {
        printf("Weak password. Minimum 4 chars.\n");
        return 0;
    }
    if (saveUser(username, password)) {
        printf("Signup successful.\n");
        writeLog("New user signup");
        return 1;
    }
    printf("Signup failed.\n");
    return 0;
}

int loginUser(char *loggedUser) {
    char username[50], password[50];
    printf("\n===== USER LOGIN =====\nUsername: ");
    scanf("%49s", username);
    printf("Password: ");
    scanf("%49s", password);
    if (verifyUser(username, password)) {
        strcpy(loggedUser, username);
        writeLog("User login success");
        return 1;
    }
    return 0;
}

int loginAdmin() {
    char user[50], pass[50];
    printf("\n===== ADMIN LOGIN =====\nAdmin Username: ");
    scanf("%49s", user);
    printf("Admin Password: ");
    scanf("%49s", pass);
    if (strcmp(user, "admin") == 0 && strcmp(pass, "admin123") == 0) {
        writeLog("Admin login success");
        return 1;
    }
    return 0;
}

/* ================= USER FEATURES ================= */
void reportItem(char *username, char *type) {
    char name[100], category[100], desc[200];
    int bounty = 0;
    printf("\nItem Name: ");
    scanf(" %99[^\n]", name);
    printf("Category: ");
    scanf(" %99[^\n]", category);
    printf("Description: ");
    scanf(" %199[^\n]", desc);
    if (strcmp(type, "LOST") == 0) {
        printf("Bounty points (will be deducted from you if found): ");
        scanf("%d", &bounty);
        if (bounty < 0) bounty = 0;
        int myPoints = getUserPoints(username);
        if (myPoints < bounty) {
            printf("Insufficient points. You have %d points.\n", myPoints);
            return;
        }
    }
    int id = createItem(username, type, name, category, desc, bounty);
    if (id != -1)
        printf("Item saved. ID = %d\n", id);
    else
        printf("Failed.\n");
}

void claimItem(char *username) {
    int itemId;
    printf("\nEnter Item ID to claim: ");
    scanf("%d", &itemId);
    char *owner = getItemOwner(itemId);
    if (!owner) {
        printf("Item not found.\n");
        return;
    }
    if (strcmp(owner, username) == 0) {
        printf("You cannot claim your own item.\n");
        return;
    }
    int claimId = createClaim(username, itemId);
    if (claimId != -1)
        printf("Claim submitted. ID = %d\n", claimId);
    else
        printf("Claim failed.\n");
}

void userPoints(char *username) {
    int points = getUserPoints(username);
    printf("\n===== YOUR POINTS =====\nYou have %d points.\n", points);
}

void requestPointsMenu(char *username) {
    int pts;
    char reason[200];
    printf("\n===== REQUEST POINTS =====\nPoints requested: ");
    scanf("%d", &pts);
    if (pts <= 0) {
        printf("Invalid amount.\n");
        return;
    }
    printf("Reason: ");
    scanf(" %199[^\n]", reason);
    if (requestPoints(username, pts, reason))
        printf("Request sent to admin.\n");
    else
        printf("Failed to send request.\n");
}

void userHistory(char *username) {
    printf("\n===== USER HISTORY =====\n(Simplified) Check inbox and your claims.\n");
}

void userDashboard(char *username) {
    int choice;
    while (1) {
        printf("\n=========================\n");
        printf(" USER DASHBOARD (%s) - Points: %d\n", username, getUserPoints(username));
        printf("=========================\n");
        printf("1. Report Lost Item\n2. Report Found Item\n3. Search Items\n4. Claim Item\n");
        printf("5. Inbox\n6. My Points\n7. Request Points\n8. History\n9. Logout\nChoice: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1: reportItem(username, "LOST"); break;
            case 2: reportItem(username, "FOUND"); break;
            case 3: searchItems(); break;
            case 4: claimItem(username); break;
            case 5: showInbox(username); break;
            case 6: userPoints(username); break;
            case 7: requestPointsMenu(username); break;
            case 8: userHistory(username); break;
            case 9: return;
            default: printf("Invalid.\n");
        }
    }
}

/* ================= ADMIN FEATURES ================= */
void adminViewAllItems() { listItems(); }
void adminViewClaims() { listClaims(); }

void adminApproveRejectClaims() {
    listClaims();
    int claimId, choice;
    printf("\nEnter Claim ID to process: ");
    scanf("%d", &claimId);
    printf("1. Approve (transfer bounty)\n2. Reject\nChoice: ");
    scanf("%d", &choice);
    if (choice == 1) {
        if (approveClaimWithPoints(claimId))
            printf("Claim approved and points transferred.\n");
        else
            printf("Approval failed (maybe already processed or insufficient owner points).\n");
    } else if (choice == 2) {
        if (updateClaimStatus(claimId, "REJECTED"))
            printf("Claim rejected.\n");
        else
            printf("Failed.\n");
    } else printf("Invalid.\n");
}

void adminViewPointRequests() {
    showPointRequests();
    int msgId, pts;
    char username[50];
    printf("\nEnter Message ID to award points (0 to skip): ");
    scanf("%d", &msgId);
    if (msgId == 0) return;
    // We need to parse the message file to extract details
    FILE *fp = fopen("messages.txt", "r");
    FILE *tmp = fopen("messages.tmp", "w");
    if (!fp || !tmp) return;
    int id;
    char from[50], to[50], msg[300], type[30];
    long ts;
    int found = 0;
    int points = 0;
    char requester[50];
    while (fscanf(fp, "%d|%49[^|]|%49[^|]|%299[^|]|%ld|%29[^\n]\n", 
                  &id, from, to, msg, &ts, type) == 6) {
        if (id == msgId && strcmp(type, "POINTS_REQUEST") == 0) {
            found = 1;
            strcpy(requester, from);
            sscanf(msg, "%d", &points);
            continue; // skip this request (delete it)
        }
        fprintf(tmp, "%d|%s|%s|%s|%ld|%s\n", id, from, to, msg, ts, type);
    }
    fclose(fp);
    fclose(tmp);
    if (found) {
        remove("messages.txt");
        rename("messages.tmp", "messages.txt");
        // Update user points
        FILE *userFile = fopen("users.txt", "r");
        FILE *userTmp = fopen("users.tmp", "w");
        if (userFile && userTmp) {
            char u[50], p[50], r[10];
            int pts;
            while (fscanf(userFile, "%49[^,],%49[^,],%9[^,],%d\n", u, p, r, &pts) == 4) {
                if (strcmp(u, requester) == 0) {
                    pts += points;
                    printf("Awarded %d points to %s. New total: %d\n", points, requester, pts);
                }
                fprintf(userTmp, "%s,%s,%s,%d\n", u, p, r, pts);
            }
            fclose(userFile);
            fclose(userTmp);
            remove("users.txt");
            rename("users.tmp", "users.txt");
        }
        char notif[200];
        sprintf(notif, "Your request for %d points has been granted.", points);
        sendMessageCore("admin", requester, notif, "POINTS_GRANTED");
    } else {
        remove("messages.tmp");
        printf("Request not found.\n");
    }
}

void adminSendMessage() {
    char to[50], msg[200];
    printf("Recipient username: ");
    scanf("%49s", to);
    printf("Message: ");
    scanf(" %199[^\n]", msg);
    if (sendMessageCore("admin", to, msg, "ADMIN_MSG"))
        printf("Message sent.\n");
    else
        printf("Failed.\n");
}

void adminShowLogs() {
    FILE *fp = fopen("logs.txt", "r");
    char line[300];
    if (!fp) { printf("No logs.\n"); return; }
    printf("\n===== SYSTEM LOGS =====\n");
    while (fgets(line, sizeof(line), fp))
        printf("%s", line);
    fclose(fp);
}

void adminDashboard() {
    int choice;
    while (1) {
        printf("\n=========================\n");
        printf("      ADMIN DASHBOARD\n");
        printf("=========================\n");
        printf("1. View All Items\n2. View Claims\n3. Approve/Reject Claims (with points)\n");
        printf("4. View Point Requests & Award\n5. Send Message to User\n6. View Logs\n7. Logout\nChoice: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1: adminViewAllItems(); break;
            case 2: adminViewClaims(); break;
            case 3: adminApproveRejectClaims(); break;
            case 4: adminViewPointRequests(); break;
            case 5: adminSendMessage(); break;
            case 6: adminShowLogs(); break;
            case 7: return;
            default: printf("Invalid.\n");
        }
    }
}