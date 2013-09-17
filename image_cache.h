#ifndef IMAGE_CACHE_H
#define IMAGE_CACHE_H
#include <stdio.h>
#include <stdint.h>
#include "list.h"

#define myoffset(a, b, c) ((size_t)(&((c*)0)->b)-(size_t)(&((c*)0)->a))

#define my_entry(p, a, b, c) ({ \
		const typeof(((c*)0)->a) *__mptr = (p); \
		(typeof(((c*)0)->b)*)((char*)__mptr + myoffset(a, b, c));})

#define my_list_for_each_entry(pos, head, a, b, c, n) \
	head--; \
	struct list_head * __##head = NULL; \
	while(head++ && n--) \
		for(pos = my_entry(head->next, a, b, c); \
				(__##head = my_entry(pos, b, a, c)) != head; \
				pos = my_entry(__##head->next, a, b, c))

#define hash_for_each(pos, head, n) \
	my_list_for_each_entry(pos, head, hlist, data, struct mList, n)

struct resList{
	char data[9];
	struct list_head node;
};

struct mData{
	int s;
	char *rgb;
	char *jpg;
	unsigned int rgblen;
	unsigned int jpglen;
};

struct mList{
	struct mData data;
	struct list_head hlist;//hash list
	struct list_head mlist;//loop list
};

typedef struct mHashTable{
	unsigned int size;
	unsigned long num;
	unsigned long caps;
	struct list_head *arr;
}mHashTable;

int mArrange(int size);

int mInitPool();

void resAdd(int s, int hash_index);

mHashTable *mInitHash(unsigned int size, unsigned long caps);

unsigned int mHash(char *data, unsigned int len, mHashTable *H);

struct mData *mFind(char *data, unsigned int rgblen, mHashTable *H);

void mUpdate(struct mData *data, mHashTable *H);

void mDelete(struct mData *data, mHashTable *H);

int mInsert(int s, char *rgb, char *jpg, unsigned int rgblen, unsigned int jpglen, mHashTable *H);

void mUninitHash(mHashTable *H);

/*
int main(){
	mHashTable * H= mInitHash(1003);
	struct mData data1 = {1, 1};
	struct mData data2 = {2, 1};
	struct mData data3 = {3, 1};
	mInsert(data1, H);
	mInsert(data2, H);
	mInsert(data3, H);
	struct mData *tmp = NULL;
	if(tmp = mFind(data1.ip, H)) printf("NULL\n");
	if(mFind(data2.ip, H)) printf("NULL\n");
	if(mFind(data3.ip, H)) printf("NULL\n");
	printf("%d\n", H->dnum);
	mDelete(tmp, H);
	if(mFind(data1.ip, H)) printf("NULL\n");
	printf("%d\n", H->dnum);
	mUninitHash(H);
	return 0;
}
*/
#endif
