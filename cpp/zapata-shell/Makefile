package_version=0.1
minor_version=8
build_configuration=Release

all: build_$(build_configuration)

build_Debug:  
	make -C Debug clean
	make -C Debug all

build_Release:  
	make -C Release clean
	make -C Release all

install:
	mkdir -p debian/tmp/usr/bin
	cp $(build_configuration)/zptsh debian/tmp/usr/bin/

package:
	rm -rf ../zapata-shell_$(package_version).$(minor_version).tar.gz
	cp -rf ../zapata-shell ../zapata-shell_$(package_version).$(minor_version)
	find ../zapata-shell_$(package_version).$(minor_version) -name "*.o" | xargs rm -rf
	find ../zapata-shell_$(package_version).$(minor_version) -name "*.d" | xargs rm -rf
	find ../zapata-shell_$(package_version).$(minor_version) -name "*.so" | xargs rm -rf
	find ../zapata-shell_$(package_version).$(minor_version) -name "*.so.$(package_version)" | xargs rm -rf
	find ../zapata-shell_$(package_version).$(minor_version) -name "zapata-rest-daemon" | xargs rm -rf
	tar cvzf ../../upstream/zapata-shell_$(package_version).$(minor_version).tar.gz ../zapata-shell_$(package_version).$(minor_version)
	rm -rf ../zapata-shell_$(package_version).$(minor_version)
