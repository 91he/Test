#include "image_queue.h"
#include <stdbool.h>

image_queue img_queue;
image_queue *img_q_ptr = &img_queue;

void init_imgq()
{
	int i = 0;
	for(i = 0;i< NUM; i++){
		img_q_ptr->num[i] = i+1;
		img_q_ptr->hash_index[i] = -1;
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
	//printf("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx s = %d\n",s);
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

/*
int main(int argc,char **argv)
{
	init_imgq();
	int s ;
	int i = 0;
	for(i = 0;i<2*NUM;i++){
		s = imgq_deque();	
		imgq_enque(s);
	}
	printf("s is %d\n",s);
	s = imgq_deque();	
	printf("s is %d\n",s);
}
*/
