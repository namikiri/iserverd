#!/bin/sh

ISERVERD="/usr/sbin/iserverd"
PIDFILE="/var/run/iserverd/iserverd.pid"

# See how we were called.
case "$1" in
  start)
        # Start daemons.
        echo -n "Starting iserverd: "
        $ISERVERD -o
	sleep 1
	if [ -f $PIDFILE ]; then
          echo "   [ OK ]"
	else
	  echo "   [ FAILED ]"
	fi
        ;;

  stop)
        # Stop daemons.
        echo -n "Shutting down iserverd: "
	if [ -f $PIDFILE ]; then
  	  kill `cat $PIDFILE`
	  echo "   [ OK ]"
	else
	  echo "   [ FAILED ]"
	fi
        rm -f $PIDFILE
        ;;

  status)
   
        if [ -f $PIDFILE ]; then
	  echo -n "Iserverd is running, pid: "
	  echo `cat $PIDFILE`
	else
	  echo "Iserverd is stopped"
	fi	
	;;

  restart)
	$0 stop
	sleep 1
	$0 start
	;;

  *)
        echo "Usage: iserverd {start|stop|restart|status}"
        exit 1
esac

exit 0
