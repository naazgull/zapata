all:
	./.build all

clean:
	./.build clean

install:
	./.build install

uninstall:
	./.build uninstall

distcheck:
	sudo rm -rfv */zapata-*.tar.gz
	./.build distcheck

config:
	./.build config

debug:
	./.build debug

bump:
	./.repo bump minor

package:
	sudo rm -rfv upstream/*
	./.build package

