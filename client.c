#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include "ctl_types.h"
#include "host.h"

int writen(int fd, char *buf, int n){
	int len = 0;
	int r;
	while(len < n){
		r = write(fd, buf, n);
		if(r == -1){
			perror("Write error!");
			return -1;
		}
		len += r;
	}
	return len;
}

int readn(int fd, char *buf, int n){
	int len = 0;
	int r;
	while(len < n){
		r = read(fd, buf, n);
		if(r == -1){
			perror("Read error!");
			return -1;
		}
		len += r;
	}
	return len;
}

int main(int argc, char **argv){

	CTL_CMD tmp;
	int ip;
	bzero(&tmp, sizeof(tmp));

	int op = 0;
	while(*++argv){
		if(!strcmp(*argv, "-s")){
			op += 1;
		}else if(!strcmp(*argv, "-g")){
			op += 2;
		}else if(!strncmp(*argv, "-ip=", 4)){
			ip = tmp.ip = inet_addr(*argv+4);
			op += 4;
		}else if(!strcmp(*argv, "-f")){
			if(*++argv)
				tmp.data.flag = atoi(*argv)+1;
			else{
				printf("\tOption flag was not specified.\n");
				return -1;
			}
			op += 8;
		}else if(!strcmp(*argv, "-j")){
			op += 16;
			if(*++argv)
				tmp.data.pps_per_second = atoi(*argv);
			else{
				printf("\tOption -j pps judgement was not specified.\n");
				return -1;
			}
		}else if(!strcmp(*argv, "-h")){
			op += 32;
		}else if(!strcmp(*argv, "-n")){
		}else if(!strcmp(*argv, "-p")){
		}
	}

	struct sockaddr_un addr;
	int fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if(fd == 1){
		perror("Socket error!");
		return -1;
	}
	strcpy(addr.sun_path, "ctl.socket");
	addr.sun_family = PF_UNIX;

	int r = connect(fd, (struct sockaddr*)&addr, sizeof(addr));
	if(r == -1){
		fprintf(stderr, "Connect error:%m!\nPlease make sure server has been started.\n");
		close(fd);
		return -1;
	}

	int len = 0;
	int size = sizeof(tmp);
	r = 0;
	char c;

	if(op&32){
		printf("\tusage: cmd [-s|g] [-f flag] [-j pps_judgement] [-ip=a.b.c.d]\n"
				"\n\t-s set args of the ip.\n"
				"\t-g get args of the ip.\n"
				"\t-f set flag of the ip which work with '-s'.\n"
				"\t-j just as '-f'. set pps_per_second of the ip.\n");
	}else if((op&3)^3){
		if(op&3){
			if(op&2){//get
				if(op&8) printf("\tUseless option -f.\n");
				if(op&16) printf("\tUseless option -j.\n");
				tmp.cmd_id = CMD_GET_IP;
				if(writen(fd, (char*)&tmp, size) == -1){
					r = -1;
					goto end;
				}
				do{
					if(readn(fd, (char*)&tmp, size) == -1){
						r = -1;
						break;
					}
					if(tmp.ip)
						printf("\tIP=%d.%d.%d.%d, flag=%d, flow = %d\n", tmp.ip&0xff, 
								(tmp.ip>>8)&0xff, (tmp.ip>>16)&0xff, (tmp.ip>>24)&0xff, 
								tmp.data.flag, tmp.data.flow);
					else{
						printf("\tDid not find IP=%d.%d.%d.%d\n", ip&0xff, (ip>>8)&0xff, (ip>>16)&0xff, (ip>>24)&0xff);
					}
					if((r = readn(fd, &c, 1)) == -1){
						r = -1;
						break;
					}
				}while(c);
			}else{//set
				if(op&4){
					if(op^5){
						tmp.cmd_id = CMD_SET_IP;
						if(writen(fd, (char*)&tmp, size) == -1)
							r = -1;
						else{
							if(readn(fd, (char*)&tmp, size) == -1)
								r = -1;
							else{
								if(tmp.ip)
									printf("\tSuccessfully setting args of ip.\n");
								else{
									printf("\tFailed setting args of ip.\n");
								}
							}
						}
					}else{
						printf("\tWhich option do you want to set?\n");
						printf("\tTry -h for help.\n");
					}
				}else{
					printf("\tNo ip was specified!\n");
				}
			}
		}else{
			printf("\tNo option!\n\tTry -h for help.\n");
			r = -1;
		}
	}else{
		printf("\tOption '-s' can not specified together with '-g'!\n");
		r = -1;
	}

end:
	close(fd);
	return r;
}
