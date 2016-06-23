all:
	./.build all

clean:
	./.build clean

install:
	./.build install

uninstall:
	./.build uninstall

distcheck:
	./.build distcheck

config:
	./.build config

debug:
	./.build debug

bump:
	./.repo bump minor
