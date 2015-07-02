#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int pwd_check(const char *str){
	int r = 1;
	char *PWD_STR = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ~`!@#$%^&*_-+=|\\{[}]:;\"'<,>.?/";
	char *tmp = (char*)str;
	
	while(*tmp){
		if(!strchr(PWD_STR, *tmp)){
			r = 0;//格式
			break;
		}
		tmp++;
	}

//	if(tmp - str > 16)
//		r = 2;//长度

	return r;
}

int mail_check(const char *str){
	int r = 0;
	char *end;
	char *front = strdup(str);
	char *tmp = front;
	
	do{
		if(!(end = strchr(front, '@'))){
			break;
		}

		*end++ = 0;

		if(*tmp == '_' || !*tmp){
			break;
		}

		while(*tmp){
			if(!isdigit(*tmp) && !isalpha(*tmp) && '_' != *tmp){
				tmp = NULL;
				break;
			}
			tmp++;
		}
		if(!tmp) break;

		tmp = end;
		
		while(*tmp){
			if(!isdigit(*tmp) && !isalpha(*tmp) && *tmp != '.'){
				tmp = NULL;
				break;
			}
			tmp++;
		}
		if(!tmp) break;

		if(!(tmp = strchr(end, '.'))){
			break;
		}

		do{
			if(!(tmp - end) || !tmp[1] || '.' == tmp[1]){
				end = NULL;
				break;
			}
			end = ++tmp;
		}while(tmp = strchr(end, '.'));
		if(!end) break;

		r = 1;
	}while(0);

	free(front);

	return r;
}
#if 0
int main(){
	char *PWD_STR = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ~`!@#$%^&*_-+=|\\{[}]:;\"'<,>.?/";

	if(pwd_check(PWD_STR)){
		printf("hello\n");
	}else{
		printf("bad!!!!\n");
	}

	//char *mail = "91he@sina.com";
	char *mail = "test@sina.com";

	if(mail_check(mail)){
		printf("right!\n");
	}else{
		printf("!!!!!!\n");
	}
	return 0;
}
#endif
