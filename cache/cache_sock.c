#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>

int cache_fd = -1;

struct cache_ret{
	int s;
	int size;
	char ret[1048576];
};

int cache_readn(int fd, char *buf, int n){
	int len = 0, r;
	while(len < n){
		r = read(fd, buf+len, n-len);
		if(r < 0){
			perror("read error!");
			return -1;
		}
		len += r;
	}
	return 0;
}

int cache_writen(int fd, char *buf, int n){
	int len = 0, r;
	while(len < n){
		r = write(fd, buf+len, n-len);
		if(r < 0){
			perror("write error!");
			return -1;
		}
		len += r;
	}
	return 0;
}


int init_cache_sock(char *path){
	int fd = socket(PF_LOCAL, SOCK_STREAM, 0);
	if(fd == -1){
		perror("socket error!");
		return -1;
	}
	struct sockaddr_un addr;
	addr.sun_family = PF_LOCAL;
	strcpy(addr.sun_path, path);
	int r = connect(fd, (struct sockaddr*)&addr, sizeof(addr));
	if(r == -1){
		perror("connect error!");
		return -1;
	}
	return fd;
}

struct cache_ret cache_find(int fd, int rgblen, char *rgb){
	struct cache_ret ret = {0};
	char buf[1048580];
	buf[0] = 0;
	memcpy(buf+1, (char*)&rgblen, 4);
	memcpy(buf+5, rgb, rgblen);
	if(cache_writen(fd, buf, 5+rgblen) < 0)
		goto find_error;
	if(cache_readn(fd, (char*)&ret.s, 4) < 0)
		goto find_error;
	if(ret.s){
		if(cache_readn(fd, (char*)&ret.size, 4) < 0)
			goto find_error;
		if(cache_readn(fd, ret.ret, ret.size) < 0)
			goto find_error;
	}
	return ret;
find_error:
	close(fd);
	cache_fd = -1;
	ret.s = 0;
	return ret;
}

int cache_insert(int fd, int rgblen, char *rgb, int jpglen, char *jpg){
	char buf[2097152];
	buf[0] = 1;
	memcpy(buf+1, (char*)&rgblen, 4);
	memcpy(buf+5, rgb, rgblen);
	memcpy(buf+5+rgblen, (char*)&jpglen, 4);
	memcpy(buf+9+rgblen, jpg, jpglen);
	if(cache_writen(fd, buf, 9+rgblen+jpglen) < 0)
		goto insert_error;
	int s = 0;
	if(cache_readn(fd, (char*)&s, 4) < 0)
		goto insert_error;
	return s;
insert_error:
	close(fd);
	cache_fd = -1;
	return 0;
}

int main(){
	int fd = init_cache_sock("/opt/cache_all.sock");
	if(fd == -1)
		return -1;
	struct cache_ret ret = cache_find(fd, 5, "hello");
	if(ret.s){
		printf("%d, %s\n", ret.s, ret.ret);
	}else{
		ret.s = cache_insert(fd, 5, "hello", 5, "world");
		printf("%d\n", ret.s);
	}
	close(fd);
	return 0;
}
