IServerd 2.5.5 -- Groupware ICQ server clone
---------------------------------------------

This program and the author are in no way affiliated with Mirabilis (AOL).
This program is designed as unix Groupware ICQ server since there is no
Groupware ICQ server on a unix box. No reverse engineering or decompilation
of any Mirabilis code took place to make this program.

**This repository contains legendary projects of A.V.Shutko and his collegues **
* Iserverd C++ ICQ compatible server project, implements most of OSCAR and V3G possibilities
* [README/V3G HTML set] - V3G protocol specification
* [README/OSCAR HTML set] - OSCAR protocol specification
* [contrib/isdwm] - administative web utility server for IServerd

Please read documents from README directory before opening issues.

The goal of this project is to get and store compilable and full functioned code of
IServerd for modern Unix enviroments and modern processor architectures.

## Credits for Iserverd

First of all I want to thanks Governing an Informatization department of
Khabarovsk State Technical Univercity for web hosting and work station(s).

Also I should express thanks to my co-workers for their patience to my
incoherent outcries and walking around table during work on IServerd.. :)

Thanks to Dmitry Panov for his immeasurable help with licence, documentation,
time zones and other...

Thanks to Alaric Dailey (Destroyer) for his help with databases and Postgres.
He was the man, who push me to start make this project real :)

Thanks to Samba team for their excellent config parser!
Thanks to Andy Shevchenko for helping with documentation and linux rpm building.
Thanks to Maxim Shaposhnikov (Shapa) for his help in porting and moral support
Thanks to Valentin Nechaev for his help with networking and bugs
Thanks to Schelstraete Bart for scripting.
Thanks to Egor Schegolkov for IServerd code improovements
Thanks to Elias Shtift for oscar protocol module addons
Thanks to Roman Kuzmenkov for high-load testing & bugreports

## Actual part of original README of Iserverd

-----======INSTALL NOTES======-------

First of all you'll need to install Postgres SQL server. After that you can
begin IServer compilation.

tar xzvf ./IServerd-2.5.5.tar.gz
cd ./IServerd-2.5.5
cat COPYRIGHT | more :)
./configure --prefix=/usr/local
make all
make install


Then rename installed *.conf.default files to *.conf and edit them.
You'll need to change database password, info password, interfaces,
admin email. If you want to use v7 clients (like icq2k) you should
also setup V7 BOS address parameter in v7_proto.conf configuration file.

Then init IServerd database. IServerd should be able connect to
database with name, database username and password specified in config file.
It is recommended to leave blank "database addr" field because this is
the simpliest way for iserverd to connect to database server. You can create
database and user with script db_manage.sh (./db_manage.sh create users_db).
Please don't use default database passwords "sicq" AND "DEFAULT" because
iserverd will not work with them.

After that you'll need to edit /etc/syslog.conf and add proper lines to
log all server output to proper log files. On FreeBSD you can use following
configuration:

==================================================
bash>cat /etc/syslog.conf |grep IServer
!IServer
*.*	/usr/local/iserverd/log/IServer.log
==================================================

Then go to server etc directory and run iserverd.sh script with "start"
as parameter.

WARNING: IServerd will write all important log data to syslog.
Debug.log is for debugging only. If you can't run it - look into your
syslog - probably iserverd have problem with database, config or interfaces.

NOTE: By default server will install in system directories (/usr/bin;
/etc/iserverd; etc..) If you want to install it as one packet in separate
directory you should specify prefix (./configure --prefix=/usr/local)

EXAMPLE: ./configure --prefix=/usr/local --with-name=ICQ_Server \
	  --with-debug; make all; make install

	  This will compile and install IServerd as single packet
	  into /usr/local/ICQ_Server directory. Binary will be
	  compiled with debugging symbols.


----======DATABASE INITIALIZATION=====------

Before you start db initialization and convertation you should start postgres
postmaster. After that go $prefix/bin/.

postgres>iserver_db_manage.sh create users_db

and press enter - script will ask you about new user password and will create server
database and server database user. IServerd will create all neccessary tables at first
run.

If you have currently worked ICQGroupwareServer/NT and want to keep your users
database you can convert it with M$Access to text mode and than convert to
postgres with bin/iserver_db_convert utility.

Creating new user:
In iserverd etc directory you'll find icquser script. With this script you can
add new or delete existing user.

WARNING: you should execute this commands from user postgres (or you can add
"root" user into postgresql)

-----=======OLD DATABASE CONVERTATION=======-------
Not actual, see original documants in [README] directory.
--------=======SYSTEM TUNING========--------
Not actual, see original documants in [README] directory.

--------=======AUTO REGISTRATION========--------

You can switch on this server future in it's config file. Just set

  V3 auto registration    = Yes
  V3 post-register info   = etc/texts/post_reg_auto.txt

and then ajust texts/post_reg_auto.txt for your needs
In this file you can use %U% and %P% variables - they are
substituted after registration to new user uin and password

WARNING: When you switching _ON_ auto registration you MUST also change
V3 post-register info file to post_reg_auto.txt or your personal.

WARNING: When you switching _OFF_ auto registration you MUST also change
V3 post-register info file to post_reg_man.txt or your personal.

If registration enabled and autoregistration disabled server insert
new user records into special registration table "register_requests"

NOTE: Protocol V7 doesn't support manual registration.
