#ifndef CACHE_SOCK_
#define CACHE_SOCK_H
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>

struct cache_ret{
	int s;
	int size;
	char ret[1048576];
};

int cache_readn(int fd, char *buf, int n);

int cache_writen(int fd, char *buf, int n);

int init_cache_sock(char *path);

struct cache_ret cache_find(int fd, int rgblen, char *rgb);

int cache_insert(int fd, int rgblen, char *rgb, int jpglen, char *jpg);
#endif
