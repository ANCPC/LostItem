#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static int nextID = 1000;

int generateID() {
    return nextID++;
}

int isValidPassword(char *pass) {
    return strlen(pass) >= 4;
}

int containsKeyword(char *text, char *word) {
    char a[300], b[100];
    strcpy(a, text);
    strcpy(b, word);
    for (int i = 0; a[i]; i++) a[i] = tolower(a[i]);
    for (int i = 0; b[i]; i++) b[i] = tolower(b[i]);
    return strstr(a, b) != NULL;
}

void searchItems() {
    char keyword[100];
    char line[400];
    FILE *fp;
    printf("\nSearch keyword: ");
    scanf("%99s", keyword);
    fp = fopen("items.txt", "r");
    if (!fp) { printf("No items.\n"); return; }
    printf("\n===== SEARCH RESULTS =====\n");
    while (fgets(line, sizeof(line), fp)) {
        if (containsKeyword(line, keyword))
            printf("%s", line);
    }
    fclose(fp);
}