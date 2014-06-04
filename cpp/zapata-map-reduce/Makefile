package_version=0.1
minor_version=3
build_configuration=Release

all: build_$(build_configuration)

build_Debug:  
	make -C Debug clean
	make -C Debug all

build_Release:  
	make -C Release clean
	make -C Release all

install:
	mkdir -p debian/tmp/usr/lib
	mkdir -p debian/tmp/usr/include/zapata
	cp map partition reduce zapata/mapreduce.h debian/tmp/usr/include/zapata -rf
	find debian/tmp/usr/include/zapata -name "*.cpp" | xargs rm -rf
	cp $(build_configuration)/libzapata-map-reduce.so.$(package_version) debian/tmp/usr/lib/
	ln -s libzapata-map-reduce.so.$(package_version) debian/tmp/usr/lib/libzapata-map-reduce.so

package:
	rm -rf ../zapata-map-reduce_$(package_version).$(minor_version).tar.gz
	cp -rf ../zapata-map-reduce ../zapata-map-reduce_$(package_version).$(minor_version)
	find ../zapata-map-reduce_$(package_version).$(minor_version) -name "*.o" | xargs rm -rf
	find ../zapata-map-reduce_$(package_version).$(minor_version) -name "*.d" | xargs rm -rf
	find ../zapata-map-reduce_$(package_version).$(minor_version) -name "*.so" | xargs rm -rf
	find ../zapata-map-reduce_$(package_version).$(minor_version) -name "*.so.$(package_version)" | xargs rm -rf
	tar cvzf ../../upstream/zapata-map-reduce_$(package_version).$(minor_version).tar.gz ../zapata-map-reduce_$(package_version).$(minor_version)
	rm -rf ../zapata-map-reduce_$(package_version).$(minor_version)
