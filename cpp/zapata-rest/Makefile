package_version=0.1
minor_version=2
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
	cp api resource thread zapata/rest.h debian/tmp/usr/include/zapata -rf
	find debian/tmp/usr/include/zapata -name "*.cpp" | xargs rm -rf
	cp $(build_configuration)/libzapata-rest.so.$(package_version) debian/tmp/usr/lib/
	ln -s libzapata-rest.so.$(package_version) debian/tmp/usr/lib/libzapata-rest.so

package:
	rm -rf ../zapata-rest_$(package_version).$(minor_version).tar.gz
	cp -rf ../zapata-rest ../zapata-rest_$(package_version).$(minor_version)
	find ../zapata-rest_$(package_version).$(minor_version) -name "*.o" | xargs rm -rf
	find ../zapata-rest_$(package_version).$(minor_version) -name "*.d" | xargs rm -rf
	find ../zapata-rest_$(package_version).$(minor_version) -name "*.so" | xargs rm -rf
	find ../zapata-rest_$(package_version).$(minor_version) -name "*.so.$(package_version)" | xargs rm -rf
	tar cvzf ../../upstream/zapata-rest_$(package_version).$(minor_version).tar.gz ../zapata-rest_$(package_version).$(minor_version)
	rm -rf ../zapata-rest_$(package_version).$(minor_version)
