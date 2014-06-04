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
	cp db zapata/mongodb.h debian/tmp/usr/include/zapata -rf
	find debian/tmp/usr/include/zapata -name "*.cpp" | xargs rm -rf
	cp $(build_configuration)/libzapata-mongodb.so.$(package_version) debian/tmp/usr/lib/
	ln -s libzapata-mongodb.so.$(package_version) debian/tmp/usr/lib/libzapata-mongodb.so

package:
	rm -rf ../zapata-mongodb-$(package_version).tar.gz
	cp -rf ../zapata-mongodb ../zapata-mongodb_$(package_version)
	find ../zapata-mongodb_$(package_version) -name "*.o" | xargs rm -rf
	find ../zapata-mongodb_$(package_version) -name "*.d" | xargs rm -rf
	find ../zapata-mongodb_$(package_version) -name "*.so" | xargs rm -rf
	find ../zapata-mongodb_$(package_version) -name "*.so.$(package_version)" | xargs rm -rf
	tar cvzf ../zapata-mongodb_$(package_version).tar.gz ../zapata-mongodb_$(package_version)
	rm -rf ../zapata-mongodb_$(package_version)
