---------------------------------------------
IServerd 2.5.5 -- Groupware ICQ server clone
---------------------------------------------

This program and the author are in no way affiliated with Mirabilis (AOL).
This program is designed as unix Groupware ICQ server since there is no 
Groupware ICQ server on a unix box. No reverse engineering or decompilation 
of any Mirabilis code took place to make this program.

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

I've made utility called iserver_db_convert. This program will convert your Micro$oft 
Access database to Postgress sql users database. First of all you'll need export 
Access database to text file with symbol ';' as delimiter.
Open ICQSDB.mdb, highlight icqusers_tb table, then select menu file/export. In 
appeared window choose save as text file after that you can copy it on your Unix 
machine and start converting.

NOTE:    You can specify apropriate translate file from translate directory
	 as parameter to have databases in native unix codepage...
	 
WARNING: You should run iserverd before convertation at least one time to 
         create all neccessary tables.

Example: ./iserver_db_convert -d icqusers_tb.txt -x RUSSIAN_WIN


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



--------=======SYSTEM TUNING========--------

If you have many users try to tune your system after installing IServerd to 
avoid iserverd hung and packet dropping. Currently I have only FreeBSD and 
Linux recomendations. If you know how to tune another systems - drop me a 
letter. Here is the list of system variables to change:

FreeBSD:

net.local.dgram.maxdgram    = 10000   # Max packet size
net.local.dgram.recvspace   = 65535   # Max receive buffer size

Maxdatagram parameter example value is strongly recommended for all 
FreeBSD systems. Default system value is 2kb and this is too small for 
ICQ with its 8kb message limit.

Linux:

net.unix.max_dgram_qlen     = 1000    # Max packets in local queue
net.core.rmem_default       = 32768   # Default recv buffer size (bytes)
net.core.wmem_default       = 32768   # Default send buffer size (bytes)
net.core.rmem_max           = 65535   # Max recv buffer size (bytes)
net.core.wmem_max           = 65535   # Max send buffer size (bytes)
net.core.netdev_max_backlog = 2000    # Max packets in global system queue


This is just example values. You should calculate them for your system.
First of all run server_status program on max server load and write down 
max queue size & max queue packets values. This is incoming queue parameters. 
Outgoing queue 8 times bigger. So 

Maximum Buffer Size  = Max_Queue_size*8*2.5 bytes
Maximum Packets Num  = Pack_in_queue*8*2.5
Maximum Packet Size  = 16384 bytes

Restart IServerd after changing sysctl variables. And write down them 
into startup file (/etc/sysctl.conf for Linux/FreeBSD)

