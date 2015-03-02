#include <stdio.h>
#include "pool.h"

void *handle(void *arg){
	int x = rand() % 4 + 1;
	printf("hello %d:%d\n", *((int *)arg), x);
	sleep(x);
}

int main(){
	srand(time(NULL));
	struct thread_pool *pool = pool_init(4);
	int i;
	int num[10];
	for(i = 0; i < 10; ++i)
		num[i] = i;
	for(i = 0; i < 10; ++i){
		pool_add(pool, handle, num+i);
	}
	sleep(5);
	pool_destroy(pool);

	return 0;
}
