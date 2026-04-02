#include <stdio.h>
#include <stdlib.h>

// Structure for User
struct User {
    int id;
    char name[50];
    int points;
};

// Add user to file (database)
void addUser() {
    FILE *fp = fopen("users.txt", "a");

    struct User u;

    printf("Enter User ID: ");
    scanf("%d", &u.id);

    printf("Enter Name: ");
    scanf("%s", u.name);

    u.points = 0;  // initial points

    fwrite(&u, sizeof(u), 1, fp);

    fclose(fp);

    printf("User added successfully!\n");
}

// Reward system
void rewardUser(int userId) {
    FILE *fp = fopen("users.txt", "r");
    FILE *temp = fopen("temp.txt", "w");

    struct User u;

    while (fread(&u, sizeof(u), 1, fp)) {
        if (u.id == userId) {
            u.points += 10;  // reward points
            printf("User rewarded! New points: %d\n", u.points);
        }
        fwrite(&u, sizeof(u), 1, temp);
    }

    fclose(fp);
    fclose(temp);

    remove("users.txt");
    rename("temp.txt", "users.txt");
}

// Main function
int main() {
    int choice, id;

    printf("1. Add User\n2. Reward User\n");
    scanf("%d", &choice);

    if (choice == 1) {
        addUser();
    } else if (choice == 2) {
        printf("Enter User ID: ");
        scanf("%d", &id);
        rewardUser(id);
    }

    return 0;
}
