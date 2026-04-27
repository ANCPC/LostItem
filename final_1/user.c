#include "database.h"

void showUserMessages(char* user){
    FILE *fp=fopen(MSG_FILE,"r");
    char line[200];
    if(!fp) return;

    while(fgets(line,200,fp)){
        char uname[50];
        sscanf(line,"%[^|]|",uname);
        if(strcmp(uname,user)==0)
            printf("MSG: %s",line);
    }
    fclose(fp);
}

void userMenu(){
    char name[50];
    printf("Enter username: ");
    scanf("%s",name);

    showUserMessages(name);

    int ch;
    while(1){
        printf("\n1.List Items\n2.I Lost Something\n3.I Found Something\n0.Exit\n");
        scanf("%d",&ch);

        if(ch==1) listItems();
        else if(ch==2) addItem(name);
        else if(ch==3){
            int id;
            printf("Enter ID: ");
            scanf("%d",&id);

            FILE *fp=fopen(MSG_FILE,"a");
            fprintf(fp,"%s|User found item %d\n",name,id);
            fclose(fp);
        }
        else break;
    }
}
