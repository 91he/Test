#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define N 10

typedef struct Mat{
	int mat[N][N];
}Mat;

Mat mat_mul(Mat a, Mat b){
	static Mat ret;
	int i, j, k;

	bzero(ret.mat, sizeof(Mat));

	for(i = 0; i < N; ++i){
		for(j = 0; j < N; ++j){
			if(!a.mat[j][i]) continue;
			for(k = 0; k < N; ++k){
				if(!b.mat[i][k]) continue;
				//ret.mat[j][k] += (a.mat[j][i]%10007)*(b.mat[i][k]%10007)%10007;
				ret.mat[j][k] += a.mat[j][i]*b.mat[i][k];
			}
		}
	}

	return ret;
}

Mat mat_exp(Mat a, unsigned int n){
	static Mat ret;
	int i, j;

	bzero(ret.mat, sizeof(Mat));

	for(i = 0; i < N; i++){
		for(j = 0; j < N; j++){
			ret.mat[i][j] = (i == j);
		}
	}

	while(n){
		if(n&1)
			ret = mat_mul(ret, a);
		a = mat_mul(a, a);
		n >>= 1;
	}

	return ret;
}

int main(){
	Mat a, b, c;
	bzero(a.mat, sizeof(Mat));
	bzero(b.mat, sizeof(Mat));
	a.mat[0][0] = 1;
	a.mat[0][1] = 1;
	a.mat[1][0] = 1;
	a.mat[1][1] = 0;
	b.mat[0][0] = 1;
	b.mat[1][0] = 0;
	a = mat_exp(a, 30);
	c = mat_mul(a, b);
	printf("%ld %ld\n", c.mat[0][0], c.mat[0][0]%10007);
	//printf("%d %d\n%d %d\n", a.mat[0][0], a.mat[0][1], a.mat[1][0], a.mat[1][1]);
	return 0;
}
