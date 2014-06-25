package_version=0.1
minor_version=6
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
	cp auth resource zapata/oauth2.h debian/tmp/usr/include/zapata -rf
	find debian/tmp/usr/include/zapata -name "*.cpp" | xargs rm -rf
	cp $(build_configuration)/libzapata-oauth.so.$(package_version) debian/tmp/usr/lib/
	ln -s libzapata-oauth.so.$(package_version) debian/tmp/usr/lib/libzapata-oauth.so

package:
	rm -rf ../zapata-oauth_$(package_version).$(minor_version).tar.gz
	cp -rf ../zapata-oauth ../zapata-oauth_$(package_version).$(minor_version)
	find ../zapata-oauth_$(package_version).$(minor_version) -name "*.o" | xargs rm -rf
	find ../zapata-oauth_$(package_version).$(minor_version) -name "*.d" | xargs rm -rf
	find ../zapata-oauth_$(package_version).$(minor_version) -name "*.so" | xargs rm -rf
	find ../zapata-oauth_$(package_version).$(minor_version) -name "*.so.$(package_version)" | xargs rm -rf
	tar cvzf ../../upstream/zapata-oauth_$(package_version).$(minor_version).tar.gz ../zapata-oauth_$(package_version).$(minor_version)
	rm -rf ../zapata-oauth_$(package_version).$(minor_version)
