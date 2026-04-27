#ifndef DATABASE_H
#define DATABASE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FILE_NAME "items.txt"
#define MSG_FILE "messages.txt"

typedef struct {
    int id;
    char user[50];
    char item[100];
    char type[50];
    char desc[200];
    char time[50];
    int status;
} Item;

int loadItems(Item arr[]);
void saveItems(Item arr[], int n);
void sortItems(Item arr[], int n);
void listItems();
void addItem(char* user);
void searchItem(int id);
void markFound(int id);
void deleteItem(int id);
void editItem(int id);

#endif
