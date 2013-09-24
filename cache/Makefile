cache: image_cache.o epoll.o image_queue.o
	gcc image_cache.o epoll.o image_queue.o -ocache
epoll.o: epoll.c
	gcc -c epoll.c
image_cache.o: image_cache.c
	gcc -c image_cache.c
image_queue.o: image_queue.c
	gcc -c image_queue.c
clean:
	rm -rf *.o *~ cache
