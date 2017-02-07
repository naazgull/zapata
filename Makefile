all:
	./.build all

j4:
	./.build j4

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

deb:
	sudo rm -rfv upstream/*
	./.build package

