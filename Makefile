all:
	./.build all

single:
	./.build single

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

from:
	./.build from ${module}

one:
	./.build one ${module}
