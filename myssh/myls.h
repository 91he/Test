#ifndef MYLS_H
#define MYLS_H
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#define MAX_WIDTH 100

void swapstr(char *a, char *b);

void fsort(char (*arr)[MAX_WIDTH+1], int len);

int my_ls(char *path);

int ls_for_ssh(int fd);
#endif
