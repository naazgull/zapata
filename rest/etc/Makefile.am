
nobase_dist_sysconf_DATA = \
./default/zapata.rc \
./zapata/rc.d/default.rc \
./zapata/backend-enabled/.dir \
./zapata/backend-available/default.conf \
./zapata/certificates/public/.default \
./zapata/conf.d/default.conf \
./zapata/zapata.conf \
./init.d/zapata \
./logrotate.d/zapata

install-exec-hook:
	if [ -f $(sysconfdir)/init.d/zapata ]; then chmod +x $(sysconfdir)/init.d/zapata; fi
	mkdir -p /var/log/zapata/
