#export PKG_CONFIG_PATH=`arm-buildroot-linux-gnueabihf-gcc -print-sysroot`/usr/lib/pkgconfig
#export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:`arm-buildroot-linux-gnueabihf-gcc -print-sysroot`/usr/share/pkgconfig
#export PKG_CONFIG_SYSROOT_DIR=`arm-buildroot-linux-gnueabihf-gcc -print-sysroot`

export PKG_CONFIG=/home/levi/work/arm/host/usr/bin/pkg-config
export  CC="arm-buildroot-linux-gnueabihf-gcc"
./configure --host=arm-buildroot-linux-gnueabihf --target=arm-buildroot-linux-gnueabihf --build=x86_64-unknown-linux-gnu CFLAGS="-mfloat-abi=hard -march=armv7-a -mfpu=neon -marm"
