#include <stdio.h>
#include <stdlib.h>

// Declarations from other modules
int signupUser();
int loginUser(char *loggedUser);
int loginAdmin();
void userDashboard(char *username);
void adminDashboard();

void showMainMenu() {
    printf("\n=================================\n");
    printf("      LOST & FOUND SYSTEM\n");
    printf("=================================\n");
    printf("1. User Signup\n2. User Login\n3. Admin Login\n4. Exit\nChoice: ");
}

void runSystem() {
    int choice;
    char username[50];
    while (1) {
        showMainMenu();
        scanf("%d", &choice);
        switch (choice) {
            case 1: signupUser(); break;
            case 2:
                if (loginUser(username)) {
                    printf("Login successful. Welcome %s\n", username);
                    userDashboard(username);
                } else printf("Invalid credentials.\n");
                break;
            case 3:
                if (loginAdmin()) {
                    printf("Admin login successful.\n");
                    adminDashboard();
                } else printf("Admin login failed.\n");
                break;
            case 4: printf("Exiting...\n"); exit(0);
            default: printf("Invalid choice.\n");
        }
    }
}

int main() {
    runSystem();
    return 0;
}