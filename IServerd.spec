Summary: Groupware ICQ server clone
Name: IServerd
Version: 2.5.5-20080609
Release: 1
License: BSD
Group: System Environment/Daemons
Source: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-buildroot
BuildRequires: postgresql-devel >= 7.2
BuildRequires: mktemp
BuildRequires: postgresql >= 7.2
Requires: postgresql-server >= 7.2
Requires: /sbin/chkconfig
Requires: mktemp
Obsoletes: IServerd-tvision, IServerd-tvision-devel
URL: http://iserverd.khstu.ru

%define	_with_name	iserverd
%define _with_uid	81

%description
IServerd is a Groupware ICQ server clone.
This program and the author are in no way affiliated with Mirabilis (AOL).
This program is designed as unix Groupware ICQ server since there is no 
Groupware ICQ server on a unix box. No reverse engineering or decompilation 
of any Mirabilis code took place to make this program.

%prep
%setup -q

%build
%configure \
    --with-etcdir=%{_sysconfdir}/%{_with_name} \
    --with-vardir=%{_localstatedir}/run/%{_with_name} \
    --with-logdir=%{_localstatedir}/log/%{_with_name} \
    --with-bindir=%{_bindir} \
    --with-sbindir=%{_sbindir} \
    --with-mandir=%{_mandir} \
    --with-pg-includes=`pg_config --includedir-server`
make

%install
rm -rf $RPM_BUILD_ROOT

mkdir -p $RPM_BUILD_ROOT%{_sbindir}
make DESTDIR=$RPM_BUILD_ROOT install

# logrotate script
mkdir -p $RPM_BUILD_ROOT/etc/logrotate.d
install -m644 script/iserverd.logrotate $RPM_BUILD_ROOT/etc/logrotate.d/iserverd
# service script
mkdir -p $RPM_BUILD_ROOT/etc/rc.d/init.d
install -m755 script/iserverd.sh.asp $RPM_BUILD_ROOT/etc/rc.d/init.d/iserverd

# rename config files
for i in $RPM_BUILD_ROOT%{_sysconfdir}/%{_with_name}/*.conf.default; do
	mv -f "${i}" "${i%.default}"
done

# fix for doc
find . -name "README.*.doc" -type f | xargs rm -f

%clean
rm -rf $RPM_BUILD_ROOT

%pre
/usr/bin/id iserverd > /dev/null 2>&1 || /usr/sbin/useradd \
				-M -o -r -d /etc/iserverd -u %{_with_uid} \
				-c "Groupware ICQ server clone" iserverd

%post
/sbin/chkconfig --add iserverd
/sbin/chkconfig iserverd off
cp /etc/services /etc/services.iserverd
cat /etc/services.iserverd | egrep -v "^icq" > /etc/services
echo >> /etc/services
echo -ne "icq\t\t4000/udp\n" >> /etc/services
echo -ne "icq\t\t5190/tcp\n" >> /etc/services
rm -f /etc/services.iserverd

%preun
if [ "$1" = "0" ]; then
    /sbin/chkconfig --del iserverd
fi
if [ -f /var/run/iserverd/iserverd.pid ]; then
    if [ -e /sbin/service ]; then
	/sbin/service iserverd stop > /dev/null
    else
	/etc/rc.d/init.d/iserverd stop > /dev/null
    fi
fi

%postun
if [ "$1" = "0" ]; then
    /usr/sbin/userdel iserverd
fi
if [ "$1" -ge "1" ]; then
    if [ -e /sbin/service ]; then
        /sbin/service iserverd condrestart > /dev/null 2>&1
    else
	/etc/rc.d/init.d/iserverd condrestart > /dev/null 2>&1
    fi
fi

%files
%defattr(-,root,root)
%doc BUGS CONTACT COPYRIGHT CREDITS README* TODO
%doc doc
/etc/logrotate.d/iserverd
%attr(755,root,root) /etc/rc.d/init.d/iserverd
%{_mandir}/man?/*
%attr(755,root,root) %{_bindir}/broadcast
%attr(550,postgres,postgres) %{_bindir}/convert_db.sh
%attr(550,postgres,postgres) %{_bindir}/db_manage.sh
%attr(755,root,root) %{_bindir}/db_check
%attr(755,root,root) %{_bindir}/db_convert
%attr(755,root,root) %{_bindir}/disconnect
%attr(755,root,root) %{_bindir}/online_cnt.cgi
%attr(755,root,root) %{_bindir}/post_mess.cgi
%attr(755,root,root) %{_bindir}/server_status
%attr(755,root,root) %{_bindir}/users_cnt.cgi
%attr(755,root,root) %{_bindir}/webpager
%attr(755,root,root) %{_sbindir}/iserverd
%attr(-,iserverd,iserverd) %config %{_sysconfdir}/%{_with_name}
%attr(-,iserverd,iserverd) %dir %{_localstatedir}/log/%{_with_name}
%attr(770,iserverd,iserverd) %dir %{_localstatedir}/run/%{_with_name}

%changelog
* Thu Jan 13 2005 Andy Shevchenko <andriy@asplinux.ru>
- update to new version

* Fri Jul 25 2003 Andy Shevchenko <andriy@asplinux.ru>
- add documentation fixes

* Tue Apr 29 2003 Andy Shevchenko <andriy@asplinux.ru>
- service is 'off' by default

* Fri Sep 21 2001 Andy Shevchenko <andy@smile.org.ua>
- upgrade to version 1.9.3
- added new man page

* Wed Apr 4 2001 Andy Shevchenko <andy@smcl.donetsk.ua>
- upgrade to version 0.10.9
- added web pages (see URL tag)

* Sun Feb 18 2001 Andy Shevchenko <andy@smcl.donetsk.ua>
- changed statical link of icontrol with TV library to dynamical
- expanded functionality of database/prepare.sh script
- added database/icquser script for manipulate icq users

* Sat Feb 17 2001 Andy Shevchenko <andy@smcl.donetsk.ua>
- changed uid from root (uid = 0) to iserverd (uid = 81)

* Thu Feb 1 2001 Leon Kanter <leon@blackcatlinux.com>
- added default config to fix "stop failed" after install

* Fri Jan 12 2001 Andy Shevchenko <andy@smcl.donetsk.ua>
- added some documentation, man pages

* Sun Nov 20 2000 Andy Shevchenko <andy@smcl.donetsk.ua>
- build rpm package


