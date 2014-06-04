all: debug release

debug: 
	make -C Debug clean
	make -C Debug all

release: 
	make -C Release clean
	make -C Release all

install:
	mkdir -p debian/tmp/usr/lib/zapata-dbg
	mkdir -p debian/tmp/usr/include
	mkdir -p debian/tmp/usr/src
