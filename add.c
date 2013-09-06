#include <stdio.h>

int add(a, b){
	if(b) 
		return add(a^b, (a&b)<<1);
	return a;
}

int _fun(int a, int b, int *x, int *y){
	if(b == 0){
		*x = 1;
		*y = 0;
		return a;
	}
	int r = _fun(b, a%b, x, y);
	int t = *x;
	*x = *y;
	*y = t-*y*(a/b);
	return r;
}

int fun(int a, int b, int *x, int *y){
	if(a > b)
		return _fun(a, b, x, y);
	return _fun(b, a, x, y);
}

int main(){
	//printf("%d\n", add(11, 6));
	int a = 6, b = 4;
	int x, y;
	int r = fun(4, 6, &x, &y);
	printf("%d*%d+%d*%d=%d\n", a, x, b, y, r);
	return 0;
}
