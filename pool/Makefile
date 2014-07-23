pool: main.c libpool.so
	gcc main.c -o pool -L. -lpool
libpool.so: pool.c pool.h list.h
	gcc -fPIC -shared -o libpool.so pool.c -lpthread
clean:
	rm libpool.so pool
