#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

int cmp_str(const void *a, const void *b){
    //printf("%s, %s\n", *(char**)a, *(char**)b);
    //return strcmp(*(char * const *)a, *(char * const *)b);
    //printf("%s, %s\n", (char*)a, (char*)b);
    return strcmp((char*)a, (char*)b);
}

int cmp_int(void *a, void *b){
    return ((int)a) - ((int)b);
}

void sort(void **arr, int n, int (*func)(const void*, const void*)){
    if(n <= 1) return;

    int i = 0;
    int j = n - 1;
    void *tmp = arr[0];
    
    while(i < j){
        while(i < j && func(tmp, arr[j]) < 0) j--;
        arr[i] = arr[j];

        while(i < j && func(arr[i], tmp) < 0) i++;
        arr[j] = arr[i];
    }
    arr[i] = tmp;

    sort(arr, i++, func);
    sort(arr + i, n - i, func);
}

int main(){
    char *word[] = {"hello", "think", "test", "haha", "nihao", "womenshi", "haoren", "dajia", "youshenmehaozhuyi", "wanquan",
                    "dingxianghua", "xiaoyuanshenghuo", "zhengju", "anhui", "beijing"};

    //qsort(word, sizeof(word) / sizeof(word[0]), sizeof(char*), cmp_str);
    int sw = 44;
    int count = sizeof(word) / sizeof(word[0]);
    sort((void**)word, count, cmp_str);

    int i, j;
    int row = 0;
    int col, len;
    int *num = malloc(count * sizeof(int));
    do{
        row++;
        int t = count % row ? 1 : 0;
        col = count / row + t;
        bzero(num, col * sizeof(int));
        for(i = 0; i < row; i++){
            len = 0;
            for(j = 0; j < col; j++){
                if((j * row + i) >= count) continue;
                int t = strlen(word[j * row + i]) + 1;
                num[j] = num[j] >= t ? num[j] : t;
            }
        }
        if(col == 1) break;
        for(i = 0; i < col; i++) len += num[i];
    }while(len - 1 > sw);
    
    for(i = 0; i < row; i++){
        for(j = 0; j < col; j++){
            if((j * row + i) >= count) continue;
            printf("%-*s", num[j], word[j * row + i]);
        }
        printf("\n");
    }
    free(num);

    return 0;
}
