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
	mkdir -p debian/tmp/usr/bin
	cp $(build_configuration)/zapata-rest-daemon debian/tmp/usr/bin/

package:
	rm -rf ../zapata-rest-server-$(package_version).tar.gz
	cp -rf ../zapata-rest-server ../zapata-rest-server_$(package_version)
	find ../zapata-rest-server_$(package_version) -name "*.o" | xargs rm -rf
	find ../zapata-rest-server_$(package_version) -name "*.d" | xargs rm -rf
	find ../zapata-rest-server_$(package_version) -name "*.so" | xargs rm -rf
	find ../zapata-rest-server_$(package_version) -name "*.so.$(package_version)" | xargs rm -rf
	find ../zapata-rest-server_$(package_version) -name "zapata-rest-daemon" | xargs rm -rf
	tar cvzf ../zapata-rest-server_$(package_version).tar.gz ../zapata-rest-server_$(package_version)
	rm -rf ../zapata-rest-server_$(package_version)
