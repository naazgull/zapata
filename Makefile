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
	./.build distcheck

distclean:
	./.build distclean

config:
	./.build config

debug:
	./.build debug

bump:
	./.repo bump minor

deb:
	sudo rm -rfv upstream/*
	./.build package

