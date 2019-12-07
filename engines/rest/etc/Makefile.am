
nobase_dist_sysconf_DATA = \
./default/zapata.rc \
./zapata/rc.d/.dir \
./zapata/backend-enabled/.dir \
./zapata/backend-available/.dir \
./zapata/certificates/public/.dir \
./zapata/conf.d/auth/.dir \
./zapata/zapata.conf \
./init.d/zapata \
./logrotate.d/zapata

install-exec-hook:
	if [ -f $(sysconfdir)/init.d/zapata ]; then chmod +x $(sysconfdir)/init.d/zapata; fi
	mkdir -p /var/log/zapata/
