#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/un.h>
#include <signal.h>
#include <sys/socket.h>
#include "image_cache.h"
#include "image_queue.h"

#define OPEN_MAX 128
#define CACHE_FIND 0
#define CACHE_INSERT 1

int fd;

void handle(int s){
	close(fd);
	unlink("/opt/cache_all.sock");
	exit(0);
}

int readn(int fd, char *buf, int n){
	int len = 0, r;
	while(len < n){
		r = read(fd, buf+len, n-len);
		if(r < 0){
			return -1;
		}
		len += r;
	}
	return 0;
}

int writen(int fd, char *buf, int n){
	int len = 0, r;
	while(len < n){
		r = write(fd, buf+len, n-len);
		if(r < 0){
			return -1;
		}
		len += r;
	}
	return 0;
}

int main(){
	signal(SIGINT, handle);
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
	r = listen(fd, 128);
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

	mInitPool();
	struct mHashTable *g_H = mInitHash(20011, 1<<29);//100003
	init_imgq();

	int sizeb;
	int sizea;
	char bufb[1048576];
	char bufa[1048576];
	struct mData *tmp;
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
				char type;
				r = read(fds[i].fd, &type, 1);
				if(r <= 0){
closefd:
					close(fds[i].fd);
					fds[i].fd = -1;
					if(r)
						perror("read error!");
					else
						printf("fd %d close connection!\n", fds[i].fd);
					continue;
				}
				switch(type){
					case CACHE_FIND:
						{
							if(readn(fds[i].fd, (char*)&sizeb, 4) < 0)
								goto closefd;
							if(readn(fds[i].fd, bufb, sizeb) < 0)
								goto closefd;
							tmp = mFind(bufb, sizeb, g_H);
							if(tmp){
								if(writen(fds[i].fd, (char*)&tmp->s, 4) < 0)
									goto closefd;
								if(writen(fds[i].fd, (char*)&tmp->jpglen, 4) < 0)
									goto closefd;
								if(writen(fds[i].fd, (char*)tmp->jpg, tmp->jpglen) < 0)
									goto closefd;
							}else{
								int s = 0;
								if(writen(fds[i].fd, (char*)&s, 4) < 0)
									goto closefd;
							}
						}
						break;
					case CACHE_INSERT:
						{
							if(readn(fds[i].fd, (char*)&sizeb, 4) < 0)
								goto closefd;
							if(readn(fds[i].fd, bufb, sizeb) < 0)
								goto closefd;
							if(readn(fds[i].fd, (char*)&sizea, 4) < 0)
								goto closefd;
							if(readn(fds[i].fd, bufa, sizea) < 0)
								goto closefd;
							int s = imgq_deque();
							mInsert(s, bufb, bufa, sizeb, sizea, g_H);
						}
						break;
				}
			}
		}
		n = t;
	}
}
