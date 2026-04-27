#include "database.h"

void showMessages(){
    FILE *fp=fopen(MSG_FILE,"r");
    char line[200];
    if(fp){
        while(fgets(line,200,fp))
            printf("MSG: %s",line);
        fclose(fp);
    }
}

void adminMenu(){
    showMessages();

    int ch;
    while(1){
        printf("\n1.List\n2.Search\n3.Mark Found\n4.Delete\n5.Edit\n0.Exit\n");
        scanf("%d",&ch);

        if(ch==1) listItems();
        else if(ch==2){
            int id; scanf("%d",&id);
            searchItem(id);
        }
        else if(ch==3){
            int id; scanf("%d",&id);
            markFound(id);
        }
        else if(ch==4){
            int id; scanf("%d",&id);
            deleteItem(id);
        }
        else if(ch==5){
            int id; scanf("%d",&id);
            editItem(id);
        }
        else break;
    }
}
