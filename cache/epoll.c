#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/un.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include "image_cache.h"
#include "image_queue.h"

#define MAX_EVENTS 128
#define CACHE_FIND 0
#define CACHE_INSERT 1

int listen_sock;
struct mHashTable *g_H = NULL;

void handle(int s){
	close(listen_sock);
	unlink("/opt/cache_all.sock");
	exit(0);
}
void setnonblocking(int sock)
{
	int opts;
	opts=fcntl(sock,F_GETFL);
	if(opts<0)
	{
		perror("fcntl(sock,GETFL)");
		exit(EXIT_FAILURE);
	}
	opts = opts|O_NONBLOCK;
	if(fcntl(sock,F_SETFL,opts)<0)
	{
		perror("fcntl(sock,SETFL,opts)");
		exit(EXIT_FAILURE);
	}
}

int readn(int fd, char *buf, int n){
	int len = 0, r;
	while(len < n){
		r = read(fd, buf+len, n-len);
		if(r == -1){
			if(errno == EAGAIN)
				continue;
			perror("read error!");
			return -1;
		}
		else if(!r){
			printf("connection closed!\n");
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
		if(r == -1){
			if(errno == EAGAIN) continue;
			perror("write error!");
			return -1;
		}
		else if(!r){
			printf("connection closed!\n");
			return -1;
		}
		len += r;
	}
	return 0;
}

int do_use_fd(int fd){
	char type;
	int sizeb;
	int sizea;
	char bufb[1048576];
	char bufa[1048584];
	if(readn(fd, &type, 1) < 0){
		return -1;
	}
	struct mData *tmp = NULL;
	switch(type){
		case CACHE_FIND:
			{
				if(readn(fd, (char*)&sizeb, 4) < 0)
					return -1;
				if(readn(fd, bufb, sizeb) < 0)
					return -1;
				tmp = mFind(bufb, sizeb, g_H);
				if(tmp){
					memcpy(bufa, (char*)&tmp->s, 4);
					memcpy(bufa+4, (char*)&tmp->jpglen, 4);
					memcpy(bufa+8, tmp->jpg, tmp->jpglen);
					if(writen(fd, bufa, tmp->jpglen+8) < 0)
						return -1;
				}else{
					int s = 0;
					if(writen(fd, (char*)&s, 4) < 0)
						return -1;
				}
			}
			break;
		case CACHE_INSERT:
			{
				if(readn(fd, (char*)&sizeb, 4) < 0)
					return -1;
				if(readn(fd, bufb, sizeb) < 0)
					return -1;
				if(readn(fd, (char*)&sizea, 4) < 0)
					return -1;
				if(readn(fd, bufa, sizea) < 0)
					return -1;
				//tmp = mFind(bufb, sizeb, g_H);
				int s;
				/*
				   if(tmp){
				   s = tmp->s;
				   }else{
				 */
				s = imgq_deque();
				mInsert(s, bufb, bufa, sizeb, sizea, g_H);
				//}
				if(writen(fd, (char*)&s, 4) < 0)
					return -1;
			}
			break;
	}
}

int main(){
	signal(SIGINT, handle);
	listen_sock = socket(PF_LOCAL, SOCK_STREAM, 0);
	if(listen_sock == -1){
		perror("socket error!");
		exit(EXIT_FAILURE);
	}
	struct sockaddr_un addr;
	addr.sun_family = PF_LOCAL;
	strcpy(addr.sun_path, "/opt/cache_all.sock");
	int r = bind(listen_sock, (struct sockaddr*)&addr, sizeof(addr));
	if(r == -1){
		close(listen_sock);
		perror("bind error!");
		exit(EXIT_FAILURE);
	}
	r = listen(listen_sock, MAX_EVENTS);
	if(r == -1){
		close(listen_sock);
		perror("listen error!");
		exit(EXIT_FAILURE);
	}

	mInitPool();
	g_H = mInitHash(20011, 1<<29);//100003
	init_imgq();

	struct epoll_event ev, events[MAX_EVENTS];
	int conn_sock, nfds, epollfd;

	/* Set up listening socket, 'listen_sock' (socket(),
	   bind(), listen()) */

	epollfd = epoll_create(10);
	if (epollfd == -1) {
		perror("epoll_create");
		exit(EXIT_FAILURE);
	}

	ev.events = EPOLLIN;
	ev.data.fd = listen_sock;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
		perror("epoll_ctl: listen_sock");
		exit(EXIT_FAILURE);
	}
	int n;

	for (;;) {
		nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
		if (nfds == -1) {
			perror("epoll_pwait");
			exit(EXIT_FAILURE);
		}

		for (n = 0; n < nfds; ++n) {
			if (events[n].data.fd == listen_sock) {
				//conn_sock = accept(listen_sock, (struct sockaddr *) &local, &addrlen);
				conn_sock = accept(listen_sock, NULL, NULL);
				if (conn_sock == -1) {
					perror("accept");
					exit(EXIT_FAILURE);
				}
				setnonblocking(conn_sock);
				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = conn_sock;
				if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock,
							&ev) == -1) {
					perror("epoll_ctl: conn_sock");
					exit(EXIT_FAILURE);
				}
			} else {
				if(do_use_fd(events[n].data.fd) == -1)
					close(events[n].data.fd);
			}
		}
	}
}
