#
# Regular cron jobs for the usermanage-client package
#
0 4	* * *	root	[ -x /usr/bin/usermanage-client_maintenance ] && /usr/bin/usermanage-client_maintenance
