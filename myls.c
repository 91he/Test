#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#define MAX_WIDTH 100

void swapstr(char *a, char *b){
	char c[MAX_WIDTH+1];
	strcpy(c, a);
	strcpy(a, b);
	strcpy(b, c);
}

void fsort(char (*arr)[MAX_WIDTH+1], int len){
	if(len <= 1) return;
	int i = 0, j = len-1;
	while(i < j){
		while(i < j && strcmp(arr[i], arr[j]) < 0){
			--j;
		}
		swapstr(arr[i], arr[j]);
		while(i < j && strcmp(arr[i], arr[j]) < 0) ++i;
		swapstr(arr[i], arr[j]);
	}
	fsort(arr, i);
	fsort(arr+i+1, len-i-1);
}

int my_ls(char *path){
	char strall[100][MAX_WIDTH + 1];
	char strdst[100][MAX_WIDTH + 1];
	bzero(strdst, (MAX_WIDTH + 1)*100);
	int i, j, r, c, n, maxlen = 0;
	struct winsize size;
	struct dirent *filedir;

	if(isatty(STDOUT_FILENO) == 0)
		return -1;
	if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) < 0){
		perror("Can't get window size");
		return -1;
	}
	int SCREEN_WIDTH = size.ws_col;
	
	DIR *dir = opendir(path);

	i = errno;
	r = 0;
	while(filedir = readdir(dir)){
		if(filedir->d_name[0] == '.'){
			if(filedir->d_name[1] == 0)
				continue;
			else if(filedir->d_name[1] == '.' && filedir->d_name[2] == 0)
				continue;
		}
		strcpy(strall[r++], filedir->d_name);
	}
	if(i != errno)
		return -1;

	n = r;
	unsigned char *len_arr = malloc(n);

	bzero(len_arr, r);
	fsort(strall, n);

	for(i = 0; i < n; ++i){
		j = len_arr[i] = strlen(strall[i]);
		j = j > SCREEN_WIDTH ? SCREEN_WIDTH : j;
		maxlen = j > maxlen ? j : maxlen;
	}

	maxlen += 2;
	/*****************************/
	unsigned char *tmp_arr = malloc(n);
	int len_t;
	for(i = 1;;++i){
		bzero(tmp_arr, n);
		len_t = 0;
		for(j = 0; j < n; ++j){
			tmp_arr[j/i] = tmp_arr[j/i] >= len_arr[j]
				? tmp_arr[j/i] : len_arr[j];
		}
		c = i;
		r = n/c + (n%c != 0);
		for(j = 0; j < r; ++j){
			len_t += tmp_arr[j] + 2;
		}
		if(len_t <= SCREEN_WIDTH + 2)  break;
	}
	/*****************************/
	for(i = 0; i < c; ++i){
		for(j = 0; j < r-1; ++j){
			if(j*c+i < n)
				printf("%-*s", tmp_arr[j]+2, strall[j*c+i]);
		}
		if(j*c+i < n)
			printf("%s", strall[j*c+i]);
		printf("\n");
	}
	return 0;
}

int main(){
	my_ls(".");
	return 0;
}
