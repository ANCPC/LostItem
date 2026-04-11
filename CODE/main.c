#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "user_db.c"  // Include user database functions
// from user_db.c
void addUser();
void rewardUser(int userId);

// ===== ITEM STRUCT =====
struct Item {
    int id;
    char name[50];
    int ownerId;
    char description[100];
};

// ===== LOAD ITEMS FROM FILE =====
int loadItems(struct Item items[]) {
    FILE *fp = fopen("items.txt", "rb");
    int count = 0;

    if (fp == NULL) return 0;

    while (fread(&items[count], sizeof(struct Item), 1, fp)) {
        count++;
    }

    fclose(fp);
    return count;
}

// ===== SAVE ITEMS TO FILE =====
void saveItems(struct Item items[], int n) {
    FILE *fp = fopen("items.txt", "wb");

    for (int i = 0; i < n; i++) {
        fwrite(&items[i], sizeof(struct Item), 1, fp);
    }

    fclose(fp);
}

// ===== MERGE FUNCTION =====
void merge(struct Item arr[], int l, int m, int r) {
    int i = l, j = m + 1, k = 0;
    struct Item temp[100];

    while (i <= m && j <= r) {
        if (strcmp(arr[i].name, arr[j].name) < 0)
            temp[k++] = arr[i++];
        else
            temp[k++] = arr[j++];
    }

    while (i <= m) temp[k++] = arr[i++];
    while (j <= r) temp[k++] = arr[j++];

    for (i = l, k = 0; i <= r; i++, k++) {
        arr[i] = temp[k];
    }
}

// ===== MERGE SORT =====
void mergeSort(struct Item arr[], int l, int r) {
    if (l < r) {
        int m = (l + r) / 2;
        mergeSort(arr, l, m);
        mergeSort(arr, m + 1, r);
        merge(arr, l, m, r);
    }
}

// ===== ADD ITEM =====
void addItem() {
    struct Item items[100];
    int n = loadItems(items);

    struct Item it;

    printf("Enter Item ID: ");
    scanf("%d", &it.id);

    printf("Enter Item Name: ");
    scanf("%s", it.name);

    printf("Enter Owner ID: ");
    scanf("%d", &it.ownerId);

    printf("Enter Description: ");
    scanf(" %[^\n]", it.description);

    items[n++] = it;

    // sort after adding
    mergeSort(items, 0, n - 1);

    saveItems(items, n);

    printf("Item added & sorted successfully!\n");
}

// ===== VIEW ITEMS =====
void viewItems() {
    struct Item items[100];
    int n = loadItems(items);

    printf("\n===== Items List =====\n");

    for (int i = 0; i < n; i++) {
        printf("\nID: %d\n", items[i].id);
        printf("Name: %s\n", items[i].name);
        printf("Owner ID: %d\n", items[i].ownerId);
        printf("Description: %s\n", items[i].description);
    }
}

// ===== BINARY SEARCH =====
int binarySearch(struct Item items[], int n, char key[]) {
    int l = 0, r = n - 1;

    while (l <= r) {
        int mid = (l + r) / 2;
        int cmp = strcmp(items[mid].name, key);

        if (cmp == 0)
            return mid;
        else if (cmp < 0)
            l = mid + 1;
        else
            r = mid - 1;
    }

    return -1;
}

// ===== SEARCH ITEM BY NAME =====
void searchItem() {
    struct Item items[100];
    int n = loadItems(items);

    char key[50];
    printf("Enter item name to search: ");
    scanf("%s", key);

    int index = binarySearch(items, n, key);

    if (index != -1) {
        printf("\nItem Found!\n");
        printf("ID: %d\n", items[index].id);
        printf("Name: %s\n", items[index].name);
        printf("Owner ID: %d\n", items[index].ownerId);
        printf("Description: %s\n", items[index].description);
    } else {
        printf("Item not found.\n");
    }
}

// ===== MENU =====
void showMenu() {
    printf("\n===== Lost & Found =====\n");
    printf("1. Add User\n");
    printf("2. Reward User\n");
    printf("3. Add Item\n");
    printf("4. View Items\n");
    printf("5. Search Item (by name)\n");
    printf("6. Exit\n");
    printf("Enter choice: ");
}

// ===== MAIN =====
int main() {
    int choice, id;

    while (1) {
        showMenu();
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                addUser();
                break;

            case 2:
                printf("Enter User ID: ");
                scanf("%d", &id);
                rewardUser(id);
                break;

            case 3:
                addItem();
                break;

            case 4:
                viewItems();
                break;

            case 5:
                searchItem();
                break;

            case 6:
                printf("Exiting...\n");
                return 0;

            default:
                printf("Invalid choice!\n");
        }
    }
}