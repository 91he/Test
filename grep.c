#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

int main(int argc, char **argv){
	int status, i, r;
	size_t n;
	int cflag = REG_EXTENDED|REG_NEWLINE;
	regmatch_t match;
	regex_t reg;

	char *buf = NULL;

	if(argc >= 2){
		regcomp(&reg, argv[1], cflag);
		while((r = getline(&buf, &n, stdin) != -1)){
			char *tmp = buf;
			bzero(&match, sizeof(match));
			while(1){
				tmp += match.rm_eo;
				status = regexec(&reg, tmp, 1, &match, 0);
				if(status == REG_NOMATCH)
					break;
				else if(status == 0){
					for(i = 0; i < match.rm_so; ++i){
						putchar(tmp[i]);
					}
					printf("\033[1;31m");
					for(i = match.rm_so; i < match.rm_eo; ++i){
						putchar(tmp[i]);
					}
					printf("\033[0m");
				}
			}
			if(tmp != buf) printf("%s", tmp);
			free(buf);
			buf = NULL;
		}
		regfree(&reg);
	}
	return 0;
}
