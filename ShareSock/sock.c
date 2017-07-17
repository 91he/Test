#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

int tcp_connect(char *ip, int port){
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    printf("fd: %d\n", fd);

    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    int ret = connect(fd, (struct sockaddr*)&addr, sizeof(addr));

    printf("connect ret: %d\n", ret);
    
    return fd;
}

int main(){
#if 0
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    printf("fd: %d\n", fd);

    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(12333);
    addr.sin_addr.s_addr = inet_addr("192.168.100.172");

    int ret = connect(fd, (struct sockaddr*)&addr, sizeof(addr));

    printf("connect ret: %d\n", ret);
#endif
    int fd = tcp_connect("192.168.100.172", 12333);

    char val;
    read(fd, &val, 1);

    int fd2 = tcp_connect("192.168.100.172", 12333);

    char buf[1024];

    read(fd2, buf, 6);

    printf("\t%s\n", buf);

    close(fd);
    close(fd2);
    return 0;
}
