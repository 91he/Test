#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

int lengthOfLongestSubstring(char* s) {
    unsigned char *tmp = s;
    unsigned char *os = tmp;
    int start, end , len, max;
    bool arr[256];
    int index[256];

    for(int i = 0; i < 256; ++i) arr[i] = false;

    start = end = len = max = 0;
    while(*tmp){
        end++;
        if(arr[*tmp] && index[*tmp] > start){
            if(index[*tmp] - start > 1){
                for(int i = start; i < index[*tmp]; ++i){
                    arr[os[start]] = false;
                }
            }
            start = index[*tmp];
            printf("%s:%d\n", tmp, start);
        }else{
            printf("%s:%d:%d\n", tmp, end, start);
            arr[*tmp] = true;
            len = end - start;
            max = len > max ? len : max;
        }
        index[*tmp] = end;
        tmp++;
    }

    return max;
}

double findMedianSortedArrays(int* nums1, int nums1Size, int* nums2, int nums2Size) {
    int n = 0;
    double ret;
    int *nums = malloc((nums1Size + nums2Size) * sizeof(int));

    int i = 0, j = 0;
    do{
        if(i < nums1Size && j < nums2Size){
            if(nums1[i] < nums2[j]){
                nums[n++] = nums1[i];
                i++;
            }else{
                nums[n++] = nums2[j];
                j++;
            }
        }else{
            for(; i < nums1Size; ++i){
                nums[n++] = nums1[i];
            }

            for(; j < nums2Size; ++j){
                nums[n++] = nums2[j];
            }
            break;
        }
    }while(true);

    if(n % 2){
        ret = nums[n / 2];
    }else{
        ret = (nums[n / 2] + nums[n / 2 - 1]) / 2.0 ;
    }

    free(nums);

    return ret;
}

char *longestPalindrome(char *s){
    int start, maxLen;
    int left, right, len;
    static char ret[1024];
    char *ptr;//, *prev, *next, *end;

    len = strlen(s);
    start = 0;
    maxLen = 1;
    /*
    ptr = s + 1;
    end = ptr + strlen(s);
    */

    for(int i = 0; i < len && (len - i) * 2 > maxLen;){
        left = right = i;
        while(right < len && s[right] == s[right + 1]) right++;
        i = right + 1;

        while(left > 0 && right < len && s[left - 1] == s[right + 1]){
            left--;
            right++;
        }

        int tmp = right - left + 1;
        if(tmp > maxLen){
            start = left;
            maxLen = tmp;
        }
    }
/*
    while(*ptr){
        if(*(ptr - 1) == *(ptr +1)){
            prev = ptr - 1;
            next = ptr + 1;

            while(prev > s && *next){
                if(*--prev != *++next){
                    ++prev;
                    --next;
                    break;
                }
            }

            int tmp = next - prev + 1;
            if(tmp > maxLen){
                maxLen = tmp;
                start = prev - s;
            }
        }

        if(*ptr == *(ptr-1)){
            prev = ptr - 1;
            next = ptr;

            while(prev > s && *next){
                if(*--prev != *++next){
                    ++prev;
                    --next;
                    break;
                }
            }

            int tmp = next - prev + 1;
            if(tmp > maxLen){
                maxLen = tmp;
                start = prev - s;
            }
        }

        if((end - ptr) * 2 <= maxLen) break;
        ptr++;
    }

    ptr = ret;

    //printf("%d:%d\n", start, maxLen);
    for(int i = start; i < start + maxLen; i++){
        *ptr++ = s[i];
    }
    *ptr = 0;
    */
    strncpy(ret, s + start, maxLen);
    ret[maxLen] = 0;

    return ret;
}

char* convert(char* s, int numRows) {
    int i, j, x, len, tabs;
    int *nums;
    char **lines, *ret;

    if(numRows == 1) return s;

    len = strlen(s);
    if(len <= numRows) return s;

    tabs = numRows + numRows - 2;
    x = len / tabs;

    ret = malloc(len + 1);
    lines = malloc(sizeof(char*) * numRows);
    nums = malloc(sizeof(int) * numRows);

    nums[0] = x;
    for(i = 1; i < numRows - 1; i++){
        nums[i] = x * 2;
    }
    nums[numRows - 1] = x;

    int y = len % tabs;
    i = 0;
    j = 1;
    while(y--){
        nums[i]++;
        i += j;
        if(i == 0 || i == numRows -1){
            j = -j;
        }
    }

    lines[0] = ret;
    for(i = 1; i < numRows; i++){
        lines[i] = lines[i - 1] + nums[i - 1];
    }

#if 0
    printf("%d\n", numRows);
    for(i = 0; i < numRows; i++){
        printf("%d\n", nums[i]);
    }
#endif

    x = 0;
    for(i = 0, j = 1; i < len; i++){
        *lines[x] = s[i];
        ++lines[x];
        x += j;
        if(x == 0 || x == numRows - 1){
            j = -j;
        }
    }
    ret[len] = 0;

    return ret;
}

int reverse(int x) {
    long long ret = 0LL;
    int min = 1 << 31; 
    int max = ~min;

    if(!x) return 0;

    do{
        ret = ret * 10 + x % 10;
        printf("%ld\n", ret);
        x /= 10;
    }while(x);

    if(ret < min || ret > max) return 0;

    return ret;
}

bool isPalindrome(int x) {
    int y = 0;

    if(x > 0){
        do{
            y = y * 10 + x % 10;
            x /= 10;
            if(x == y || x / 10 == y) return true;
        }while(x && x > y);
    }else{
        do{
            y = y * 10 + x % 10;
            x /= 10;
            if(x == y || x / 10 == y) return true;
        }while(x && x < y);
    }

    return false;
}

char* dec(char *str){
    int i, j;
    for(i = 0, j = 0; str[j]; ++i){
        while(str[j] && str[j] == str[j + 1])
            ++j;
        str[i] = str[j++];
    }
    str[i] = 0;

    return str;
}

void calc(char *reg, int *start, int *end, int *total){
    //char *org = reg;

    *start = *end = *total = 0;
    while(reg[0] == '.' && reg[1] != '*'){
        ++(*start);
        ++(*total);
        ++reg;
    }

    char *tmp = reg + strlen(reg);
    while(tmp-- > reg){
        if(*tmp != '.')
            break;

        ++(*end);
        ++(*total);
    }

    while(reg <= tmp){
        if(reg[1] == '*'){
            reg += 2;
        }else{
            ++(*total);
            ++reg;
        }
    }
}

char* decReg(char *str){
    int i, j;

    for(i = 0, j = 0; str[j]; ++i){
        while(str[j] && str[j] == str[j + 2])
            j += 2;
        str[i] = str[j];
        j += 2;
    }
    str[i] = 0;

    return str;
}

bool ainb(char *a, char *b){
#if 0
    int s, e, t, lens;

    lens = strlen(a);
    calc(b, &s, &e, &t);

    if(t == lens){
        return true;
    }else if(t > lens){
        return false;
    }
    
    if(t == 0){
        a = dec(a);
        b= decReg(b);
#endif
        while(*b){
            if(*b == '.') return true;

            if(*b == *a) ++a;

            if(!*a) return true;
            ++b;
        }
    //}

    return false;
}

int myncmp(char *a, char *b, int n){
    while(n--){
        if(!*b || *a != '.')
            if(*a != *b)
                return *a - *b;
        ++a;
        ++b;
    }

    return 0;
}

char *mystrstr(char *str, char *sub){
    bool has = false;
    int n = 0;;
    char *tmp = sub;

    while(*tmp){
        ++n;
        if(*tmp++ == '.')
            has = true;
    }

    if(!has) return strstr(str, sub);

    do{
        printf("%s:%s\n", str, sub);
        if(!myncmp(sub, str, n)){
            return str;
        }
    }while(*str++);

    return NULL;
}

bool isMatch(char* s, char* p) {
    int i = 0, lens, lenp;
    int x = 0, y = 0, z = 0;

    if(!*s && !*p) return true;

    lens = strlen(s);
    lenp = strlen(p);
    char *ts = malloc(lens + 1);
    char *tpx = malloc(lenp + 1);
    char *tpy = malloc(lenp + 1);

    while(p[y]){
        if(p[y + 1] != '*'){
#if 0
            if(p[y] == '.'){
                ++y;
                continue;
            }
#endif
            z = y + 1;
            while(p[z] && p[z + 1] != '*') ++z;
            int rLen = y - x;
            int sLen = z - y;

            strncpy(&tpx[x], &p[x], y - x);
            tpx[y] = 0;
            strncpy(&tpy[y], &p[y], z - y);
            tpy[z] = 0;

            //printf("%s:%s\n", &tpx[x], &tpy[y]);
            if(!sLen){
                strcpy(&ts[i], &s[i]);
                return ainb(dec(&ts[i]), decReg(&tpx[x]));
            }else{
                if(!rLen){
                    //if(strncmp(&tpy[y], &s[i], sLen)) return false;
                    if(myncmp(&tpy[y], &s[i], sLen)) return false;
                    i += sLen;
                }else{
                    int j = i;
                    char *tmp, *regTmp = decReg(&tpx[x]);
                    //printf("%s:%s\n", &s[j], &tpy[y]);
                    //printf("%s#%s\n", s, p);
                    //printf("%d^%d\n", i, z);
                    //while(tmp = strstr(&s[j], &tpy[y])){
                    while(tmp = mystrstr(&s[j], &tpy[y])){
                        int n = tmp - &s[i];
                        strncpy(ts, &s[i], n);
                        ts[n] = 0;

                        //printf("%s=%s\n", ts, &tpx[x]);
                        if(ainb(dec(ts), regTmp)){
                            //printf("%s#%s\n", &tmp[sLen], &p[z]);
                            //if(isMatch(&s[j + n], &p[z]))
                            if(isMatch(&tmp[sLen], &p[z]))
                                return true;
                        }
                        ++j;
                    };

                    return false;
                }
            }

            x = y = z;
        }else{
            y += 2;
        }
    }

    if(y >= z){
        if(!s[i]) return true;

        strcpy(&ts[i], &s[i]);
        strncpy(&tpx[x], &p[x], y - x);
        tpx[y] = 0;

        //printf("%s:%s\n", &ts[i], &tpx[x]);
        return ainb(dec(&ts[i]), decReg(&tpx[x]));
    }

    free(ts);
    free(tpx);
    free(tpy);

    return false;
}

int main(){
#if 0
    int arr[] = {1, 2};
    int arr1[] = {3, 4};
    printf("%f\n", findMedianSortedArrays(arr, 2, arr1, 2));
    printf("%d\n", lengthOfLongestSubstring("abcabcbb"));
    printf("%d\n", lengthOfLongestSubstring("eeydgwdykpv"));
    printf("%d\n", lengthOfLongestSubstring("aleiivuuxszpaqojv"));

    printf("%s\n", longestPalindrome("babad"));
    printf("%s\n", longestPalindrome("cbbd"));
    printf("%s\n", longestPalindrome("ccc"));
    printf("%s\n", longestPalindrome("aaaaaaabbbbbbbbbb"));
    printf("%s\n", longestPalindrome("abbaabbacabbaabba"));

    printf("%s\n", convert("PAYPALISHIRING", 3));
    printf("%s\n", convert("abcde", 4));
    printf("%s\n", convert("abcd", 4));
    printf("%s\n", convert("abcde", 1));

    printf("%d\n", reverse(-1234567822));

    printf("%d\n", isPalindrome(-12321));
    printf("%d\n", isPalindrome(12322));
#endif
#if 0
    printf("%d\n", ainb("ab", "."));

    char *str = strdup("aaaabbcdeefg");
    printf("%s\n", dec(str));

    str = strdup("a*a*a*a*b*b*c*d*e*e*.*f*g*");
    printf("%s\n", decReg(str));
#endif

#if 0
    printf("%d\n", isMatch("aa", "a"));
    printf("%d\n", isMatch("aa", "aa"));
    printf("%d\n", isMatch("aaa", "aa"));
    printf("%d\n", isMatch("aa", "a*"));
    printf("%d\n", isMatch("aa", ".*"));
    printf("%d\n", isMatch("ab", ".*"));
    printf("%d\n", isMatch("aab", "c*a*b*"));
    printf("%d\n", isMatch("aab", "c*.*b*"));
    printf("%d\n", isMatch("aab", "c*.*ab*"));
    printf("%d\n", isMatch("aab", "c*.*ab*b"));
    printf("%d\n", isMatch("ab", ".*.."));
#endif
    //printf("%d\n", myncmp("a.aa", "aaa", 3));
    //printf("%d\n", isMatch("bbbb", ".c*."));
    printf("%d\n", isMatch("", "..*"));

    return  0;
}
