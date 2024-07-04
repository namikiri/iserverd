#!/bin/sh

# Andy Shevchenko <andy@smile.org.ua>

# postgres binaries
PSQL="/usr/bin/psql"
CREATEDB="/usr/bin/createdb"
DROPDB="/usr/bin/dropdb"
DROPUSER="/usr/bin/dropuser"
DUMP="/usr/bin/pg_dump"
 
usage() {
    name=`basename $0`
    echo "Usage: $name create database [user [passwd]]"
    echo "  or : $name drop database"
    echo "  or : $name dump database [tempfile]"
    echo "  or : $name restore database tempfile"
    exit 0
}

create() {
    user="$1"; passwd="$2"
    if [ "$user" = "" ]; then
        echo -n "Please, input database username: " ; read user
        while [ "$user" = "" ]; do   
	    echo -n "Please, input database username: " ; read user
        done
    fi
 	
    if [ "$passwd" = "" ]; then
        echo -n "Please, input database user passwd: " ; stty -echo ; read passwd ; echo " " ; stty echo
	while [ "$passwd" = "" ]; do
    	    echo -n "Please, input database user passwd: " ; stty -echo ; read passwd ; echo " " ; stty echo
	done
    fi

    # We should create databases
    [ -z "`$PSQL -l | grep $users_db`" ] && $CREATEDB $users_db
    # Database initialization (creating tables, views, etc)
    # create new user in database
    if [ -z "`$PSQL -c \"select usename from pg_shadow\" $users_db | grep $user`" ]
    then
        $PSQL -c "CREATE USER $user WITH PASSWORD '$passwd' NOCREATEDB NOCREATEUSER;" $users_db
    fi
}

drop() {
    [ -n "`$PSQL -l | grep $users_db`" ] && $DROPDB $users_db
}

dump() {
    if [ -n "`$PSQL -l | grep $users_db`" ]; then
        if [ -n "$1" ]; then
    	    $DUMP $users_db > "$1"
        else
    	    $DUMP $users_db
        fi
    fi
}

restore() {
    if [ -s "$1" ]; then
        create
        $PSQL -c "\i $1" $users_db
    fi
}

[ -z "$2" ] && usage
users_db="$2"

case "$1" in
    create)
	create "$3" "$4"
	;;
    drop)
	drop
	;;
    dump)
	dump "$3"
	;;
    restore)
	restore "$3"
	;;
    *)
	usage
	;;
esac

