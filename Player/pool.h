#ifndef POOL_H
#define POOL_H

#include "list.h"

struct pool_work{
	void *arg;
	void *(*proc)(void *arg);
	struct list_head list;
};

struct thread_pool{
	unsigned int max;
	unsigned int cur;
	int alive;
	struct list_head work_head;
	pthread_t *pids;
	pthread_mutex_t lock;
	pthread_cond_t cond_lock;
};

static void *routine(void *arg);

struct thread_pool *pool_init(unsigned int num);

void pool_add(struct thread_pool *pool, void *(*proc)(void *arg), void *arg);

void pool_destroy(struct thread_pool *pool);
#endif
