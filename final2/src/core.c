#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Declarations from other modules (no header)
int generateID();
int saveItem(int id, char *owner, char *type, char *name, char *category, char *desc, int bounty);
int saveClaim(int claimId, int itemId, char *username, char *status);
int saveMessage(int id, char *fromUser, char *toUser, char *message, char *type);
void writeLog(char *action);
int getUserPoints(char *username);
int updateUserPoints(char *username, int delta);
int getItemBounty(int itemId);
char* getItemOwner(int itemId);
int updateClaimStatus(int claimId, char *status);

int createItem(char *owner, char *type, char *name, char *category, char *desc, int bounty) {
    int id = generateID();
    if (saveItem(id, owner, type, name, category, desc, bounty)) {
        writeLog("Item created");
        return id;
    }
    return -1;
}

int createClaim(char *username, int itemId) {
    int claimId = generateID();
    if (saveClaim(claimId, itemId, username, "PENDING")) {
        writeLog("Claim created");
        return claimId;
    }
    return -1;
}

int sendMessageCore(char *fromUser, char *toUser, char *message, char *type) {
    int msgId = generateID();
    if (saveMessage(msgId, fromUser, toUser, message, type)) {
        writeLog("Message sent");
        return 1;
    }
    return 0;
}

int requestPoints(char *username, int points, char *reason) {
    char msg[300];
    sprintf(msg, "%d|%s", points, reason);
    return sendMessageCore(username, "admin", msg, "POINTS_REQUEST");
}

int approveClaimWithPoints(int claimId) {
    // Read claim details
    FILE *fp = fopen("claims.txt", "r");
    FILE *tmp = fopen("claims.tmp", "w");
    if (!fp || !tmp) return 0;
    int id, itemId;
    char claimant[50], status[20];
    long ts;
    int found = 0;
    int itemId_val = 0;
    char claimant_val[50];
    while (fscanf(fp, "%d|%d|%49[^|]|%19[^|]|%ld\n", &id, &itemId, claimant, status, &ts) == 5) {
        if (id == claimId && strcmp(status, "PENDING") == 0) {
            found = 1;
            strcpy(claimant_val, claimant);
            itemId_val = itemId;
            strcpy(status, "APPROVED");
        }
        fprintf(tmp, "%d|%d|%s|%s|%ld\n", id, itemId, claimant, status, ts);
    }
    fclose(fp);
    fclose(tmp);
    if (!found) {
        remove("claims.tmp");
        return 0;
    }
    remove("claims.txt");
    rename("claims.tmp", "claims.txt");

    // Transfer points
    char *owner = getItemOwner(itemId_val);
    int bounty = getItemBounty(itemId_val);
    if (owner && bounty > 0) {
        int ownerPoints = getUserPoints(owner);
        if (ownerPoints >= bounty) {
            updateUserPoints(owner, -bounty);
            updateUserPoints(claimant_val, bounty);
            char logmsg[200];
            sprintf(logmsg, "Claim %d approved: %d points from %s to %s", claimId, bounty, owner, claimant_val);
            writeLog(logmsg);
            char notif[300];
            sprintf(notif, "Your claim on item %d was approved. You gained %d points.", itemId_val, bounty);
            sendMessageCore("system", claimant_val, notif, "CLAIM_UPDATE");
            sprintf(notif, "User %s claimed your item %d. %d points deducted.", claimant_val, itemId_val, bounty);
            sendMessageCore("system", owner, notif, "CLAIM_UPDATE");
        } else {
            // insufficient points - reject instead
            updateClaimStatus(claimId, "REJECTED");
            char msg[200];
            sprintf(msg, "Claim %d rejected: owner insufficient points.", claimId);
            writeLog(msg);
            return 0;
        }
    } else {
        // no bounty, just approve without transfer
        writeLog("Claim approved (no bounty)");
    }
    return 1;
}