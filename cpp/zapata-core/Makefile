package_version=0.1

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

package:
	rm -rf ../zapata-core-$(package_version).tar.gz.
	cp -rf ../zapata-core ../zapata-core_$(package_version)
	tar cvzf ../zapata-core_$(package_version).tar.gz ../zapata-core_$(package_version)
	rm -rf ../zapata-core_$(package_version)
