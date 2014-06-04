package_version=0.1
build_configuration=Release

all: debug release

debug: 
	make -C Debug clean
	make -C Debug all

release: 
	make -C Release clean
	make -C Release all

install:
	mkdir -p debian/tmp/usr/lib
	mkdir -p debian/tmp/usr/include/zapata
	cp base exceptions file json log mail mem parsers text thread zapata/core.h debian/tmp/usr/include/zapata -rf
	cp $(build_configuration)/libzapata-core.so debian/tmp/usr/lib/libzapata-core.so.$(package_version)
	ln -s debian/tmp/usr/lib/libzapata-core.so.$(package_version) debian/tmp/usr/lib/libzapata-core.so

package:
	rm -rf ../zapata-core-$(package_version).tar.gz.
	cp -rf ../zapata-core ../zapata-core_$(package_version)
	tar cvzf ../zapata-core_$(package_version).tar.gz ../zapata-core_$(package_version)
	rm -rf ../zapata-core_$(package_version)
