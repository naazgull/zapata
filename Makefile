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

autoupdate:
	./.build autoupdate ${exclude}

config:
	./.build config ${exclude}

config_debug:
	./.build config_debug ${exclude}

bump:
	./.repo bump minor

format:
	./.build format ${exclude}

from:
	./.build from ${module} ${action}

to:
	./.build to ${module} ${action}

one:
	./.build one ${module} ${action}
