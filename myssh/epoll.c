#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MAX_EVENTS 128
#define SSH_GET 0
#define SSH_LS 1
#define SSH_PWD 2
#define SSH_CD 2

int listen_sock;

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
			if(errno == EAGAIN) continue;
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
	size_t size;
	char buf[1024];
	if(readn(fd, &type, 1) < 0){
		return -1;
	}
	switch(type){
		case SSH_PWD:
			{
				char *path = getcwd(NULL, 0);
				if(!path)
					return -1;
				unsigned int len = strlen(path) + 1;
				if(writen(fd, (char*)&len, sizeof(len)) < 0)
					return -1;
				if(writen(fd, path, len) < 0)
					return -1;
				free(path);
			}
			break;
		case SSH_GET:
			break;
		case SSH_LS:
			{
				struct dirent *filedir = NULL;
				DIR *dir = opendir(".");
				int err = errno;
				char len;
				while(filedir = readdir(dir)){
					if(filedir->d_name[0] == '.'){
						if(filedir->d_name[1] == 0)
							continue;
						else if(filedir->d_name[1] == '.' && filedir->d_name[2] == 0)
							continue;
					}
					len = strlen(filedir->d_name)+1;
					if(writen(fd, &len, 1) < 0)
						return -1;
					if(writen(fd, filedir->d_name, len) < 0)
						return -1;
				}
				len = 0;
				if(writen(fd, &len, 1))
					return -1;
				closedir(dir);
			}
			break;
	}
}

int main(){
	signal(SIGINT, handle);
	listen_sock = socket(PF_INET, SOCK_STREAM, 0);
	if(listen_sock == -1){
		perror("socket error!");
		exit(EXIT_FAILURE);
	}
	struct sockaddr_in addr;
	addr.sin_family = PF_INET;
	addr.sin_port = htons(2222);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

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

	struct epoll_event ev, events[MAX_EVENTS];
	int conn_sock, nfds, epollfd;

	/* Set up listening socket, 'listen_sock' (socket(), bind(), listen()) */

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

	for(;;){
		nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
		if (nfds == -1) {
			perror("epoll_wait");
			exit(EXIT_FAILURE);
		}

		for(n = 0; n < nfds; ++n){
			if(events[n].data.fd == listen_sock){
				//conn_sock = accept(listen_sock, (struct sockaddr *) &local, &addrlen);
				conn_sock = accept(listen_sock, NULL, NULL);
				if(conn_sock == -1){
					perror("accept");
					exit(EXIT_FAILURE);
				}
				setnonblocking(conn_sock);
				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = conn_sock;
				if(epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock,
							&ev) == -1){
					perror("epoll_ctl: conn_sock");
					exit(EXIT_FAILURE);
				}
			}else{
				if(do_use_fd(events[n].data.fd) == -1)
					close(events[n].data.fd);
			}
		}
	}
}
