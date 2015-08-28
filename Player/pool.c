#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "pool.h"

struct thread_pool *pool_init(unsigned int num){
	struct thread_pool *pool = malloc(sizeof(struct thread_pool));
	if(!pool) return NULL;

	pool->pids = malloc(sizeof(pthread_t) * num);

	if(!pool->pids) goto done1;
	if(pthread_mutex_init(&pool->lock, NULL) < 0) goto done2;
	if(pthread_cond_init(&pool->cond_lock, NULL) < 0) goto done2;

	pool->max = num;
	pool->cur = 0;
	pool->alive = 1;
	INIT_LIST_HEAD(&pool->work_head);

	int i = 0;
	for(; i < num; ++i){
		pthread_create(pool->pids + i, NULL, routine, (void*)pool);
	}

	return pool;
done2:
	free(pool->pids);
done1:
	free(pool);

	return NULL;
}

static void *routine(void *arg){
	struct thread_pool *pool = arg;
	void *(*proc)(void *arg);
	void *varg;
	while(pool->alive){
		pthread_mutex_lock(&pool->lock);

		while(pool->cur == 0 && pool->alive)
			pthread_cond_wait(&pool->cond_lock, &pool->lock);

		if(!pool->alive){
			pthread_mutex_unlock(&pool->lock);
			break;
		}

		struct pool_work *tmp = list_entry(pool->work_head.prev, struct pool_work, list);
		proc = tmp->proc;
		varg = tmp->arg;
		list_del(&tmp->list);
		free(tmp);

		pool->cur--;
		pthread_mutex_unlock(&pool->lock);

		proc(varg);
	}
}

void pool_add(struct thread_pool *pool, void *(*proc)(void *arg), void *arg){
	pthread_mutex_lock(&pool->lock);
	
	if(!pool->alive){
		pthread_mutex_unlock(&pool->lock);
		return;
	}

	struct pool_work *head = calloc(1, sizeof(struct pool_work));

	head->proc = proc;
	head->arg = arg;
	pool->cur++;

	list_add(&head->list, &pool->work_head);

	pthread_cond_signal(&pool->cond_lock);

	pthread_mutex_unlock(&pool->lock);
}

void pool_destroy(struct thread_pool *pool){
	int i;
	pool->alive = 0;

	pthread_cond_broadcast(&pool->cond_lock);

	for(i = 0; i < pool->max; ++i){
		pthread_join(pool->pids[i], NULL);
	}
	free(pool->pids);

	struct list_head *pos, *n;
	struct pool_work *tmp;
	list_for_each_safe(pos, n, &pool->work_head){
		tmp = list_entry(pos, struct pool_work, list);
		list_del(pos);
		free(tmp);
	}

	pthread_mutex_destroy(&pool->lock);
	pthread_cond_destroy(&pool->cond_lock);
	free(pool);
}
