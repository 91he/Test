.PHONY:
	all
all: libflash_redir.so FRPlayer

#FRPlayer: npp_funcs.c npn_funcs.c curl_url.c pool.c frapi.c box-fixed.c rbtree.c
#	gcc frapi.c npp_funcs.c npn_funcs.c pool.c curl_url.c box-fixed.c rbtree.c -DXP_UNIX=1 -DMOZ_X11=1 -fPIC -ldl -g -o FRPlayer `pkg-config --libs --cflags gtk+-2.0` -lcurl -lX11 -lXcursor
FRPlayer: frapi.c box-fixed.c
	gcc frapi.c box-fixed.c -g -o FRPlayer `pkg-config --libs --cflags webkit2gtk-3.0` -lX11 -lXcursor
libflash_redir.so: frapi.c box-fixed.c
	gcc frapi.c box-fixed.c -DMAKE_SO -fPIC -shared -g -o libflash_redir.so `pkg-config --libs --cflags webkit2gtk-3.0` -lX11 -lXcursor
clean:
	rm FRPlayer libflash_redir.so
