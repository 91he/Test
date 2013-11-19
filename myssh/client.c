#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "myls.h"

#define SSH_GET 0
#define SSH_LS 1
#define SSH_PWD 2
#define SSH_CD 2

int cfd;

void handle(int a){
	close(cfd);
	exit(0);
}

int main(){
	signal(SIGINT, handle);
	cfd = socket(PF_INET, SOCK_STREAM, 0);
	if(cfd == -1){
		perror("Socket");
		return -1;
	}
	struct sockaddr_in addr;
	addr.sin_family = PF_INET;
	addr.sin_port = htons(2222);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	int r = connect(cfd, (struct sockaddr*)&addr, sizeof(addr));
	if(r == -1){
		close(cfd);
		perror("Connect");
		//return -1;
	}
	char *buf = NULL;
	size_t len = 0;
	char cmd;
	while(getline(&buf, &len, stdin) != -1){
		if(!strncmp(buf, "ls", 2)){
			cmd = SSH_LS;
		}else if(!strncmp(buf, "pwd", 3)){
			cmd = SSH_PWD;
		}
		write(cfd, &cmd, 1);
		switch(cmd){
			case SSH_LS:
				if(ls_for_ssh(cfd) < 0)
					return -1;
				break;
			case SSH_PWD:
				{
					unsigned int len = 0;
					char path[1024];
					if(read(cfd, &len, sizeof(len)) <= 0)
						return -1;
					if(read(cfd, path, len) <= 0)
						return -1;
					printf("%s\n", path);
				}
				break;
		}
		free(buf);
		buf = NULL;
	}
	close(cfd);
	return 0;
}
