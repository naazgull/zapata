all:
	./.build all ${exclude}

clean:
	./.build clean ${exclude}

install:
	./.build install ${exclude}

uninstall:
	./.build uninstall ${exclude}

distcheck:
	./.build distcheck ${exclude}

distclean:
	./.build distclean ${exclude}

config:
	./.build config ${exclude}

asan:
	./.build asan ${exclude}

config_debug:
	./.build debug ${exclude}

bump:
	./.repo bump minor

deb:
	sudo rm -rfv upstream/*
	./.build package ${exclude}

format:
	./.build format ${exclude}

tidy:
	./.build tidy ${exclude}

from:
	./.build from ${module} ${exclude}

one:
	./.build one ${module}
