.PHONY:
	all
#all: player plug FRPlayer
all: FRPlayer

#player: main.c npp_funcs.c npn_funcs.c curl_url.c pool.c player.c
#	gcc main.c npp_funcs.c npn_funcs.c pool.c curl_url.c -DXP_UNIX=1 -DMOZ_X11=1 -fPIC -ldl -g -o player `pkg-config --libs --cflags gtk+-2.0` -lcurl -lX11
FRPlayer: main.c npp_funcs.c npn_funcs.c curl_url.c pool.c frapi.c box-fixed.c
	gcc frapi.c npp_funcs.c npn_funcs.c pool.c curl_url.c box-fixed.c -DXP_UNIX=1 -DMOZ_X11=1 -fPIC -ldl -g -o FRPlayer `pkg-config --libs --cflags gtk+-2.0` -lcurl -lX11 -lXcursor
#gcc main.c npp_funcs.c npn_funcs.c pool.c curl_url.c -DXP_UNIX=1 -DMOZ_X11=1 -ldl -g -o player `pkg-config --libs --cflags gtk+-3.0` -lX11 -lcurl
#plug: plug.c npp_funcs.c npn_funcs.c curl_url.c pool.c player.c
#	gcc plug.c npp_funcs.c npn_funcs.c pool.c curl_url.c -DXP_UNIX=1 -DMOZ_X11=1 -fPIC -ldl -g -o plug `pkg-config --libs --cflags gtk+-2.0` -lcurl -lX11 -lXcursor
clean:
	rm player plug FRPlayer
