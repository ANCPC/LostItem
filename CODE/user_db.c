#include <stdio.h>
#include <stdlib.h>

// Structure for User
struct User {
    int id;
    char name[50];
    int points;
};

// Add user to file
void addUser() {
    FILE *fp = fopen("users.txt", "ab");  // binary append

    if (fp == NULL) {
        printf("Error opening file!\n");
        return;
    }

    struct User u;

    printf("Enter User ID: ");
    scanf("%d", &u.id);

    printf("Enter Name: ");
    scanf("%s", u.name);

    u.points = 0;

    fwrite(&u, sizeof(u), 1, fp);

    fclose(fp);

    printf("User added successfully!\n");
}

// Reward system
void rewardUser(int userId) {
    FILE *fp = fopen("users.txt", "rb");
    FILE *temp = fopen("temp.txt", "wb");

    if (fp == NULL || temp == NULL) {
        printf("Error opening file!\n");
        return;
    }

    struct User u;
    int found = 0;

    while (fread(&u, sizeof(u), 1, fp)) {
        if (u.id == userId) {
            u.points += 10;
            found = 1;
            printf("User rewarded! New points: %d\n", u.points);
        }
        fwrite(&u, sizeof(u), 1, temp);
    }

    fclose(fp);
    fclose(temp);

    remove("users.txt");
    rename("temp.txt", "users.txt");

    if (!found) {
        printf("User not found!\n");
    }
}