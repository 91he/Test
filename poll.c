#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/un.h>
#include <signal.h>
#include <sys/socket.h>

#define OPEN_MAX 128

int fd;

void handle(int s){
	close(fd);
	unlink("/opt/cache_all.sock");
	exit(0);
}

int main(){
	fd = socket(PF_LOCAL, SOCK_STREAM, 0);
	if(fd == -1){
		perror("socket error!");
		return -1;
	}
	struct sockaddr_un addr;
	addr.sun_family = PF_LOCAL;
	strcpy(addr.sun_path, "/opt/cache_all.sock");
	int r = bind(fd, (struct sockaddr*)&addr, sizeof(addr));
	if(r == -1){
		close(fd);
		perror("bind error!");
		return -1;
	}
	r = listen(fd, 10);
	if(r == -1){
		close(fd);
		perror("listen error!");
		return -1;
	}

	struct pollfd fds[OPEN_MAX];
	fds[0].fd = fd;
	fds[0].events = POLLIN;
	int i, n = 0, msgnum = 0;
	for(i = 1; i < OPEN_MAX; i++){
		fds[i].fd = -1;
	}

	while(1){
		msgnum = poll(fds, n+1, -1);
		if(msgnum == -1){
			perror("poll error");
		}
		if(fds[0].revents & POLLIN){
			r = accept(fds[0].fd, NULL, NULL);
			if(r != -1){
				for(i = 1; i < OPEN_MAX; i++){
					if(fds[i].fd == -1){
						fds[i].fd = r;
						fds[i].events = POLLIN;
						break;
					}
				}
				if(i == OPEN_MAX){
					printf("to much clients!");
					close(r);
				}
				if(i > n){
					n = i;
				}
			}
			msgnum--;
		}
		int t;
		for(i = 1; i <= n && msgnum; i++){
			if(fds[i].fd == -1)
				continue;
			t = i;
			if(fds[i].revents&(POLLIN|POLLERR)){
				msgnum--;
				char buf[32];
				r = read(fds[i].fd, buf, 32);
				if(r <= 0){
					close(fds[i].fd);
					fds[i].fd = -1;
					if(r)
						perror("read error!");
					else
						printf("fd %d close connection!\n", fds[i].fd);
					continue;
				}
				buf[r] = 0;
				printf("%s\n", buf);
				write(fds[i].fd, "world", 5);
			}
		}
		n = t;
	}
}
