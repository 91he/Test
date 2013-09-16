#include <string.h>
#include <time.h>
#include <unistd.h>
#include "image_cache.h"
#include "image_queue.h"

struct list_head *POOL_HEAD = NULL;
mHashTable *g_H = NULL;

struct list_head *RES_HEAD = NULL;
int g_res = 0;

FILE *gLogFile = NULL;

extern image_queue img_queue;
extern image_queue *img_q_ptr;

void resAdd(int s, int hash_index){
	g_res++;

	struct resList *tmp = malloc(sizeof(struct resList));
	list_add_tail(&tmp->node, RES_HEAD);
	tmp->data[0] = 106;
	*(int*)(tmp->data+1) = s;
	*(int*)(tmp->data+5) = hash_index;
}

int mInitPool(){
	char buf[32];
	sprintf(buf, "/tmp/%d.log", getpid());
	gLogFile = fopen(buf, "w+");

	POOL_HEAD = malloc(sizeof(struct list_head));
	if(POOL_HEAD){
		INIT_LIST_HEAD(POOL_HEAD);
		RES_HEAD = malloc(sizeof(struct list_head));
		if(RES_HEAD){
			INIT_LIST_HEAD(RES_HEAD);
			return 0;
		}
	}
	fprintf(stderr, "POOL_HEAD initialize error: out of space!\n");
	return -1;
}

mHashTable *mInitHash(unsigned int size, unsigned int caps){
	mHashTable *H = malloc(sizeof(mHashTable));
	if(!H){
		fprintf(stderr, "Out of space!\n");
		return NULL;
	}

	H->size = size;
	H->num = 0;
	H->caps = caps;

	H->arr = malloc(sizeof(struct list_head)*size); if(!H->arr){
		free(H);
		fprintf(stderr, "Out of space!\n");
		return NULL;
	}
	int i;
	for(i = 0; i < size; i++){
		INIT_LIST_HEAD(H->arr+i);
	}
	return H;
}

unsigned int mHash(char *data, unsigned int len, mHashTable *H){
	unsigned int r = 0;
	//int a = 63689;
	//int b = 378551;
	int i, j = len/11;
	for(i = 0; i < 10; i++){
		/*
		r = r*a+data[i*j];
		a *= b;
		*/
		r = (r<<5)-r+data[i*j];
	}
	return r%H->size;
}

struct mData *mFind(char *rgb, unsigned int rgblen, mHashTable *H){
	unsigned int key = mHash(rgb, rgblen, H);

	struct list_head *pos = NULL, *n = NULL;
	struct mList *tmp = NULL;
	list_for_each_safe(pos, n, H->arr+key){
		tmp = list_entry(pos, struct mList, hlist);
		if(rgblen == tmp->data.rgblen && !memcmp(tmp->data.rgb, rgb, rgblen))
			return &tmp->data;
	}
	return NULL;
}

void mUpdate(struct mData *pData, mHashTable *H){
	struct mList *tmp = list_entry(pData, struct mList, data);
	unsigned int key = mHash(pData->rgb, pData->rgblen, H);

	list_del(&tmp->hlist);
	list_add(&tmp->hlist, H->arr+key);
	list_del(&tmp->mlist);
	list_add(&tmp->mlist, POOL_HEAD);
}

void mDelete(struct mData *pData, mHashTable *H){
	struct mList *tmp = list_entry(pData, struct mList, data);
	list_del(&tmp->hlist);
	list_del(&tmp->mlist);
	H->num -= tmp->data.jpglen+tmp->data.rgblen;
	free(tmp->data.rgb);
	free(tmp->data.jpg);
	free(tmp);
}


int mInsert(int s, char *rgb, char *jpg, unsigned int rgblen, unsigned int jpglen, mHashTable *H){
	struct mList *tmp = malloc(sizeof(struct mList));
	if(!tmp){
		fprintf(stderr, "Out of space!\n");
		return -1;
	}
	unsigned int key = mHash(rgb, rgblen, H);

	tmp->data.s = s;
	tmp->data.rgb = malloc(rgblen);
	if(!tmp->data.rgb){
		free(tmp);
		fprintf(stderr, "Out of space!\n");
		return -1;
	}
	memcpy(tmp->data.rgb, rgb, rgblen);
	tmp->data.jpg = malloc(jpglen);
	if(!tmp->data.jpg){
		free(tmp->data.rgb);
		free(tmp);
		fprintf(stderr, "Out of space!\n");
		return -1;
	}
	memcpy(tmp->data.jpg, jpg, jpglen);
	tmp->data.rgblen = rgblen;
	tmp->data.jpglen = jpglen;
	list_add(&tmp->hlist, H->arr+key);
	list_add(&tmp->mlist, POOL_HEAD);
	H->num += rgblen+jpglen;
	while(H->num > H->caps){
		tmp = list_entry(POOL_HEAD->prev, struct mList, mlist);
		//if(tmp->data.s)
		if(img_q_ptr->hash_index[tmp->data.s] != -1){
			resAdd(-tmp->data.s, ~img_q_ptr->hash_index[tmp->data.s]);
		}
		img_q_ptr->hash_index[tmp->data.s] = -1;
		imgq_enque(tmp->data.s);
		list_del(&tmp->hlist);
		list_del(&tmp->mlist);
		H->num -= tmp->data.rgblen+tmp->data.jpglen;
		free(tmp->data.rgb);
		free(tmp->data.jpg);
		free(tmp);
	}
	//printf("memory now=%d\n", H->num);
	static time_t tt = 0;
	if(time(NULL) - tt > 20){
		tt = time(NULL);
		char buf[64];
		sprintf(buf, "memory=%d, res=%d\n", H->num, g_res);
		fwrite(buf, 1, strlen(buf), gLogFile);
		fflush(gLogFile);
	}
	return 0;
}

void mUninitHash(mHashTable *H){
	free(H->arr);
	H->arr = NULL;
	free(H);
}

