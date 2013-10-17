#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <signal.h> 
#include <sys/param.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <time.h>

void init_daemon(void) 
{ 
	int pid; 
	int i; 
	if(pid=fork()) 
		exit(0);//是父进程，结束父进程 
	else if(pid< 0) 
		exit(1);//fork失败，退出 
	//是第一子进程，后台继续执行 
	setsid();//第一子进程成为新的会话组长和进程组长 
	//并与控制终端分离 
	if(pid=fork()) 
		exit(0);//是第一子进程，结束第一子进程 
	else if(pid< 0) 
		exit(1);//fork失败，退出 
	//是第二子进程，继续 
	//第二子进程不再是会话组长 
#if 0
	for(i=0;i< NOFILE;++i)//关闭打开的文件描述符 
		close(i); 
#endif
	chdir("/tmp");//改变工作目录到/tmp 
	umask(0);//重设文件创建掩模 
	return; 
} 

int main(){
	init_daemon();
	FILE *f;
	while(1){
		f = fopen("log", "a+");
		if(f){
			time_t t = time(0); 
			fprintf(f, "Im here at %s", asctime(localtime(&t))); 
			fclose(f); 
		}
		sleep(10);
	}
	return 0;
}
