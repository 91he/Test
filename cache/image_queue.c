#include "image_queue.h"
#include <stdbool.h>

image_queue img_queue;
image_queue *img_q_ptr = &img_queue;

void init_imgq()
{
	int i = 0;
	for(i = 0;i< NUM; i++){
		img_q_ptr->num[i] = i+1;
		//img_q_ptr->hash_index[i] = -1;
	}
	img_q_ptr->size = NUM;
	img_q_ptr->cur = 0;
}

bool imgq_isfull()
{
	return img_q_ptr->size >= NUM;
}

bool imgq_isempty()
{
	return img_q_ptr->size <= 0;
}

bool imgq_enque(int s)
{
	if(imgq_isfull(img_q_ptr) || s <=0)
		return false;
	int index = (img_q_ptr->cur + img_q_ptr->size)%NUM;
	img_q_ptr->num[index] = s;
	img_q_ptr->size++;
	return true;
}

int imgq_deque()
{
	if(imgq_isempty(img_q_ptr))
		return 0;
	img_q_ptr->size--;

	int tmp = img_q_ptr->cur;
	if(img_q_ptr->cur + 1 >= NUM)
		img_q_ptr->cur = (img_q_ptr->cur + 1)%NUM;
	else 
		img_q_ptr->cur++;
	return img_q_ptr->num[tmp];
}
