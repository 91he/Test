#include <stdio.h>

typedef unsigned int T;

T mod_mul(T a, T b, T n) {
	T res = 0;
	while(b) {
		if(b&1)    
			res = (res + a) % n;
		a = (a + a) % n;
		b >>= 1;
	}
	return res;
}

T mod_exp(T a, T b, T n){
	T res = 1;
	while(b){
		if(b&1)
			res = mod_mul(res, a, n);
		a = mod_mul(a, a, n);
		b >>= 1;
	}
	return res;
}

int main(){
	int r = mod_exp(2, 102, 7);
	printf("%d\n", r);
	return 0;
}
