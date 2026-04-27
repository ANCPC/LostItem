#include "database.h"

void userMenu();
void adminMenu();

int main(){
    int ch;
    while(1){
        printf("\n1.User\n2.Admin\n0.Exit\n");
        scanf("%d",&ch);

        if(ch==1) userMenu();
        else if(ch==2) adminMenu();
        else break;
    }
    return 0;
}
