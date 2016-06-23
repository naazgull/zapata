
nobase_dist_sysconf_DATA = \
./init.d/zapata \
./logrotate.d/zapata-rest-server

install-exec-hook:
	if [ -f $(sysconfdir)/init.d/zapata ]; then chmod +x $(sysconfdir)/init.d/zapata; fi
