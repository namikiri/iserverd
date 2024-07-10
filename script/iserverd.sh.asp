#! /bin/sh
#
# iserverd		Groupware ICQ server clone
#
# chkconfig: 345 95 05
# description: Groupware ICQ Server clone
# processname: iserverd
# pidfile: /var/run/iserverd/iserverd.pid

# pathes
VAR=/var/run/iserverd
BIN=/usr/bin
SBIN=/usr/sbin

# Source function library.
. /etc/rc.d/init.d/functions

# Get config.
. /etc/sysconfig/network

# Check that networking is up.
[ ${NETWORKING} = "no" ] && exit 0

[ -f ${SBIN}/iserverd ] || exit 0 

# Source iserverd configuration.
DATABASE="users_db"
USER="iserverd"
PASSWD="DEFAULT"
if [ -f /etc/sysconfig/iserverd ] ; then
        . /etc/sysconfig/iserverd
fi

RETVAL=0
prog="iserverd"

start() {
	# Check database (and create it if need)
	su -l postgres -c "${BIN}/db_manage.sh create $DATABASE $USER $PASSWD"
	# Start daemon.
	echo -n $"Starting $prog: "
	daemon --user iserverd ${SBIN}/iserverd
	RETVAL=$?
	echo
	[ $RETVAL -eq 0 ] && touch /var/lock/subsys/iserverd
}

stop() {
        # Stop daemon.
	echo -n $"Shutting down $prog: "
        [ -f ${VAR}/iserverd.pid ] && \
	    kill -TERM `cat ${VAR}/iserverd.pid` && success || failure
        RETVAL=$?
        echo
        rm -f ${VAR}/iserverd.pid
        [ $RETVAL -eq 0 ] && rm -f /var/lock/subsys/iserverd
}

# See how we were called.
case "$1" in
  start)
	start
        ;;
  stop)
	stop
	;;
  restart|reload)
	stop
	sleep 1
	start
	RETVAL=$?
	;;
  condrestart)
        if [ -f /var/lock/subsys/iserverd ]; then
            stop
            start
            RETVAL=$?
        fi
        ;;
  status)
	status iserverd
	RETVAL=$?
	;;
  *)
        echo "Usage: `basename $0` {start|stop|restart|condrestart|status}"
        exit 1
esac

exit $RETVAL

