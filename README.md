IServerd - [OSCAR](https://en.wikipedia.org/wiki/OSCAR_protocol) server, compatible with old [ICQ](https://en.wikipedia.org/wiki/ICQ)
-----------------------------------------------------------------------
 
Iserverd can ne used as Groupware ICQ server clone.
This program and the author are in no way affiliated with Mirabilis (AOL).
This program is designed as unix Groupware ICQ server since there is no
Groupware ICQ server on a unix box. No reverse engineering or decompilation
of any Mirabilis code took place to make this program.

**This repository contains legendary projects of A.V.Shutko and his collegues**

* Iserverd C++ ICQ compatible server project, implements most of OSCAR and V3G possibilities
* [README/V3G HTML set](README/V3G%20HTML%20set) - V3G protocol specification
* [README/OSCAR HTML set](README/OSCAR%20HTML%20set) - OSCAR protocol specification
* [isdwm](contrib/isdwm) - administative web utility for IServerd

Please read documents from README directory before opening issues.

#### Our goal
The goal of this project is to get and store compilable and full functioned code of
IServerd for modern Unix enviroments and modern processor architectures like amd64 or arm.

## Credits for Iserverd

First of all I want to thanks Governing an Informatization department of
Khabarovsk State Technical Univercity for web hosting and work station(s).

Also I should express thanks to my co-workers for their patience to my
incoherent outcries and walking around table during work on IServerd.. :)

* Thanks to Dmitry Panov for his immeasurable help with licence, documentation,
time zones and other...
* Thanks to Alaric Dailey (Destroyer) for his help with databases and Postgres.
He was the man, who push me to start make this project real :)
* Thanks to Samba team for their excellent config parser!
* Thanks to Andy Shevchenko for helping with documentation and linux rpm building.
* Thanks to Maxim Shaposhnikov (Shapa) for his help in porting and moral support
* Thanks to Valentin Nechaev for his help with networking and bugs
* Thanks to Schelstraete Bart for scripting.
* Thanks to Egor Schegolkov for IServerd code improovements
* Thanks to Elias Shtift for oscar protocol module addons
* Thanks to Roman Kuzmenkov for high-load testing & bugreports

## Install notes

### Prerequisites and compilation
**Iserverd** is compatible with multiple Unix-like and POSIX
compatible OS and PostgreSQL 7.4+ including PostgreSQL 17.
You should install PostgreSQL first and create user and database for Iserverd.
Please select and install some supported PostgreSQL version.
In Debian-like linux you can get fresh PostgreSQL from PGDG repository after following command:
```sh
pgver=16;
sudo apt install postgresql-common -y;
sudo /usr/share/postgresql-common/pgdg/apt.postgresql.org.sh;
sudo apt update && sudo apt install postgresql-$pgver postgresql-server-dev-$pgver libpq-dev -y;
```
Please create special database and user for Iserverd after something like
`sudo -u postgres psql -d postgres;`
```SQL
CREATE USER "iserverd" PASSWORD '***';
CREATE DATABASE "ICQ" OWNER "iserverd";
```
⚠ NOTE: Utility from this project for OS PostgreSQL administator user `iserver_db_manage.sh`
can do this automatically during executing like `iserver_db_manage.sh create "ICQ"`, but
after PostgreSQL 10 there is no reason to use outer program instead of native SQL.

IP address, port, user name and password is a typical PostgreSQL network login set.
Please use later the same login requsites as was created.

After you can get this project code and compile Iserverd.
```sh
git clone https://github.com/namikiri/iserverd.git -b master Iserverd && cd Iserverd;
cat COPYRIGHT | more;
./configure;
make all;
```
Please note `confugure` script is very flexible and allows use different non default 
directoties for installing, include files, libraries, man files etc. 

⚠ NOTE: By default server will install in system directories (`/usr/bin`;
`/etc/iserverd`; etc..) If you want to install it as one packet in separate
directory you should specify prefix (`./configure --prefix=/usr/localМ)

EXAMPLE: `./configure --prefix=/usr/local --with-name=ICQ_Server \`
	  `--with-debug; make all;`

	  This will compile and install IServerd as single packet
	  into `/usr/local/ICQ_Server` directory. Binary will be
	  compiled with debugging symbols.

If `make all` shows no errors, switch to OS superuser, `cd` to the directory of this project and execute
`sudo make install;`
Installed executable files will have `iserver_` prefix and will be added by default to `/usr/bin` on Linux
except to main `iserverd` installed to other place.

### Intergration to system

Rename installed `*.conf.default` files to `*.conf` and edit them.
You'll need to change database login requsites such as address, port, user name and user password.
Also please change info password, interfaces, admin email. If you want to use v7 clients (like icq2k) you should
also setup V7 BOS address parameter in v7_proto.conf configuration file.

IServerd should be able connect to database with name, database username and password specified in config file.

After that you'll need to edit `/etc/rsyslog.d/*.conf` and `/etc/rsyslog.conf` and add proper lines to
log all server output to proper log files. On FreeBSD you can use following
configuration:

```
bash>cat /etc/syslog.conf |grep IServer
!IServer
*.*	/usr/local/iserverd/log/IServer.log
```

Then go to server `/etc/iserverd` (default if no other during `confignre` run) directory
and run `iserverd.sh` script with "start" as parameter.

WARNING: IServerd will write all important log data to syslog.
Debug.log is for debugging only. If you can't run it - look into your
syslog - probably iserverd have problem with database, config or interfaces.

### Database initialization

1. Ensure you have accsessable PostgreSQL database. On local system this can be made by `sudo ss -lntp`, `pg_lsclusters`, `psql -h 127.0.0.1 -d "ICQ" -U "iserverd"` or other test or diagnostics commands.
2. `iserver_db_convert` can be usefully for ICQGroupwareServer/NT + M$Access users. Note from 2024: does anybody???
3. `iserver_db_check` runned with PostgereSQL database rights of ownwer of special Iserverd database **should automaticaly create all needed tables and views**.

### Create new ICQ user

There is special script `iserver_icquser` placed in `/etc/iserver/db/` by default. Please use this script with `add`, `del`
and other parametres for ICQ user account management.

⚠ WARNING: please insure this script can be executed from usual user. On some systems you should execute this script from
PostgreSQL superuser in DB and/or from OS superuser.

### Auto ICQ user registration

You can switch on this server feature in it's config file. Just set

  V3 auto registration    = Yes
  V3 post-register info   = etc/texts/post_reg_auto.txt

and then ajust texts/post_reg_auto.txt for your needs
In this file you can use %U% and %P% variables - they are
substituted after registration to new user uin and password

⚠ WARNING: When you switching _ON_ auto registration you MUST also change
V3 post-register info file to post_reg_auto.txt or your personal.

⚠ WARNING: When you switching _OFF_ auto registration you MUST also change
V3 post-register info file to post_reg_man.txt or your personal.

If registration enabled and autoregistration disabled server insert
new user records into special registration table "register_requests"

⚠ NOTE: **Protocol V7 doesn't support manual registration**.

## License
See [COPYRIGHT](COPYRIGHT)
