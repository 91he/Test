#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#define max(a, b) ((a) > (b) ? (a) : (b))

void reverse(char *dst, const char *src){
    int len = strlen(src);

    dst[len] = 0;
    while(len) dst[--len] = *src++;
}

void recurAdd(char *numA, char *numB, char *numC){
    char *tmpA = numA;
    char *tmpB = numB;
    char *tmpC = numC;
    char *tmpPtr = tmpC;

    *tmpC++ = '0';

    while(*tmpA && *tmpB){
        char tmp = *tmpA + (*tmpB - '0');

        if(tmp - 10 >= '0'){
            *tmpA = tmp - 10;
            *tmpC = '1';
        }else{
            *tmpA = tmp;
            *tmpC = '0';
        }

        tmpA++;
        tmpB++;
        tmpC++;
    }

    if(!*tmpA){
        strcpy(tmpA, tmpB);
        //while(*tmpA++ = *tmpB++);
    }

    while(*tmpPtr && *tmpPtr == '0') tmpPtr++;
    if(!*tmpPtr) return;

    if(*--tmpC == '0') *tmpC = 0;

    recurAdd(numA, numC, numB);
}

void recurAdd2(char *dst, char *numA, char *numB){
    int lenA = strlen(numA);
    int lenB = strlen(numB);
    int tmpLen = max(lenA, lenB) + 2;
    char *tmpA, *tmpB, *tmpC;

    tmpA = malloc(tmpLen);
    tmpB = malloc(tmpLen);
    tmpC = malloc(tmpLen);

    reverse(tmpA, numA);
    reverse(tmpB, numB);
    
    recurAdd(tmpA, tmpB, tmpC);

    reverse(dst, tmpA);

    free(tmpA);
    free(tmpB);
    free(tmpC);
}

char *bigAdd(char *numA, char *numB){
    int lenA = strlen(numA);
    int lenB = strlen(numB);
    int dstLen = max(lenA, lenB) + 2;
    char *numC, *numD, *numE;
    char *dstC, *dstD, *dstE;

    if(lenA < lenB){
        char *tmpPtr;

        tmpPtr = numA;
        numA = numB;
        numB = tmpPtr;
    }

    dstC = malloc(dstLen);
    dstD = malloc(dstLen);
    dstE = malloc(dstLen);

    memset(dstC, 0, dstLen);
    memset(dstD, 0, dstLen);
    memset(dstE, 0, dstLen);
    reverse(dstC, numA);
    reverse(dstD, numB);
    
    do{
        char *tmpPtr;

        numC = dstC;
        numD = dstD;
        numE = dstE;
        tmpPtr = numE;

        *numE = '0';

        while(*numD){
            char tmp;

            ++numE;

            tmp = *numC + *numD - '0';
            if(tmp - 10 >= '0'){
                *numC = tmp - 10;
                *numE = '1';
            }else{
                *numC = tmp;
                *numE = '0';
            }

            ++numC;
            ++numD;
        }
        while(*tmpPtr && *tmpPtr == '0') tmpPtr++;

        if(!*tmpPtr) break;

        tmpPtr = dstD;
        dstD = dstE;
        dstE = tmpPtr;
        
        if(strlen(dstD) > strlen(dstC) && *tmpPtr == '1'){
            tmpPtr = dstC;
            dstC = dstD;
            dstD = tmpPtr;
        }
    }while(1);

    reverse(dstE, dstC);

    free(dstC);
    free(dstD);

    return dstE;
}

void bigAdd2(char *dst, char *a, char *b){
    char *tmp = bigAdd(a, b);

    strcpy(dst, tmp);
    free(tmp);
}

bool isAdditiveNumberByStr(char *num, char *tmpA, char *tmpB, char *tmpC){
    int lenA, lenB, lenC;

    bigAdd2(tmpC, tmpA, tmpB);

    lenA = strlen(tmpA);
    lenB = strlen(tmpB);
    lenC = strlen(tmpC);

    //printf("---%s, %s, %d, %d, %d\n", tmpC, &num[lenA + lenB], lenA, lenB, lenC);
    
    if(strncmp(tmpC, &num[lenA + lenB], lenC)){
        return false;
    }else{
        if(lenA + lenB + lenC == strlen(num))
            return true;
    }

    return isAdditiveNumberByStr(num + lenA, tmpB, tmpC, tmpA);
}

bool isAdditiveNumber(char* num) {
    bool ret = false;
    int len = strlen(num);
    int maxALen = len / 3;
    int maxBLen = len / 2;
    char *tmpA, *tmpB, *tmpC;

    tmpA = malloc(maxBLen);
    tmpB = malloc(maxBLen);
    tmpC = malloc(maxBLen);
    
    if(len < 3) return false;

    for(int i  = 1; i <= maxALen; i++){
        for(int j = 1; j <= maxBLen; j++){
            memcpy(tmpA, num, i);
            tmpA[i] = 0;
            memcpy(tmpB, num + i, j);
            tmpB[j] = 0;
            
            //printf("%s, %s\n", tmpA, tmpB);
            if(isAdditiveNumberByStr(num, tmpA, tmpB, tmpC)){
                ret = true;
                goto end;
            }
        }
    }

end:
    free(tmpA);
    free(tmpB);
    free(tmpC);

    return ret;
}

int main(int argc, char **argv){
    //printf("%d\n", strncmp("100", "100199", 3));
    printf("%d\n", isAdditiveNumber(argv[1]));
    char *dst = malloc(128);

    recurAdd2(dst, "123456789101112131415161718", "123456789101112131415161718");
    printf("%s\n", dst);

    free(dst);

    return 0;
}
