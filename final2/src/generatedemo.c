#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_USERS 10
#define NUM_ITEMS 25
#define NUM_CLAIMS 15
#define NUM_MESSAGES 12

char *firstNames[] = {"Raj", "Simran", "Amit", "Priya", "Vikram", "Neha", "Rohan", "Kavya", "Arjun", "Divya"};
char *lastNames[] = {"Sharma", "Verma", "Singh", "Kaur", "Patel", "Reddy", "Gupta", "Nair"};

char *itemNames[] = {
    "Wallet", "Phone", "Laptop", "Backpack", "Water Bottle",
    "Sunglasses", "Headphones", "Watch", "Keys", "Umbrella",
    "Calculator", "Textbook", "ID Card", "Charger", "Pen Drive"
};

char *categories[] = {"Electronics", "Accessories", "Stationery", "Personal", "Clothing"};
char *locations[] = {"Library", "Canteen", "Classroom 101", "Lab", "Ground", "Parking"};

/* Generate a random username */
void genUsername(char *buffer, int idx) {
    sprintf(buffer, "%s%s%d", firstNames[idx % (sizeof(firstNames)/sizeof(char*))],
            lastNames[idx % (sizeof(lastNames)/sizeof(char*))], idx+1);
}

/* Generate a random password */
void genPassword(char *buffer) {
    sprintf(buffer, "pass%d", rand() % 9000 + 1000);
}

/* Generate random points (0 to 500) */
int genPoints() {
    return rand() % 501;
}

/* Generate random bounty (0 to 200) */
int genBounty() {
    return rand() % 201;
}

/* Generate random timestamp within last 30 days */
long genTimestamp() {
    long now = time(NULL);
    long offset = rand() % (30 * 24 * 3600);
    return now - offset;
}

/* Main generator */
int main() {
    srand(time(NULL));
    
    // -------------------- USERS --------------------
    FILE *fp = fopen("users.txt", "w");
    if (!fp) {
        printf("Cannot create users.txt\n");
        return 1;
    }
    
    // Admin user (must exist for admin login)
    fprintf(fp, "admin,admin123,ADMIN,500\n");
    
    // Regular users
    char username[50], password[20];
    int points;
    for (int i = 0; i < NUM_USERS; i++) {
        genUsername(username, i);
        genPassword(password);
        points = genPoints();
        fprintf(fp, "%s,%s,USER,%d\n", username, password, points);
    }
    fclose(fp);
    printf("Created users.txt with %d users + admin\n", NUM_USERS);
    
    // -------------------- ITEMS --------------------
    fp = fopen("items.txt", "w");
    if (!fp) {
        printf("Cannot create items.txt\n");
        return 1;
    }
    
    // We need a list of usernames to assign as owners
    char *usernames[NUM_USERS + 1];
    usernames[0] = "admin";
    for (int i = 0; i < NUM_USERS; i++) {
        char temp[50];
        genUsername(temp, i);
        usernames[i+1] = strdup(temp);
    }
    
    for (int i = 1; i <= NUM_ITEMS; i++) {
        int ownerIdx = rand() % (NUM_USERS + 1);  // 0 = admin, 1..NUM_USERS = users
        char *owner = usernames[ownerIdx];
        char *type = (rand() % 2) ? "LOST" : "FOUND";
        char *item = itemNames[rand() % (sizeof(itemNames)/sizeof(char*))];
        char *cat = categories[rand() % (sizeof(categories)/sizeof(char*))];
        char desc[100];
        sprintf(desc, "%s found in %s", item, locations[rand() % (sizeof(locations)/sizeof(char*))]);
        int bounty = 0;
        if (strcmp(type, "LOST") == 0) {
            bounty = genBounty();
            // Optionally reduce bounty if owner has low points (just for realism)
        }
        fprintf(fp, "%d|%s|%s|%s|%s|%s|%d\n", i, owner, type, item, cat, desc, bounty);
    }
    fclose(fp);
    printf("Created items.txt with %d items\n", NUM_ITEMS);
    
    // -------------------- CLAIMS --------------------
    fp = fopen("claims.txt", "w");
    if (!fp) {
        printf("Cannot create claims.txt\n");
        return 1;
    }
    
    char *statuses[] = {"PENDING", "APPROVED", "REJECTED"};
    for (int i = 1; i <= NUM_CLAIMS; i++) {
        int itemId = rand() % NUM_ITEMS + 1;
        // Claimant cannot be the owner, so we need to read owner of item
        // For simplicity, we'll just pick a random user and later ensure consistency if needed.
        // But for demo, it's fine; the C code will check anyway.
        int claimantIdx = rand() % NUM_USERS + 1;  // exclude admin as claimant? allow admin for demo
        char *claimant = usernames[claimantIdx];
        char *status = statuses[rand() % 3];
        long ts = genTimestamp();
        fprintf(fp, "%d|%d|%s|%s|%ld\n", i, itemId, claimant, status, ts);
    }
    fclose(fp);
    printf("Created claims.txt with %d claims\n", NUM_CLAIMS);
    
    // -------------------- MESSAGES --------------------
    fp = fopen("messages.txt", "w");
    if (!fp) {
        printf("Cannot create messages.txt\n");
        return 1;
    }
    
    char *msgTypes[] = {"NORMAL", "POINTS_REQUEST", "CLAIM_UPDATE", "ADMIN_MSG", "POINTS_GRANTED"};
    for (int i = 1; i <= NUM_MESSAGES; i++) {
        int fromIdx = rand() % (NUM_USERS + 1);
        int toIdx = rand() % (NUM_USERS + 1);
        char *from = (fromIdx == 0) ? "admin" : usernames[fromIdx];
        char *to = (toIdx == 0) ? "admin" : usernames[toIdx];
        char *type = msgTypes[rand() % 5];
        char msgText[200];
        
        if (strcmp(type, "POINTS_REQUEST") == 0) {
            int reqPoints = rand() % 300 + 50;
            sprintf(msgText, "%d|Need points for claiming an item", reqPoints);
        } else if (strcmp(type, "ADMIN_MSG") == 0) {
            sprintf(msgText, "Hello, your item claim has been reviewed.");
        } else if (strcmp(type, "POINTS_GRANTED") == 0) {
            sprintf(msgText, "Your points request has been granted.");
        } else if (strcmp(type, "CLAIM_UPDATE") == 0) {
            sprintf(msgText, "Your claim status has been updated.");
        } else {
            sprintf(msgText, "General message about lost & found item.");
        }
        
        long ts = genTimestamp();
        fprintf(fp, "%d|%s|%s|%s|%ld|%s\n", i, from, to, msgText, ts, type);
    }
    fclose(fp);
    printf("Created messages.txt with %d messages\n", NUM_MESSAGES);
    
    // -------------------- LOGS --------------------
    fp = fopen("logs.txt", "w");
    if (!fp) {
        printf("Cannot create logs.txt\n");
        return 1;
    }
    
    char *actions[] = {
        "User login success",
        "Item created",
        "Claim created",
        "Admin login success",
        "Message sent",
        "Claim approved",
        "Points awarded"
    };
    for (int i = 0; i < 20; i++) {
        long ts = genTimestamp();
        char *action = actions[rand() % (sizeof(actions)/sizeof(char*))];
        fprintf(fp, "%ld - %s\n", ts, action);
    }
    fclose(fp);
    printf("Created logs.txt with 20 log entries\n");
    
    // Free allocated usernames
    for (int i = 1; i <= NUM_USERS; i++) {
        free(usernames[i]);
    }
    
    printf("\n=====================================\n");
    printf("Database generation complete!\n");
    printf("You can now run the main program.\n");
    printf("Admin login: admin / admin123\n");
    printf("=====================================\n");
    
    return 0;
}