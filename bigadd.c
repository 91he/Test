#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LEN 100

void swap(char *str){
	char *tmp = str+strlen(str)-1;
	char x;
	while(tmp > str){
		x = *tmp;
		*tmp-- = *str;
		*str++ = x;
	}
}

unsigned char *str_add(unsigned char *x, unsigned char *y){
	int m = strlen(x);
	int n = strlen(y);
	int max = m > n ? m : n;
	unsigned char *a = malloc(strlen(x));
	while(m--) a[m] = *x++;
	unsigned char *b = malloc(strlen(y));
	while(n--) b[n] = *y++;
	static unsigned char c[1024];
	memset(c, '0', max+1);
	c[max+1] = 0;
	int i;
	for(i = 0; i < max; i++){
		c[i] += a[i];
		c[i] = c[i] >= 10+2*'0'?c[i]-10-'0':c[i]-'0';
		if(c[i] < a[i]) c[i+1]++;
		c[i] += b[i];
		c[i] = c[i] >= 10+2*'0'?c[i]-10-'0':c[i]-'0';
		if(c[i] < b[i]) c[i+1]++;
	}
	if(c[max] == '0') c[max] = 0;
	free(a);
	free(b);
	swap(c);
	return c;
}

int add(int a, int b){
	return (a+b) >= 1000000000 ? a+b-1000000000 : a+b;
}

unsigned int *big_add(unsigned int *a, unsigned int *b, int n){
	static unsigned int c[LEN+1];
	bzero(c, sizeof(unsigned int)*n);
	int i, t;
	for(i = 0; i < n; i++){
		t = add(c[i], a[i]);
		if(t < c[i] && t < a[i]) c[i+1]++;
		c[i] = t;
		t = add(c[i], b[i]);
		if(t < c[i] && t < b[i]) c[i+1]++;
		c[i] = t;
	}
	return c;
}

print_big(unsigned int *a, int n){
	int i = 1;
	while(n--){
		if(a[n]){
			if(i-- > 0)
				printf("%u", a[n]);
			else
				printf("%09u", a[n]);
		}
	}
	printf("\n");
}

int main(int argc, char **argv){
	unsigned int a[LEN], b[LEN];
	bzero(a, sizeof(unsigned int)*LEN);
	bzero(b, sizeof(unsigned int)*LEN);
	a[0] = 1;
	b[0] = 2;
	unsigned int *c = NULL;
	int i;
	int n = atoi(argv[1]);
	for(i = 0; i < n; i++){
		c = big_add(a, b, LEN);
		memcpy(a, b, sizeof(unsigned int)*LEN);
		memcpy(b, c, sizeof(unsigned int)*LEN);
	}
	print_big(c, LEN);
	printf("%s\n", str_add("1234", "9376"));
	return 0;
}
