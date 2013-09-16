cache: image_cache.o poll.o image_queue.o
	gcc image_cache.o poll.o image_queue.o -ocache
poll.o: poll.c
	gcc -c poll.c
image_cache.o: image_cache.c
	gcc -c image_cache.c
image_queue.o: image_queue.c
	gcc -c image_queue.c

