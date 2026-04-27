#include "database.h"

int loadItems(Item arr[]){
    FILE *fp = fopen(FILE_NAME,"r");
    int i=0;
    if(!fp) return 0;
    while(fscanf(fp,"%d,%[^,],%[^,],%[^,],%[^,],%[^,],%d\n",
        &arr[i].id,arr[i].user,arr[i].item,arr[i].type,
        arr[i].desc,arr[i].time,&arr[i].status)==7){
        i++;
    }
    fclose(fp);
    return i;
}

void saveItems(Item arr[], int n){
    FILE *fp = fopen(FILE_NAME,"w");
    for(int i=0;i<n;i++){
        fprintf(fp,"%d,%s,%s,%s,%s,%s,%d\n",
        arr[i].id,arr[i].user,arr[i].item,arr[i].type,
        arr[i].desc,arr[i].time,arr[i].status);
    }
    fclose(fp);
}

void sortItems(Item arr[], int n){
    for(int i=0;i<n;i++)
        for(int j=i+1;j<n;j++)
            if(arr[i].id > arr[j].id){
                Item t=arr[i]; arr[i]=arr[j]; arr[j]=t;
            }
}

void listItems(){
    Item arr[200];
    int n=loadItems(arr);
    sortItems(arr,n);
    for(int i=0;i<n;i++){
        printf("%d | %s | %s | %s | %s | %s | %d\n",
        arr[i].id,arr[i].user,arr[i].item,arr[i].type,
        arr[i].desc,arr[i].time,arr[i].status);
    }
}

void safeInput(char *buf,int size){
    fgets(buf,size,stdin);
    buf[strcspn(buf,"\n")]=0;
}

void addItem(char* user){
    Item arr[200];
    int n=loadItems(arr);
    Item it;
    it.id = (n==0)?1:arr[n-1].id+1;

    strcpy(it.user,user);

    printf("Item: "); getchar(); safeInput(it.item,100);
    printf("Type: "); safeInput(it.type,50);
    printf("Desc: "); safeInput(it.desc,200);

    time_t t=time(NULL);
    strcpy(it.time,ctime(&t));
    it.status=0;

    arr[n]=it;
    saveItems(arr,n+1);
    printf("Saved ID: %d\n",it.id);
}

void searchItem(int id){
    Item arr[200];
    int n=loadItems(arr);
    sortItems(arr,n);

    int l=0,r=n-1;
    while(l<=r){
        int m=(l+r)/2;
        if(arr[m].id==id){
            printf("Found: %s (%s)\n",arr[m].item,arr[m].user);
            return;
        } else if(arr[m].id<id) l=m+1;
        else r=m-1;
    }
    printf("Not found\n");
}

void markFound(int id){
    Item arr[200];
    int n=loadItems(arr);

    for(int i=0;i<n;i++){
        if(arr[i].id==id){
            arr[i].status=1;

            FILE *fp=fopen(MSG_FILE,"a");
            fprintf(fp,"%s|Your item ID %d has been FOUND\n",arr[i].user,id);
            fclose(fp);
        }
    }
    saveItems(arr,n);
}

void deleteItem(int id){
    Item arr[200];
    int n=loadItems(arr);

    for(int i=0;i<n;i++){
        if(arr[i].id==id){
            for(int j=i;j<n-1;j++)
                arr[j]=arr[j+1];
            n--;
            break;
        }
    }
    saveItems(arr,n);
}

void editItem(int id){
    Item arr[200];
    int n=loadItems(arr);

    for(int i=0;i<n;i++){
        if(arr[i].id==id){
            printf("New Item: "); getchar(); safeInput(arr[i].item,100);
            printf("New Type: "); safeInput(arr[i].type,50);
            printf("New Desc: "); safeInput(arr[i].desc,200);
        }
    }
    saveItems(arr,n);
}
