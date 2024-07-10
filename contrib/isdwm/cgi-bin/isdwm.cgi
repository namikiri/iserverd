#!/usr/bin/perl

use DBI;
use Time::Local;
use Time::localtime;
	$Copyright = q(<hr width="250"><small><a href="http://iserverd.khstu.ru/isdwm/">IServerd Web Manager</a> v1.1<br>Copyright &copy; 2001-2002 by <a href="mailto:isdwm@nwnet.ru">A.V. Linetskiy</a><br></small>);
# ================ DEFAULTS
	$config_version = 3;
	$db_name = "users_db";
	$db_user = "iserverd";
	$db_pass = "password";
	$sendmailprog = "sendmail";
        $Admins = qq(1000,1001,1111);
	$MaxPageItems = 25;
	$AdminMail = "";
	$MailRegSubject = "����������� � ���� ICQ GroupWare";
	$MailTextSubject = qq|��� ���������������� � ���� ICQ GroupWare!
��� ����� (UIN): %uin%
������         : %pass%

�� ���� �������� ����������� � �������������� �������: %adminmail%|;
	$MainTableSize = 950;
	$LeftTableSize = 200;
	$InputBGColor = "#E4F3FF";
	$InputMouseOver = "#F6FBFF";
	$TableBorderColor = "#FFFFFF";
	$TableColor = "#F6FBFF";
	$RightTablesCellPadding = "2";
	$MainTableColor="#E4F3FF";
	$MainTableCellPadding = "0";
	$TableColorStrip = "#6699FF";
	$TextColor = "#000088";
	$BGColor = "#FFFFFF";
	$MenuSelectedColor="#6699FF";
	$MenuOverColor="#00CCFF";
	$WebCodepage="windows-1251";
	$MenuCellPadding = "3";
	$win2koi="yes";
	$koi2win="yes";
	$ShowISDStatus="yes";
	$FirstFreeUIN="";
	$BodyTDStyle = qq|
    font-family : Verdana, Arial, sans-serif;
    font-size : 9pt;
    scrollbar-face-color       : #48ADFB;
    scrollbar-highlight-color  : #eeeee1;
    scrollbar-shadow-color     : #959689;
    scrollbar-3dlight-color    : #000000;
    scrollbar-arrow-color      : #ffff00;
    scrollbar-track-color      : #48ADFB;
    scrollbar-darkshadow-color : #333c67|;
	$ALink=qq|
    color : #000000;
    text-decoration: underline|;
	$AHover=qq|
    text-decoration: none;
    color : #555555|;
	$ISD_control = "/etc/iserverd/iserverd.sh";
	$mail_reg = "yes";
	$default_pass = "isdrulez";
	$default_nick = "������� ��� ��� �����";
	$default_frst = "";
	$default_last = "";
	$default_lock_text = "��� ����� ������������ ��������������� �������";
	$MaxAddUIN = 2000; 
	$MinAddUIN = 1000;
	$URLToImages = "/img";
	$ThemeFile = "";
# ================ END DEFAULTS
	%MenuItems = (
		10  =>  (['������� ��������', '']),
		20  =>  (['���������� ������������', 'adduser']),
		30  =>  (['������ �������������', 'list']),
#		35  =>  (['��������� ���������', 'sysmess']),
		40  =>  (['�������� ������������', 'deluser']),
		50  =>  (['��������������� ������������', 'lockuser']),
		60  =>  (['����� ������������', 'newlock']),
		70  =>  (['������� �� �����������', 'regreq']),
		80  =>  (['������� �����������', 'online']),
		90  =>  (['���������� ���������', 'offmess']),
		100 =>  (['��������� �������', 'setup']),
#		105 =>  (['Postgres ��������', 'bench']),
		110 =>  (['����� �� �������', 'exit'])
	);
	%OnlineStatus = (
		0  =>  (['online.gif', 'Online']),
		1  =>  (['away.gif', 'Away']),
		2  =>  (['dnd.gif', 'DND']),
		4  =>  (['na.gif', 'N/A']),
		5  =>  (['na.gif', 'N/A']),
		16 =>  (['occupied.gif', 'Occupied']),
		17 =>  (['dnd.gif', 'DND']),
		32 =>  (['ffc.gif', 'Free For Chat']),
		256=>  (['invisible.gif', 'Invisible']),
		257=>  (['status.gif', 'Unknown']),
		258=>  (['offline.gif', 'Offline'])
	);
	%DesktopStatus = ( # ������ � ������������? ������ ��-������...
		 -1 =>  (['systray.gif', '��������� ����']),
		 0  =>  (['unknown.gif', '����������']),
		 1  =>  (['desktop.gif', '�� ��������'])
	);
	print "Content-type: text/html; charset=windows-1251\n";

	if ($ENV{'REQUEST_METHOD'} eq 'GET')
	{
		$query_string=$ENV{'QUERY_STRING'};
	} 
	elsif ($ENV{'REQUEST_METHOD'} eq 'POST') 
	{
		sysread(STDIN,$query_string,$ENV{'CONTENT_LENGTH'});
	};
	@formfields=split(/&/,$query_string);
		foreach (@formfields) 
		{
			my ($key, $value) = split(/=/,$_);
			$value=~s/\+/ /g;
			$value=~s/%([0-9A-H]{2})/pack('C',hex($1))/ge;
			$value=&win2koi($value);
			$in{$key}=$value;
		}
	if ($in{action} eq "savesettings")
	{
		($in{MailRegSubject},$in{MailTextSubject}) = &koi2win($in{MailRegSubject},$in{MailTextSubject});
		foreach (keys %in)
		{
#			$in{$_} =~ s/[^\\]([$|@|\|])/\\$1/g;
			$in{$_}=~s/\|/\\|/g if $in{$_} =~/[^\\]\|/g;
		}
		open (CONFIG, ">isdwm.conf") or die "$!";
		print CONFIG <<CONF__;
# IServerd Web Manager configuration file

\$config_version=$config_version;
\$db_name=q|$in{db_name}|;
\$db_user=q|$in{db_user}|;
\$db_pass=q|$in{db_pass}|;
\$Admins=q|$in{Admins}|;
\$ShowISDStatus=q|$in{ShowISDStatus}|;
\$MaxPageItems=q|$in{MaxPageItems}|;
\$AdminMail=q|$in{AdminMail}|;
\$MainTableSize=q|$in{MainTableSize}|;
\$LeftTableSize=q|$in{LeftTableSize}|;
\$InputBGColor=q|$in{InputBGColor}|;
\$InputMouseOver=q|$in{InputMouseOver}|;
\$TableBorderColor=q|$in{TableBorderColor}|;
\$TableColor=q|$in{TableColor}|;
\$RightTablesCellPadding=q|$in{RightTablesCellPadding}|;
\$MainTableColor=q|$in{MainTableColor}|;
\$MainTableCellPadding=q|$in{MainTableCellPadding}|;
\$TableColorStrip=q|$in{TableColorStrip}|;
\$BGColor=q|$in{BGColor}|;
\$MenuSelectedColor=q|$in{MenuSelectedColor}|;
\$MenuOverColor=q|$in{MenuOverColor}|;
\$WebCodepage=q|$in{WebCodepage}|;
\$MenuCellPadding=q|$in{MenuCellPadding}|;
\$win2koi=q|$in{win2koi}|;
\$koi2win=q|$in{koi2win}|;
\$sendmailprog=q|$in{sendmailprog}|;
\$BodyTDStyle=q|$in{BodyTDStyle}|;
\$ALink=q|$in{ALink}|;
\$AHover=q|$in{AHover}|;
\$ISD_control=q|$in{ISD_control}|;
\$mail_reg=q|$in{mail_reg}|;
\$default_pass=q|$in{default_pass}|;
\$default_nick=q|$in{default_nick}|;
\$default_frst=q|$in{default_frst}|;
\$default_last=q|$in{default_last}|;
\$default_lock_text=q|$in{default_lock_text}|;
\$MaxAddUIN=$in{MaxAddUIN}; 
\$MinAddUIN=$in{MinAddUIN};
\$FirstFreeUIN=q|$in{FirstFreeUIN}|;
\$URLToImages=q|$in{URLToImages}|;
\$ThemeFile=q|$in{ThemeFile}|;
\$TextColor=q|$in{TextColor}|;
\$MailRegSubject=q|$in{MailRegSubject}|;
\$MailTextSubject=q|$in{MailTextSubject}|;
CONF__
		close(CONFIG);
		chmod (0600, "isdwm.conf");
	}
	$CurrentConfig = $config_version;
	eval  {do "isdwm.conf"} || &Setup(1);
	if ($CurrentConfig != $config_version) { &Setup(3) };
        if ($ThemeFile ne "")
	{
		eval  {do "$ThemeFile"} || &Setup(2);
	}
	$InputParam = qq(Onmouseover="this.style.backgroundColor='$InputMouseOver'"; onmouseout="this.style.backgroundColor='$InputBGColor'";);

	$dbh = DBI->connect("dbi:Pg:dbname=$db_name", "$db_user", "$db_pass") || die "Cannot connect to database $db_name: $DBI::errstr";
	# ��������� ������.
	if (&ParseCook ne "succ")
	{
		$a = &CheckAccess ($in{'adminuin'}, $in{'adminpass'});
		if (($a ne "succ") || ($in{'adminuin'} eq "") || ($in{'adminpass'} eq ""))
		{
			&PrintEntryExit("���� � ������� ISD WM");
			print <<HTML__;
	<table border=0 bgcolor=$TableBorderColor cellpadding=3 cellspacing=1 width=220>
	<FORM ACTION="isdwm.cgi" METHOD="POST" name="login">
	  <tr bgcolor=$TableColorStrip><td colspan=2 align=center>���� � ������� ����������������� IServerd</td></tr>
	  <tr bgcolor=$TableColor><td>UIN:</td><td><INPUT TYPE="text" NAME="adminuin" SIZE="8" $InputParam></td></tr>
	  <tr bgcolor=$TableColor><td>������</td><td><INPUT TYPE="password" SIZE="8" NAME="adminpass" $InputParam></td></tr>
          <tr bgcolor=$TableColor><td><INPUT TYPE="SUBMIT" VALUE=" ���� " $InputParam></td><td><INPUT TYPE="CHECKBOX" NAME="remember" $InputParam> ���������</td></tr>
	</FORM>
	</table>
	<br>
	$Copyright
<SCRIPT LANGUAGE="JavaScript">
<!-- HIDE
document.login.adminuin.focus();
// STOP HIDING FROM OTHER BROWSERS -->
</SCRIPT>
</td><tr></table>
</body>
</HTML>
HTML__
exit;
		}
		else
		{
			&SetCookies;
		}
	}	

if ($in{action} eq "offmess") 
{
	&PrintHeader ("���������� ���������");
	&PrintMenu (90);
        if (exists $in{datefrom}) 
	{
		($datefrom=$in{datefrom}) =~ s/(\d+).(\d+).(\d+)/timelocal(0,0,0,$1,$2-1,$3-1900)/ge;
		unless (defined $datefrom) 
		{
			print "UIN $in{uin}: ������ � ��������� ����: <b>$in{datefrom}</b>. ��������� ����� � ��������� ����.";
			&PrintFooter;
			exit;
		}
		($dateto=$in{dateto}) =~ s/(\d+).(\d+).(\d+)/timelocal(59,59,23,$1,$2-1,$3-1900)/ge;
		unless (defined $dateto) 
		{
			print "UIN $in{uin}: ������ � ��������� ����: <b>$in{dateto}</b>. ��������� ����� � ��������� ����.";
			&PrintFooter;
			exit;
		}
		$addon="WHERE (msg_date>$datefrom AND msg_date<$dateto)";
	        if (exists $in{del}) 
		{
			$rv=$dbh->do("DELETE FROM users_messages $addon");
			print "������� ���������� ��������� � $in{datefrom} �� $in{dateto}.";
			&PrintFooter;
			exit;
		}
		$urlsearch = "&show=&datefrom=$in{datefrom}&dateto=$in{dateto}";
	}
	elsif ($in{do} eq "search")
	{
		$addon="WHERE msgtext ILIKE '\%$in{search}\%'";
		$search = &koi2win($in{search});
		$urlsearch = "&do=search&search=$search";
	}	
	if ($in{sort} eq "to_uin") { $sort="ORDER BY to_uin" }
		elsif ($in{sort} eq "from_uin") { $sort="ORDER BY from_uin" }
			elsif ($in{sort} eq "msg_type") { $sort="ORDER BY msg_type" }
				else { $sort="ORDER BY msg_date" };
	if (!$in{page}) {$in{page}=0;} else {$in{page}--};

	$PageS = $in{page}+1;
	{
		$LIMIT = qq(LIMIT $MaxPageItems, ).$MaxPageItems*$in{page};
	}
	$sth=$dbh->prepare("SELECT COUNT(to_uin) from users_messages $addon");
	$sth->execute;
	$TotalByQuery = $sth->fetchrow;
	$sth=$dbh->prepare("SELECT * FROM users_messages $addon $sort DESC $LIMIT ");
	$sth->execute;
	for ($i = 1; $i<($TotalByQuery/$MaxPageItems)+1; $i++) 
	{
		if ($PageS != $i) 
		{
			$page .= qq(<a href="isdwm.cgi?action=offmess&sort=$in{sort}&page=$i$urlsearch"><b>$i</b></a> );
		}
			else
		{
			$page .= "<b>$i </b>";
		}
	}

	print qq|
	<FORM ACTION="isdwm.cgi" METHOD="GET">
	<INPUT TYPE="hidden" NAME="action" VALUE="offmess">
	<table border=0 bgcolor="$TableBorderColor" cellpadding="$RightTablesCellPadding" cellspacing="1" width="100%">
	<tr bgcolor=$TableColorStrip><td colspan=5 align=center><b>���������� ���������</b></td></tr>
	<tr bgcolor=$TableColorStrip><td colspan=5>�������� ���: c <INPUT TYPE="text" NAME="datefrom" SIZE="10" VALUE="$in{datefrom}" $InputParam> �� <INPUT TYPE="text" NAME="dateto" VALUE="$in{dateto}" SIZE="10" $InputParam> <INPUT TYPE="SUBMIT" NAME="show" VALUE="�������" $InputParam> <INPUT TYPE="SUBMIT" NAME="del" VALUE="�������" $InputParam> <small>(��� �������������!)</small></td></tr></form>
	<FORM ACTION="isdwm.cgi" METHOD="GET">
	<INPUT TYPE="hidden" NAME="action" VALUE="offmess">
	<INPUT TYPE="hidden" NAME="do" VALUE="search">
	<tr bgcolor=$TableColorStrip><td colspan=5>�����: <INPUT TYPE="text" NAME="search" SIZE="10" VALUE="$search" $InputParam> <INPUT TYPE="SUBMIT" VALUE="������" $InputParam></td></tr>
	</FORM>
	<tr bgcolor=$TableColorStrip align="center"><td><a href="isdwm.cgi?action=offmess&page=1$urlsearch"><b>����</b></td><td width=1><a href="isdwm.cgi?action=offmess&sort=from_uin&page=1$urlsearch"><nobr><b>�� (UIN)</b></nobr></a></td><td width=1><a href="isdwm.cgi?action=offmess&sort=to_uin&page=1$urlsearch"><nobr><b>��� (UIN)</b></nobr></a></td><td width=1><a href="isdwm.cgi?action=offmess&sort=msg_type&page=1$urlsearch"><b>���</b></a></td><td><b>���������</b></td></tr>|;
	if ($sth->rows != 0)
	{
		while (@rows=$sth->fetchrow)
		{
			@rows = &koi2win(@rows);
			print qq|<tr bgcolor=$TableColor align="center"><td heigth="1"><nobr>|.sprintf ("%02d:%02d <b>%02d.%02d.%d</b>",($t = localtime($rows[2]))->hour, $t->min, $t->mday, $t->mon+1, $t->year+1900).qq|</nobr></td><td align="center"><a href="isdwm.cgi?action=edituin&uin=$rows[1]">$rows[1]</a></td><td><a href="isdwm.cgi?action=edituin&uin=$rows[0]">$rows[0]</a></td><td>|;
			if ($rows[3]==1)
			{
				print qq(<img src="$URLToImages/message.gif" border=0 width="16" height="16" alt="Message">);
				$msg="$rows[4]";
			}
			elsif ($rows[3]==4)
			{
				print qq(<img src="$URLToImages/url.gif" border=0 width="16" height="16" alt="URL">);
				$msg = sprintf("�����: %s<br>URL: <a href=\"%s\">%s </a>", (@_=split(/�/,$rows[4])), $_[1]);
			}
			elsif ($rows[3]==6)
			{
				print qq(<img src="$URLToImages/auth.gif" border=0 width="16" height="16" alt="Authorise">);
				$msg = sprintf("%s %s %s %s", (@_=split(/�/,$rows[4])));
			}
			elsif ($rows[3]==12)
			{
				print qq(<img src="$URLToImages/regreq.gif" border=0 width="16" height="16" alt="Registration request">);
				$msg = sprintf("%s %s %s %s", (@_=split(/�/,$rows[4])));
			}
			elsif ($rows[3]==20)
			{
				print qq(<img src="$URLToImages/system.gif" border=0 width="16" height="16" alt="System message">);
				$msg = sprintf("%s %s %s %s %s %s", (@_=split(/�/,$rows[4])));
			}
			elsif ($rows[3]==13)
			{
				print qq(<img src="$URLToImages/web.gif" border=0 width="16" height="16" alt="Web message">);
				$msg = $rows[4];
			}
			else 
			{
				print qq(<img src="$URLToImages/status.gif" border=0 width="16" height="16" alt="����������� ��� [$rows[3]]">);
				$msg = $rows[4];
			}	
			print qq|</td><td align="left">$msg</td></tr>|;
		}
	}
	print qq|<tr><td colspan=5>��������: $page</td></tr></table>|;
	&PrintFooter; exit;
}	
if ($in{action} eq "online")
{
	&PrintHeader ("���������� ���������");
	&PrintMenu (80);
	$sth=$dbh->prepare("SELECT * from online_users ORDER BY uin");
	$sth->execute;
	$count=$sth->rows;
	print qq|
	<table border=0 bgcolor="$TableBorderColor" cellpadding="$RightTablesCellPadding" cellspacing="1" width="100%">
	<tr bgcolor=$TableColorStrip><td colspan=6 align=center><b>������� ����������� ($count)</b></td></tr>
	<tr bgcolor=$TableColor align=center><td><b>UIN</b></td><td><b>IP</b></td><td><b>V</b></td><td><b>����� �����. (���)</b></td><td><b>�������</b></td><td><b><img src="$URLToImages/status.gif" border=0 width="16" height="16" alt="������ ������������"></b></td></tr>|;
	while (@rows=$sth->fetchrow)
	{
		$rows[1]=GetIP($rows[1]);
		$rows[5] = int((time-$rows[5])/60);
		print qq|<tr bgcolor=$TableColor align=center><td><a href="isdwm.cgi?action=edituin&uin=$rows[0]">$rows[0]</a></td><td>$rows[1]</td><td>$rows[8]</td><td>$rows[5]</td><td>|;
		if ($rows[8] == 3) {  print qq|<img src="$URLToImages/$DesktopStatus{$rows[12]}->[0]" border=0 width="16" height="16" alt="$DesktopStatus{$rows[12]}->[1] [$rows[12]]">| }
		else { print "-" }; print "</td><td>";
		&PrintOnlineStatus($rows[4]); 
		print qq|</td></tr>|;
	}
	print "</table>";
	&PrintFooter; exit;
}
if ($in{action} eq "edituin") 
{

	if ($in{do} eq "submit")
	{
		$in{llog} =~ s/(\d{2}):(\d{2}) (\d{2}).(\d{2}).(\d{4})/$llog_time = timelocal(0,$2,$1,$3,$4-1,$5-1900)/ge;
		unless (defined $llog_time) 
		{
			&PrintHeader ("������ ��������� ������ � #$in{uin}");
			&PrintMenu (30);
			print "UIN $in{uin}:������ ��������� ������. ������������ ���� ���������� ����� � �������: <b>$in{llog}</b>. ��������� ����� � ��������� ����.";
			&PrintFooter;
			exit;
		}
		$in{cdate} =~ s/(\d{2}):(\d{2}) (\d{2}).(\d{2}).(\d{4})/$cr_time = timelocal(0,$2,$1,$3,$4-1,$5-1900)/ge;
		unless ($cr_time) 
		{
			&PrintHeader ("������ ��������� ������ � #$in{uin}");
			&PrintMenu (30);
			print "UIN $in{uin}:������ ��������� ������. ������������ ���� ��������: <b>$in{cdate}</b>. ��������� ����� � ��������� ����.";
			&PrintFooter;
			exit;
		}
		if ($in{'auth'} ne "0") { $in{'auth'} = 1 };
		$rv=$dbh->do("UPDATE users_info_ext SET pass='$in{pass}', llog='$llog_time', bcst='$in{bcst}', cdate='$cr_time', cpass='$in{cpass}', nick='$in{nick}', frst='$in{frst}', last='$in{last}', email1='$in{email1}', email2='$in{email2}', email3='$in{email3}', e1publ='$in{e1publ}', auth='$in{auth}', sex='$in{sex}', age='$in{age}', notes='$in{notes}' where uin=$in{uin}");
		if ($rv != 1)
		{
			&PrintHeader ("������ ��������� ������ � #$in{uin}");
			&PrintMenu (30);
			print "UIN $in{uin}:������ ��������� ������. ��������� ����� � ��������� ����";
			&PrintFooter;
			exit;
		}
		&PrintHeader ("������ ������� ��������. ($in{uin})");
		&PrintMenu (30);
		print qq(<b>UIN $in{uin}: ������ ������� ��������.</b> <BR> <a href="isdwm.cgi?action=edituin&uin=$in{uin}">������� �����</A>, ����� ��������� � �������������� � ��������� ��������� ���������.);
		&PrintFooter;
		exit;
	}
	$sth=$dbh->prepare("SELECT * from users_info_ext WHERE uin=$in{uin}");	
	$sth->execute;
	$Result = $sth->rows;
	if ($Result != 1) {
		&PrintHeader ("������!");
		&PrintMenu (30);
		print "<h2>������! ������������ � ����� ������� ($in{uin}) �� ����������!</h2>";
		&PrintFooter; exit;
	}
	@rows=$sth -> fetchrow;
	&PrintHeader ("�������������� ������������ #$in{uin}");
	&PrintMenu (30);
	@rows = &koi2win(@rows);
	$rows[4]=GetIP($rows[4]);
	print qq|
	<FORM ACTION="isdwm.cgi" METHOD="POST">
	<INPUT TYPE="hidden" NAME="action" VALUE="edituin">
	<INPUT TYPE="hidden" NAME="do" VALUE="submit">
	<INPUT TYPE="hidden" NAME="uin" VALUE="$rows[0]">
	<table border=0 bgcolor="$TableBorderColor" cellpadding="$RightTablesCellPadding" cellspacing="1" width="100%">
		<tr><td bgcolor="$TableColorStrip" colspan="2" align="center"><b>�������������� ���������� #$rows[0]</b></td></tr>
		<tr bgcolor="$TableColor"><td>������:</td><td><INPUT TYPE="text" NAME="pass" VALUE="$rows[1]" $InputParam></td></tr>
		<tr bgcolor="$TableColor"><td>������ <a href="isdwm.cgi?action=newlock&uin=$rows[0]">����������</a> UIN'�:</td><td>|; if ($rows[2]==1) {print "������������";} else {print "���������� �����������"} print qq|</td></tr>
		<tr bgcolor="$TableColor"><td>���� � ����� ���������� ����� � �������:</td><td><INPUT TYPE="text" NAME="llog" VALUE="|; printf ("%02d:%02d %02d.%02d.%d",($t = localtime($rows[3]))->hour, $t->min, $t->mday, $t->mon+1, $t->year+1900); print qq|" $InputParam></td></tr>|;
		if ($rows[3] != 0) 
		{
			print qq|	
		<tr bgcolor="$TableColor"><td>��������� ���� � IP:</td><td>$rows[4]</td></tr>|;
		}
		print qq|
		<tr bgcolor="$TableColor"><td>����� �� ����� ����������������� ���������:</td><td><INPUT TYPE="CHECKBOX" NAME="bcst" VALUE="1"|; if ($rows[5] == 1) {print " checked";} print qq| $InputParam> ��</td></tr>
		<tr bgcolor="$TableColor"><td>���� � ����� �������� ������:</td><td><INPUT TYPE="text" NAME="cdate" VALUE="|; printf ("%02d:%02d %02d.%02d.%d",($t = localtime($rows[6]))->hour, $t->min, $t->mday, $t->mon+1, $t->year+1900); print qq|" $InputParam></td></tr>
		<tr bgcolor="$TableColor"><td>��� ��������� ����� � ������� ��������� ������� ������:</td><td><INPUT TYPE="CHECKBOX" NAME="cpass" VALUE="1"|; if ($rows[7] == 1) {print " checked";} print qq| $InputParam> ��</td></tr>
		<tr bgcolor="$TableColor"><td>���:</td><td><INPUT TYPE="text" NAME="nick" VALUE="$rows[8]" $InputParam></td></tr>
		<tr bgcolor="$TableColor"><td>���:</td><td><INPUT TYPE="text" NAME="frst" VALUE="$rows[9]" $InputParam></td></tr>
		<tr bgcolor="$TableColor"><td>�������:</td><td><INPUT TYPE="text" NAME="last" VALUE="$rows[10]" $InputParam></td></tr>
		<tr bgcolor="$TableColor"><td>E-mail 1:</td><td><INPUT TYPE="text" NAME="email1" VALUE="$rows[11]" $InputParam></td></tr>
		<tr bgcolor="$TableColor"><td>E-mail 2:</td><td><INPUT TYPE="text" NAME="email2" VALUE="$rows[12]" $InputParam></td></tr>
		<tr bgcolor="$TableColor"><td>E-mail 3:</td><td><INPUT TYPE="text" NAME="email3" VALUE="$rows[13]" $InputParam></td></tr>
		<tr bgcolor="$TableColor"><td>����������� 1-� e-mail:</td><td><INPUT TYPE="CHECKBOX" NAME="e1publ" VALUE="1"|; if ($rows[14] == 1) {print " checked";} print qq| $InputParam> ��</td></tr>
		<tr bgcolor="$TableColor"><td>��������� ����������� ��� ���������� ����� ������������:</td><td><INPUT TYPE="CHECKBOX" NAME="auth" VALUE="0"|; if ($rows[16] == 0) {print " checked";} print qq| $InputParam> ��</td></tr>
		<tr bgcolor="$TableColor"><td>���:</td><td><INPUT TYPE="RADIO" NAME="sex" VALUE="1"|; if ($rows[17]==1) {print " checked";} print qq|>� <INPUT TYPE="RADIO" NAME="sex" VALUE="2"|; if ($rows[17]==2) {print " checked";} print qq|>� <INPUT TYPE="RADIO" NAME="sex" VALUE="0"|; if ($rows[17]==0) {print " checked";} print qq| $InputParam>?</td></tr>
		<tr bgcolor="$TableColor"><td>�������:</td><td><INPUT TYPE="text" NAME="age" VALUE="$rows[18]" $InputParam></td></tr>
		<tr bgcolor="$TableColor"><td>���� ��������:</td><td><INPUT TYPE="text" NAME="bthd" VALUE="|; printf("%02d.%02d.%04d",$rows[19],$rows[20],$rows[21]+1900); print qq|" $InputParam></td></tr>
		<tr bgcolor="$TableColor"><td colspan=2>�������:<BR><TEXTAREA NAME="notes" ROWS=4 COLS=50 $InputParam>$rows[35]</TEXTAREA></td></tr>
		<tr bgcolor="$TableColor"><td>�������������:</td><td><a href="isdwm.cgi?action=deluser&uin=$in{uin}">�������</a> / <a href="isdwm.cgi?action=newlock&uin=$in{uin}">�����������</a></td></tr>
		<tr bgcolor="$TableColor"><td colspan="2" align="right"><INPUT TYPE="SUBMIT" VALUE=" �������� " $InputParam></td></tr>
	</table>
	</form>|;
	&PrintFooter; exit;
}
if ($in{action} eq "list") 
{
	&PrintHeader ("������ �������������");
	&PrintMenu (30);
	if ($in{sort} eq "nick") { $sort="ORDER BY nick" }
		elsif ($in{sort} eq "fname") { $sort="ORDER BY frst" }
			elsif ($in{sort} eq "lname") { $sort="ORDER BY last" }
				elsif ($in{sort} eq "email") { $sort="ORDER BY email1" }
					else { $sort="ORDER BY uin" };

	if (!$in{page}) {$in{page}=0;} else {$in{page}--};

	$PageS = $in{page}+1;
	{
		$LIMIT = qq(LIMIT $MaxPageItems, ).$MaxPageItems*$in{page};
	}
	
	$sth=$dbh->prepare("SELECT COUNT(uin) from users_info_ext");	
	$sth->execute;
	$TotalByQuery = $sth->fetchrow;
	$sth=$dbh->prepare("SELECT uin,nick,frst,last,pass,email1 from users_info_ext $sort $LIMIT");
	$sth->execute;
	$online=$dbh->prepare("SELECT uin, stat from online_users");
	$online->execute;
	$online_hash = $online -> fetchall_hashref('uin');
	for ($i = 1; $i<($TotalByQuery/$MaxPageItems)+1; $i++) 
	{
		if ($PageS != $i) 
		{
			$page .= qq(<a href="isdwm.cgi?action=list&sort=$in{sort}&page=$i"><b>$i</b></a> );
		}
			else
		{
			$page .= "<b>$i </b>";
		}

	}
	print qq(	<table border=0 bgcolor=$TableBorderColor cellpadding="$RightTablesCellPadding" cellspacing=1 width=100%>
	<tr bgcolor=$TableColorStrip><td colspan=7 align="center"><b>������ �������������</b></td></tr>
	<FORM ACTION="isdwm.cgi" METHOD="GET">
	<tr bgcolor=$TableColorStrip><td colspan=7>������� ������, UIN: <INPUT TYPE="hidden" NAME="action" VALUE="edituin"><INPUT TYPE="text" NAME="uin" SIZE="8" $InputParam> <INPUT TYPE="SUBMIT" VALUE="�������������" $InputParam></td></tr></form>
	<tr bgcolor=$TableColorStrip align="center"><td><a href="isdwm.cgi?action=list&sort=uin&page=1"><b>UIN</b></a></td><td><a href="isdwm.cgi?action=list&sort=nick&page=1"><b>���</b></a></td><td><a href="isdwm.cgi?action=list&sort=fname&page=1"><b>���</b></a></td><td><a href="isdwm.cgi?action=list&sort=lname&page=1"><b>�������</b></a></td><td><b>������*</b></td><td><a href="isdwm.cgi?action=list&sort=email&page=1"><b>E-mail</b></a></td><td><img src="$URLToImages/status.gif" border=0 width="16" height="16" alt="������ ������������"></td></tr>);
	while($hash_ref = $sth->fetchrow_hashref) 
	{
		&WriteUser ($hash_ref->{uin},$hash_ref->{nick},$hash_ref->{frst},$hash_ref->{last},$hash_ref->{pass},$hash_ref->{email1}, $online_hash->{$hash_ref->{uin}}->{stat});
	}
	print qq|<tr><td colspan=7>��������: $page</td></tr>|;
	print qq|<tr><td colspan=7><small>* ����������: � ����� �������������, ������ ����� ���� ����. �������� ������ ������, ����� ������� ������.</small></td></tr></table>|;
	&PrintFooter; exit;
}
if ($in{action} eq "adduser") 
{
	&PrintHeader ("���������� ������ ������������");
	&PrintMenu (20);
	if ($in{do} eq "submit") 
	{
		if ($in{'cpass'} eq "on") { $cpass=1 } else { $cpass=0 };
		if ($in{'auth'} eq "on") { $auth=0 } else { $auth=1 };
		$time=time;
		$sth=$dbh->prepare("INSERT INTO users_info_ext (uin, pass, nick, frst, last, email1, cdate, cpass, auth, sex) values ($in{uin}, '$in{password}', '$in{nick}', '$in{frst}', '$in{last}', '$in{email}', $time, $cpass, $auth, '$in{sex}')");
		$sth->execute;
		$result = $sth->rows;
		if ($result==-2) { print "<b>������ ���������� ������������! ��������, UIN ��� ���������� ��� �������� ������������ ������</b>"; }
		elsif ($result==1) { print qq(<b>������������ #$in{uin} ��������</b> <br><a href="isdwm.cgi?action=edituin&uin=$in{uin}">������� �����</A>, ����� ������� � ��������������.); }
		else { print "<b>����������� ��������� ���������� ��������. ��, ��������, UIN ������.</b>"; }
		if ($in{sendmail} eq "on") 
		{
			unless (($in{email} =~ /.*\@.*\..*/) || ($in{email} =~ tr/;<>*|`&$!#()[]{}:'"//))
			{
				print "<br>� ������ � ���������� � ����������� ������������ �� ����������� - ������ � ���������� e-mail...";
			}
			else
			{
				unless (open (MAIL, "|$sendmailprog -t")) 
				{
					print qq(<br>����������: ���������� ������� �������� ���������: <b>$sendmailprog</b>.  ��������� <a href="isdwm.cgi?action=setup">���������</a>!);
					&PrintFooter;
					exit
				};
				print MAIL "To: $in{email}\n";
				print MAIL "From: $AdminMail\n";
				print MAIL "Subject: $MailRegSubject\n\n";
				$MailTextSubject =~ s/%uin%/$in{uin}/gi;
				$MailTextSubject =~ s/%pass%/$in{password}/gi;
				$MailTextSubject =~ s/%adminmail%/$AdminMail/gi;
				print MAIL "$MailTextSubject";
				close (MAIL);
			}
		}
		&PrintFooter;
		if ($in{regreq} eq "true") 
		{
			$rv=$dbh->do("DELETE FROM register_requests WHERE uin=$in{ruin}");
		}
		exit;
	}
	$MaxUIN = &GetFreeUIN;
	if ($in{'nick'}) {$default_nick = $in{'nick'};}
	if ($in{'frst'}) {$default_frst = $in{'frst'};}	
	if ($in{'last'}) {$default_last = $in{'last'};}	
	&PrintRegForm ($MaxUIN, $default_pass, $default_nick, $default_frst, $default_last, $in{email1}, $in{sex}, $in{regreq});
	&PrintFooter;
	exit;
};
if ($in{action} eq "regreq") 
{
	&PrintHeader ("������ �������� �� �����������");
	&PrintMenu (70);
	if ($in{do} eq "delreq")
	{
		$rv=$dbh->do("DELETE FROM register_requests WHERE uin=$in{uin}");
		if ($rv != 1)
		{
			print ("<b>������ �������� ���������������� ������� #$in{uin}</b>");
		}
		else
		{
			print qq(<b>������ �� ����������� #$in{uin} ������� ������!</b><br><br><a href="isdwm.cgi?action=regreq">��������� �� �������� �������� �� �����������</a>);
		}
		&PrintFooter;
		exit;
	}
	$sth=$dbh->prepare("SELECT uin,nick,frst,last,cdate,email1,notes,sex from register_requests");
	$sth->execute;
	$req=$sth->rows;
	print qq(<table border=0 bgcolor=$TableBorderColor cellpadding="$RightTablesCellPadding" cellspacing=1 width=100%>
	<tr bgcolor=$TableColorStrip><td colspan=7 align=center><b>������� �� ����������� ($req)</b></td></tr>
	<tr bgcolor=$TableColorStrip><td width="1"><nobr>���� �������</nobr></td><td width="1">���</td><td width="1">���</td><td width="1">���.</td><td width="1"><nobr>e-mail</nobr></td><td>������� �������:</td><td width="1">��������</td></tr>);
	while(@row=$sth -> fetchrow) 
	{
		@row=&koi2win(@row);
		$date = sprintf ("%02d:%02d %02d.%02d.%d",($t = localtime($row[4]))->hour, $t->min, $t->mday, $t->mon+1, $t->year+1900);
		print qq(<tr bgcolor=$TableColor><td width="1">$date</td><td width="1">$row[1]</td><td width="1">$row[2]</td><td width="1">$row[3]</td><td width="1">$row[5]</td><td>$row[6]</td><td width="1"><a href="isdwm.cgi?action=adduser&nick=$row[1]&frst=$row[2]&last=$row[3]&email1=$row[5]&sex=$row[7]&regreq=true">�����������</a> <a href="isdwm.cgi?action=regreq&do=delreq&uin=$row[0]">�������</a></td></tr>);
	}
	print "</table>"; &PrintFooter; exit;
}

if ($in{action} eq "exit") 
{
	&ClearCookies;
	&PrintEntryExit("����� ������ ��������");
	print qq(<table><tr><td align=center><b>�� ����� �� �������!</b><br>������� �������, �������� ���...<br><br>��������� ���� ��������� ����� <a href="isdwm.cgi">��������������</a>.$Copyright</td></tr></table>);
	print <<HTML__;
</body>
</html>
HTML__
	exit;
}
if ($in{action} eq "lockchangetext") 
{
	&PrintHeader ("������������ ������������");
	&PrintMenu(50);
	$rv=$dbh->do("UPDATE users_lock SET lck_text='$in{lck_text}' WHERE luin=$in{uin}");
	if ($rv != 1) 
	{
		print "<b>������ ��������� ����������� � ���������� UIN'� #$in{uin}!<b>";
	}
	else
	{
		print "<b>���������� ������� �������� (UIN #$in{uin})</b><br><br>";
		print qq(<a href="isdwm.cgi?action=lockuser">��������� � ������ ��������������� UIN'��</a><br>
		<a href="isdwm?action=lockuser&do=lockedit&uin=$in{uin}">��������� � �������������� UIN #$in{uin}</a>);
	}
	&PrintFooter; exit;
}

if ($in{action} eq "newlock") 
{
	&PrintHeader ("������������ ������������");
	&PrintMenu(60);
	if ($in{do} eq "lock")
	{
		$rv=$dbh->do("UPDATE users_info_ext SET ulock=1 WHERE uin=$in{uin}");
		if ($rv != 1) 
		{
			print "���������� ������������� UIN #$in{uin}!"; &PrintFooter; exit;
		}
		if ($in{lck_text} ne "")
		{
			$rv=$dbh->do("UPDATE users_lock SET lck_text='$in{lck_text}' where luin=$in{uin}");
			if ($rv !=1) 
			{
				$rv=$dbh->do("INSERT INTO users_lock VALUES ($in{uin},'$in{lck_text}')");
			}			
		}
		print qq(<b>UIN #$in{uin} ������� ������������!</b><br><br><a href="isdwm.cgi?action=lockuser">������� � ������ ��������������� �������������</a>);
		&PrintFooter; exit;
	}
	$default_lock_text=&koi2win($default_lock_text);
	print qq(
	<FORM ACTION="isdwm.cgi" METHOD="POST">
	<INPUT TYPE="HIDDEN" NAME="action" VALUE="newlock">
	<INPUT TYPE="HIDDEN" NAME="do" VALUE="lock">
	<table border=0 bgcolor=$TableBorderColor cellpadding="$RightTablesCellPadding" cellspacing=1 width=200 align=center>
	<tr bgcolor=$TableColorStrip><td colspan=2 align=center><b>������������� ����� UIN</b></td></tr>
	<tr bgcolor=$TableColor><td>UIN:</td>
	<td><INPUT TYPE="TEXT" NAME="uin" VALUE="$in{uin}" size=7 $InputParam></td></tr>
	<tr bgcolor=$TableColor><td>������� ����������:</td><td><TEXTAREA NAME="lck_text" ROWS=5 COLS=50 $InputParam>$default_lock_text</TEXTAREA></td></tr>
	<tr bgcolor=$TableColor><td>&nbsp;</td><td><INPUT TYPE="SUBMIT" VALUE="�������������" $InputParam></td></tr></form></table>);
	&PrintFooter; exit;
}

if ($in{action} eq "lockremove")
{
	&PrintHeader ("������������ ������������");
	&PrintMenu(50);
	$rv=$dbh->do("UPDATE users_info_ext SET ulock=0 where uin=$in{uin}") || warn "���������� �������� ������� �������������!";
	$rv=$dbh->do("DELETE FROM users_lock where luin=$in{uin}");
	if ($rv != 1)
	{
		print "����������� � ���������� UIN #$in{uin} �����������! �������.<BR><br>";
	}
	print "<b>���������� � UIN #$in{uin} ������� �����!</b><br><br>";
	print qq(<a href="isdwm?action=lockuser">��������� � ������ ��������������� UIN'��</a></b>);
	&PrintFooter; exit;
}

if ($in{action} eq "lockuser")
{
	if ($in{do} eq "lockedit")
	{
		$addon = "uin=$in{uin}";
	}
	else
	{
		$addon = "ulock=1";
	}
	$sth=$dbh->prepare("SELECT uin,nick,frst,last,email1 FROM users_info_ext WHERE $addon");
	$sth->execute;
	$count=$sth->rows;
	$sth2=$dbh->prepare("SELECT * FROM users_lock");
	$sth2->execute;
	$ban_hash = $sth2 -> fetchall_hashref('luin');
	if ($in{do} eq "lockedit")
	{
		&PrintHeader ("�������������� ���������������� ������������");
		&PrintMenu(50);
		@row=$sth -> fetchrow;
		@row=koi2win(@row);
		$ban_comments = &koi2win($ban_hash->{$row[0]}->{lck_text});
		print qq|
		<FORM ACTION="isdwm.cgi" METHOD="POST">
		<table border=0 bgcolor=$TableBorderColor cellpadding="$RightTablesCellPadding" cellspacing=1 width=100% align=center>
		<tr bgcolor=$TableColorStrip><td colspan=5 align=center><b>��������� ���������� ����������</b></td></tr>
		<tr bgcolor=$TableColor><td><b>UIN</b></td><td><b>���</b></td><td><b>���</b></td><td><b>�������</b></td><td><b>E-mail</b></td></tr>		<tr bgcolor=$TableColor><td><a href="isdwm.cgi?action=edituin&uin=$row[0]">$row[0]</a></td><td>$row[1]</td><td>$row[2]</td><td>$row[3]</td><td>$row[4]</td></tr>
		<tr bgcolor=$TableColor><td colspan=5 align=left>
		<BR>����� ����������:<BR>
		<INPUT TYPE="HIDDEN" NAME="action" VALUE="lockchangetext">
		<INPUT TYPE="HIDDEN" NAME="uin" VALUE="$row[0]">
		<TEXTAREA NAME="lck_text" ROWS=5 COLS=50 $InputParam>$ban_comments</TEXTAREA><BR><INPUT TYPE="SUBMIT" VALUE="��������" $InputParam> [ <a href="isdwm.cgi?action=lockremove&uin=$row[0]">c���� ����������</a> \| <a href="isdwm.cgi?action=deluser&uin=$row[0]">������� ������������</a> ]
		</td></tr></table></FORM>|;
		&PrintFooter; exit;
	}
	&PrintHeader ("������ ��������������� �������������");
	&PrintMenu(50);
	print qq|
	<FORM ACTION="isdwm.cgi" METHOD="POST"><INPUT TYPE="HIDDEN" NAME="action" VALUE="dobanuser">
	<table border=0 bgcolor=$TableBorderColor cellpadding="$RightTablesCellPadding" cellspacing=1 width=100% align=center>
	<tr bgcolor=$TableColorStrip><td colspan=7 align=center><b>������ ��������������� UIN'�� ($count)</b></td></tr>|;
	if ($count != 0) {
		print qq|<tr bgcolor=$TableColor align=center><td width="1">UIN</td><td width="1">���</td><td width="1">���</td><td width="1">�������</td><td width="1"><nobr>E-mail</nobr></td><td>�����������</td><td width="1"><nobr>��./������</nobr></td></tr>|;
		while(@row=$sth -> fetchrow) 
		{
			@row=koi2win(@row);
			$ban_comments = &koi2win($ban_hash->{$row[0]}->{lck_text});
			print qq(<tr bgcolor=$TableColor><td>$row[0]</td><td>$row[1]</td><td>$row[2]</td><td>$row[3]</td><td>$row[4]</td><td>$ban_comments</td><td width="1"><a href="isdwm.cgi?action=lockuser&do=lockedit&uin=$row[0]">�����..</a></td></tr>);
		}
	} else {
		print qq(<tr bgcolor=$TableColor><td colspan=7 align=center><b>������ ����</b></td></tr>);
	}
	print "</table>";
	&PrintFooter;
	exit;
}
if ($in{action} eq "setup") 
{
	&Setup;
}
if ($in{action} eq "deluser") 
{
	if (($in{do} eq "delete") || ($in{del} eq "delete"))
	{
		&PrintHeader ("�������� ������������ #$in{uin}");
		&PrintMenu(40);
		if ($in{uin} =~ /\D/) 
		{
			print "������ �������� ������������! ������ ���� ���� �������?";
			&PrintFooter;
			exit;
		}
		$rv=$dbh->do("DELETE FROM users_info_ext WHERE uin=$in{uin}");
		if ($rv != 1)
		{
			print "������ �������� UIN #$in{uin}! ������ ������ �� ����������!";
		} 
			else
		{
			$rv=$dbh->do("DELETE FROM users_messages WHERE to_uin=$in{uin} OR from_uin=$in{uin}");
			$rv=$dbh->do("DELETE FROM users_lock WHERE luin=$in{uin}");
			print qq(<b>UIN #$in{uin} ������� ������!</b><br><br><a href="isdwm.cgi?action=list">������� � ������ �������������</a>);
		}
		&PrintFooter;
		exit;
	}

	if ($in{do} eq "confirm")
	{
		&PrintHeader ("������������� �������� ������������ #$in{uin}");
		&PrintMenu(40);
		$sth=$dbh->prepare("SELECT nick,frst,last,email1,llog FROM users_info_ext WHERE uin=$in{uin}");
		$sth->execute;
		@row=$sth -> fetchrow;
		if (@row ==0) {print "������ ������ �� ����������. ��������� ����!"; print @Footer; exit;}
		@row=koi2win(@row);
		print qq(
		<FORM ACTION="isdwm.cgi" METHOD="GET">
		<INPUT TYPE="HIDDEN" NAME="action" VALUE="deluser">
		<INPUT TYPE="HIDDEN" NAME="do" VALUE="delete">
		<INPUT TYPE="HIDDEN" NAME="uin" VALUE="$in{uin}">
		<table border=0 bgcolor=$TableBorderColor cellpadding="$RightTablesCellPadding" cellspacing=1 width=450 align=center>
		<tr bgcolor=$TableColorStrip><td colspan=7 align=center><b>������������� �������� ������������ #$in{uin}</b></td></tr>
		<tr bgcolor=$TableColor align=center><td>���</td><td>���</td><td>�������</td><td>E-mail</td><td>��������� ����</td></tr>
	        <tr bgcolor=$TableColor><td>$row[0]</td><td>$row[1]</td><td>$row[2]</td><td>$row[3]</td><td align="right">); printf ("%02d:%02d %02d.%02d.%d",($t = localtime($rows[6]))->hour, $t->min, $t->mday, $t->mon+1, $t->year+1900); print qq(</td></tr>
		<tr bgcolor=$TableColor><td colspan=5 align=center><INPUT TYPE="SUBMIT" TYPE="NAME" VALUE="�������" $InputParam></td></tr></form></td></tr></table>);
		&PrintFooter;
		exit;
	}
	&PrintHeader ("�������� ������������");
	&PrintMenu(40);
	print qq(
	<FORM ACTION="isdwm.cgi" METHOD="GET">
	<INPUT TYPE="HIDDEN" NAME="action" VALUE="deluser">
	<INPUT TYPE="HIDDEN" NAME="do" VALUE="confirm">
	<table border=0 bgcolor=$TableBorderColor cellpadding="$RightTablesCellPadding" cellspacing=1 width=350 align=center>
	<tr bgcolor=$TableColorStrip><td colspan=7 align=center><b>�������� ������������</b></td></tr>
	<tr bgcolor=$TableColor><td>UIN ��� ��������:</td><td><INPUT TYPE="TEXT" SIZE=8 VALUE="$in{uin}" NAME="uin" $InputParam>
	<INPUT TYPE="SUBMIT" TYPE="NAME" VALUE="����������" $InputParam><br><INPUT TYPE="CHECKBOX" NAME="del" value="delete" $InputParam> ������� �����</td></tr>
	</table>
	</FORM>);
	&PrintFooter;
	exit;
}
#########################################################################
#			MAIN PAGE
#########################################################################

	&PrintHeader ("������� �������� - ISD WM ");
	&PrintMenu;
	if ($ShowISDStatus eq "yes")
	{
		$pid = `$ISD_control status`;
		$sth=$dbh->prepare("SELECT COUNT (uin) from users_info_ext");
		$sth->execute;
		$TotalCount = $sth->fetchrow_array;
		$sth=$dbh->prepare("SELECT COUNT (uin) from online_users");
		$sth->execute;
		$OnlineCount = $sth->fetchrow_array;
		$sth=$dbh->prepare("SELECT COUNT (uin) FROM register_requests");
		$sth->execute;
		$RRequestCount = $sth->fetchrow_array;
		$sth=$dbh->prepare("SELECT COUNT (to_uin) FROM users_messages");
		$sth->execute;
		$MessagesCount= $sth->fetchrow_array;
		$sth=$dbh->prepare("SELECT COUNT (ulock) FROM users_info_ext where ulock=1");
		$sth->execute;
		$UlockCount= $sth->fetchrow_array;
		if ($pid =~ /stopped/) {$pid = "����������"}
			elsif ($pid =~ /running/) {$pid = "�������"}
				else {$pid = "����������"}
		print <<HTML__;
	<table border=0 bgcolor=$TableBorderColor cellpadding="$RightTablesCellPadding" cellspacing=1 width=420 valign="center">
		<tr bgcolor=$TableColorStrip><td colspan=2 align=center><b>���������� � ��������� IServerd:</b></td></tr>
		<tr bgcolor=$TableColor><td>��������� �������:</td><td><b>$pid</b></td></tr>
		<tr bgcolor=$TableColor><td>����� �������������:</td><td><b>$TotalCount</b></td></tr>
		<tr bgcolor=$TableColor><td>������������ �������������:</td><td><b>$OnlineCount</b></td></tr>
		<tr bgcolor=$TableColor><td>�������� �� �����������:</td><td><b>$RRequestCount</b></td></tr>
		<tr bgcolor=$TableColor><td>���������� ���������:</td><td><b>$MessagesCount</b></td></tr>
		<tr bgcolor=$TableColor><td>�������������� �������������:</td><td><b>$UlockCount</b></td></tr>
	</table>
HTML__
	}
	else
	{
		print qq(<b>����������� ��������� ���������� ���������.</b> <br>��� ���������, ��������� � <a href="isdwm.cgi?action=setup">��������� �������</a>);
	}
	&PrintFooter; exit;

sub PrintHeader ()
{
	print "\n";
	$Title = shift || "ISD Web Manager";
	print <<HTML__;
<html>
<head>
<style type="text/css">
BODY, TD {
$BodyTDStyle
}
 A:link {
$ALink
}
 A:visited { 
$ALink
}
 A:hover {
$AHover
}
INPUT {
	background-color : $InputBGColor;
	color: Black;
	border: solid Black 1;
	font : 14px Arial;
	height : 20px;
}
TEXTAREA {
	background-color : $InputBGColor;
	border: solid Black 1;
}

TABLE {
	background-color : $MainTableColor;
	font : smaller Verdana;
}

</style>

<title>$Title</title>
<meta http-equiv="Content-Type" content="text/html; $WebCodepage">
</head>

<body bgcolor="$BGColor" text="$TextColor">
<table width="$MainTableSize" border="0" cellspacing="1" cellpadding="$MainTableCellPadding">
  <tr valign=top>
    <td width="$LeftTableSize" align="center"><img src="$URLToImages/isdwmlogo.gif" width="150" height="50"><br>
HTML__
}

sub PrintFooter ()
{
	print qq(
</td></tr><tr valign=top><td>&nbsp;</td><td align=center>$Copyright<br><br></td></tr>
</table>
</body>
</html>);
}
sub PrintMenu ()
{
	$Item = shift || 10;
	print <<HTML__;
      <table width="100%" border="0" cellspacing="0" cellpadding="$MenuCellPadding">
HTML__
	foreach (sort { $a<=>$b} keys %MenuItems)
	{
		if ($Item == $_) {
			print <<HTML__;
        <tr> 
          <td bgcolor="$MenuSelectedColor" onmouseover="this.style.backgroundColor='$MenuOverColor';"
	      onmouseout="this.style.backgroundColor='$MenuSelectedColor';"><a href="isdwm.cgi?action=$MenuItems{$_}->[1]">$MenuItems{$_}->[0]</a></td>
        </tr>
HTML__
		} else {
			print <<HTML__;
        <tr> 
          <td onmouseover="this.style.backgroundColor='$MenuOverColor';"
	      onmouseout="this.style.backgroundColor='$MainTableColor';"><a href="isdwm.cgi?action=$MenuItems{$_}->[1]">$MenuItems{$_}->[0]</a></td>
        </tr>
HTML__
		}
	}
	print qq(      </table>
    </td>
    <td align="center"><br>);
}

sub PrintRegForm ()
{
	@_=&koi2win(@_);
	my ($uin, $pass, $nick, $frst, $last, $email, $sex, $regreq) = @_;
	my ($a, $b, $c);
	if ($sex == 1)
	{
		$a="selected";
	}
	elsif ($sex == 2)
	{
		$b="selected";
	}
	else
	{
		$c="selected";
	}
	print qq|	<FORM ACTION="isdwm.cgi" METHOD="GET">
	<INPUT TYPE="hidden" NAME="do" VALUE="submit">
	<table border=0 bgcolor=$TableBorderColor cellpadding="$RightTablesCellPadding" cellspacing=1 width=100%>
		<tr bgcolor=$TableColorStrip><td colspan=2 align=center><b>���������� ������ ������������</b></td>
		<tr bgcolor=$TableColor><td>UIN:</td><td><INPUT TYPE="HIDDEN" NAME="action" VALUE="adduser"><INPUT TYPE="text" NAME="uin" SIZE="8" VALUE="$uin" $InputParam> </td></tr>
		<tr bgcolor=$TableColor><td>������:</td><td><INPUT TYPE="password" NAME="password" SIZE="8" MAXLENGTH="8" VALUE="$pass" $InputParam> </td></tr>
		<tr bgcolor=$TableColor><td>���:</td><td><INPUT TYPE="text" NAME="nick" VALUE="$nick" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td>���:</td><td><INPUT TYPE="text" NAME="frst" VALUE="$frst" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td>�������:</td><td><INPUT TYPE="text" NAME="last" VALUE="$last" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td>e-mail:</td><td><INPUT TYPE="text" NAME="email" VALUE="$email" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td>���:</td><td><SELECT NAME="sex" $InputParam><OPTION VALUE="2" $b>�������<OPTION value="1" $a>�������<OPTION value="0" $c>����������</SELECT></td></tr>|;
	if ($mail_reg eq "yes") {print qq|		<tr bgcolor=$TableColor><td>��������� � �����������:</td><td><INPUT TYPE="checkbox" NAME="sendmail" $InputParam> ��, ������� �������� ��������� � �����������</td></tr>|;}
		print qq|		<tr bgcolor=$TableColor><td>����� ������:</td><td><INPUT TYPE="checkbox" NAME="cpass" CHECKED $InputParam> ������������ ������ ������� ������ ��� ������ ����� � �������</td></tr>
		<tr bgcolor=$TableColor><td>������ �����������:</td><td><INPUT TYPE="checkbox" NAME="auth" $InputParam> ��������� ����������� ������������� ��� ���������� ��� � �������-����</td></tr>|;
	if (defined $regreq)
		{
			print qq|		<tr bgcolor=$TableColor><td>������� �� �������� �� �����������:</td><td><INPUT TYPE="checkbox" NAME="regreq" VALUE="true" checked $InputParam> �������</td></tr>
			<INPUT TYPE="hidden" NAME="ruin" VALUE="$in{uin}">|;
		}
	print <<HTML__;
		<tr bgcolor=$TableColor><td>�����������:</td><td><INPUT TYPE="submit" NAME="submit" VALUE="����������������" $InputParam> <INPUT TYPE="RESET" NAME="Reset" VALUE="�� ���������" $InputParam></td></tr>
	</table>
	</FORM>

HTML__
}

sub PrintEntryExit()
{
	print "\n";
	$title = shift;
print <<HTML__;
<html>
<head>
<style type="text/css">
INPUT {
	background-color : $InputBGColor;
	border: solid  Black 1;
	font : 14px Arial;
	height : 20px;
}
BODY, TD {
$BodyTDStyle
}
</style>
<title>$title</title>
<meta http-equiv="Content-Type" content="text/html; $WebCodepage">
</head>
<body bgcolor="$BGColor" text="$TextColor">
<div align="center">
        <img src="$URLToImages/isdwmlogo.gif" width="150" height="50">
HTML__
}
# KOI8-R => WIN1251
sub koi2win ()
{
	if ($koi2win ne "yes") { return @_ };
	my (@ret, $ret);
	foreach $ToTranslate (@_)
	{
		$ToTranslate =~ tr/\200-\377/\200\201\202\203\204\205\206\207\210\211\212\213\214\215\216\217\220\221\222\223\224\225\226\227\230\231\240\233\260\235\267\237\232\241\242\270\244\245\246\247\263\251\252\253\254\255\256\257\234\261\262\250\264\265\266\236\243\271\272\273\274\275\276\251\376\340\341\366\344\345\364\343\365\350\351\352\353\354\355\356\357\377\360\361\362\363\346\342\374\373\347\370\375\371\367\372\336\300\301\326\304\305\324\303\325\310\311\312\313\314\315\316\317\337\320\321\322\323\306\302\334\333\307\330\335\331\327\332/;
		push (@ret, $ToTranslate);
	}
	return (wantarray ? @ret : "@ret")
};

# WIN1251 => KOI8-R
sub win2koi ()
{
	if ($win2koi ne "yes") { return @_ };
	my (@ret, $ret);
		foreach $ToTranslate (@_)
	{
		$ToTranslate =~ tr/\200-\377/\200\201\202\203\204\205\206\207\210\211\212\213\214\215\216\217\220\221\222\223\224\225\226\227\230\231\240\233\260\235\267\237\232\241\242\270\244\245\246\247\263\251\252\253\254\255\256\257\234\261\262\250\264\265\266\236\243\271\272\273\274\275\276\251\341\342\367\347\344\345\366\372\351\352\353\354\355\356\357\360\362\363\364\365\346\350\343\376\373\375\377\371\370\374\340\361\301\302\327\307\304\305\326\332\311\312\313\314\315\316\317\320\322\323\324\325\306\310\303\336\333\335\337\331\330\334\300\321/;
		push (@ret, $ToTranslate);
	}
	return (wantarray ? @ret : "@ret")
}

# ������� ��������� 2 �������� - ����� (UIN) � ������,
# ��������� �� �� ������� ����������������� ����������.

# SECURITY
sub CheckAccess ()
{
	my ($uin, $pass) = @_;
	$hash_ref = $dbh->selectall_hashref("SELECT uin,pass FROM users_info WHERE uin IN ($Admins)",uin) || die "Cannot execute query: $!";
	if ((defined $hash_ref->{$uin}->{pass}) && ($hash_ref->{$uin}->{pass} eq $pass))
	{
		return "succ";
	}
	return undef;
};
# ������������� ������ ���������� http-���������.
sub SetCookies ()
{
	if ($in{remember} eq "on") {$exp = q| expires="|.gmtime(time+10000000).q|";|;}
	print "Set-Cookie: ICQADMIN=$in{'adminuin'}#1234567890#$in{'adminpass'};$exp\n\n"
}
# ������� ������
sub ClearCookies ()
{
	print qq(Set-Cookie: ICQADMIN=;\n\n);
}

sub GetFreeUIN ()
{
	if ($FirstFreeUIN eq "yes")
	{
		$sth=$dbh->prepare("SELECT uin FROM users_info_ext WHERE (uin<=$MaxAddUIN AND uin>=$MinAddUIN) ORDER BY uin");
		$sth->execute;
		$prevUIN=0;
		$FreeUIN=0;
		while ($hash_ref = $sth->fetchrow_hashref and $FreeUIN == 0)
		{
			if ($prevUIN!=0) 
				{
					if ($hash_ref->{uin}-$prevUIN > 1 ) {$FreeUIN=++$prevUIN;}
				};
			$prevUIN=$hash_ref->{uin};
		}
	return $FreeUIN;
	}
        $sth=$dbh->prepare("SELECT MAX (uin) from users_info_ext WHERE (uin<$MaxAddUIN and uin>$MinAddUIN)");
	$sth->execute;
	my $MaxUIN = $sth->fetchrow_array;
	return ++$MaxUIN;
}

# SECURITY
sub ParseCook ()
{
	my ($par, $val, $uin, $pass);
	# ��������� ��� ������, ������������� ���� ������:
	foreach ($ENV{'HTTP_COOKIE'}) 
	{
		($par, $val) = split(/=/,$_);
		if (($par =~ /ICQADMIN/) && (($uin,$pass) = split(/#1234567890#/,$val)) && (&CheckAccess($uin,$pass) eq "succ"))
		{
			# ������� �������� ���� �����.
			# � ��� ���� ������.
			return "succ";
		}
	}
}

sub WriteUser()
{
        @_ = &koi2win(@_);
	my ($uin, $nick, $fname, $lname,$pass, $email, $status) = @_;
	print qq(<tr bgcolor = $TableColor><td align=center><a href="isdwm.cgi?action=edituin&uin=$uin">$uin</a></td><td>$nick</td><td>$fname</td><td>$lname</td><td width=1><font color="$TableColor"><nobr>$pass</nobr></font></td><td width="1">$email</td><td align=center>);
	if (defined $status)
	{
		defined $OnlineStatus{$status} ? &PrintOnlineStatus ($status) : &PrintOnlineStatus (257);
	}
	else
	{
		&PrintOnlineStatus (258);
	}
	print "</td></tr>\n";
}

sub PrintOnlineStatus()
{
	my $status = shift;
	print qq(<img src="$URLToImages/$OnlineStatus{$status}->[0]" border=0 width="16" height="16" alt="$OnlineStatus{$status}->[1]">);
}

sub GetIP()
{
	use Socket;
	return inet_ntoa(pack("L", shift));
}
sub Setup()
{
	$CallStatus = shift;
	&PrintHeader ("��������� ������� ISD Web Manager");
	if ($CallStatus == 1) {$AddOn = "<tr bgcolor=$TableColor><td colspan=2><b>������: �������� ��� ��������� ���� ������������: $0/isdwm.conf.<BR>������� ������ ������.</b></td>"}
	   elsif ($CallStatus == 2) {$AddOn = "<tr bgcolor=$TableColor><td colspan=2><b>������: ���� ����� �� ������ ��� ���������: $ThemeFile</b></td>"}
   	    elsif ($CallStatus == 3) {$AddOn = "<tr bgcolor=$TableColor><td colspan=2><b>������: ���������������� ���� ������ ������.<BR>��������� ����� ���������.</b></td>"}
	&PrintMenu(100);
	($default_lock_text,$default_pass,$default_nick,$default_frst,$default_last) = &koi2win($default_lock_text,$default_pass,$default_nick,$default_frst,$default_last);
	print qq|
	<FORM ACTION="isdwm.cgi" METHOD="POST">
	<INPUT TYPE="hidden" NAME="action" VALUE="savesettings">
	<table border=0 bgcolor=$TableBorderColor cellpadding="$RightTablesCellPadding" cellspacing=1 width=100%>
		<tr bgcolor=$TableColorStrip><td colspan=2><B>��������� ������� IServerd Web Manager</B></td>
		$AddOn
		<tr bgcolor=$TableColor><td>��� ���� ������</td><td><INPUT TYPE="TEXT" NAME="db_name" VALUE="$db_name" $InputParam> </td></tr>
		<tr bgcolor=$TableColor><td>��� ������������ ���� ������</td><td><INPUT TYPE="TEXT" NAME="db_user" VALUE="$db_user" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td>������ �� ��</td><td><INPUT TYPE="TEXT" NAME="db_pass" VALUE="$db_pass" $InputParam> </td></tr>
		<tr bgcolor=$TableColor><td>������, ����������� �������</td><td><INPUT TYPE="TEXT" NAME="ISD_control" VALUE="$ISD_control" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td>������������ ������� ������� e-mail - ���������</td><td><INPUT TYPE="CHECKBOX" NAME="mail_reg" VALUE="yes"|; if ($mail_reg eq "yes") {print " checked";} print qq|></td></tr>
		<tr bgcolor=$TableColor><td>����������� ��������� ���������� �� ������� �������� (������� �������, ���� � ��� ����� ����� ����������� ������� ��������)</td><td><INPUT TYPE="CHECKBOX" NAME="ShowISDStatus" VALUE="yes"|; if ($ShowISDStatus eq "yes") {print " checked";} print qq|></td></tr>
		<tr bgcolor=$TableColor><td>�������� �������������� �� KOI8-R � Windows-1251 (�������� ���, ���� ���� �������� � ������������ ���������� �� ���� ������ IServerd)</td><td><INPUT TYPE="CHECKBOX" NAME="koi2win" VALUE="yes"|; if ($koi2win eq "yes") {print " checked";} print qq|></td></tr>
		<tr bgcolor=$TableColor><td>�������� �������������� �� Windows-1251 � KOI8-R (���� ���� �������� � ���������� ��� �������� ��������� � ���� ������ IServerd. ��� �������, ������������ ������ � �������������� �� KOI8-R � Windows-1251)</td><td><INPUT TYPE="CHECKBOX" NAME="win2koi" VALUE="yes"|; if ($win2koi eq "yes") {print " checked";} print qq|></td></tr>
		<tr bgcolor=$TableColor><td>������ �� ��������� ��� ����� �������������</td><td><INPUT TYPE="TEXT" NAME="default_pass" VALUE="$default_pass" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td>������� �� ��������� ��� ����� �������������</td><td><INPUT TYPE="TEXT" NAME="default_nick" VALUE="$default_nick" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td>��� �� ��������� ��� ����� �������������</td><td><INPUT TYPE="TEXT" NAME="default_frst" VALUE="$default_frst" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td>������� �� ��������� ��� ����� �������������</td><td><INPUT TYPE="TEXT" NAME="default_last" VALUE="$default_last" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td colspan=2>����� ���������� ������������ �� ���������:<br><INPUT TYPE="TEXT" NAME="default_lock_text" SIZE="85" VALUE="$default_lock_text" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td>��������, �� �������� ���������� ��������� UIN</td><td><INPUT TYPE="TEXT" NAME="MinAddUIN" VALUE="$MinAddUIN" SIZE="5" $InputParam> - <INPUT TYPE="TEXT" NAME="MaxAddUIN" VALUE="$MaxAddUIN" SIZE="5" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td>�������� ������ ��������� UIN �� ���������� ���������</td><td><INPUT TYPE="CHECKBOX" NAME="FirstFreeUIN" VALUE="yes"|; if ($FirstFreeUIN eq "yes") {print " checked";} print qq|></td></tr>
		<tr bgcolor=$TableColor><td>URL � ��������� (������ ��� ������������ �����)</td><td><INPUT TYPE="TEXT" NAME="URLToImages" VALUE="$URLToImages" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td>���� �������������� ����</td><td><INPUT TYPE="TEXT" NAME="ThemeFile" VALUE="$ThemeFile" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td>������ UIN'��, ������� ������ � ������� (����� �������)</td><td><INPUT TYPE="TEXT" NAME="Admins" VALUE="$Admins" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td>������������ ���-�� ��������� �� �������� �� ��������</td><td><INPUT TYPE="TEXT" NAME="MaxPageItems" VALUE="$MaxPageItems" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td>�������� ����� (��������, ����������� ������� � ���� � ����)</td><td><INPUT TYPE="TEXT" NAME="sendmailprog" VALUE="$sendmailprog" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td>E-mail, �� �������� ������������� ����� ��������� ��������� � �����������</td><td><INPUT TYPE="TEXT" NAME="AdminMail" VALUE="$AdminMail" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td colspan=2>���� ������ � ���������� � �����������:<br><INPUT TYPE="TEXT" NAME="MailRegSubject" SIZE="85" VALUE="$MailRegSubject" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td colspan=2>����� ������:<BR><TEXTAREA NAME="MailTextSubject" ROWS=5 COLS=65 $InputParam>$MailTextSubject</TEXTAREA><br>(%uin% - UIN, �������� ������������; %pass% - ������ �� UIN'�; %adminmail% - ��������������� �������� ����� �������������� �������)</td></tr>
		<tr bgcolor=$TableColor><td>������ ������� ������� (� �������� ��� � %)</td><td><INPUT TYPE="TEXT" NAME="MainTableSize" VALUE="$MainTableSize" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td>������ ������� ���� (� �������� ��� � %, �� ����� - 150)</td><td><INPUT TYPE="TEXT" NAME="LeftTableSize" VALUE="$LeftTableSize" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td>���� ��������� ����</td><td><INPUT TYPE="TEXT" NAME="InputBGColor" VALUE="$InputBGColor" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td>���� �������� �����, ��� ��������� �� ���� ������</td><td><INPUT TYPE="TEXT" NAME="InputMouseOver" VALUE="$InputMouseOver" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td>���� ������� �������������� �������</td><td><INPUT TYPE="TEXT" NAME="TableBorderColor" VALUE="$TableBorderColor" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td>���� ����� �������������� �������</td><td><INPUT TYPE="TEXT" NAME="TableColor" VALUE="$TableColor" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td>�������� CellPadding ��� �������������� ������</td><td><INPUT TYPE="TEXT" NAME="RightTablesCellPadding" VALUE="$RightTablesCellPadding" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td>���� �������� �������</td><td><INPUT TYPE="TEXT" NAME="MainTableColor" VALUE="$MainTableColor" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td>�������� CellPadding ��� �������� �������</td><td><INPUT TYPE="TEXT" NAME="MainTableCellPadding" VALUE="$MainTableCellPadding" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td>���� ��������� �������������� �������</td><td><INPUT TYPE="TEXT" NAME="TableColorStrip" VALUE="$TableColorStrip" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td>���� ������</td><td><INPUT TYPE="TEXT" NAME="TextColor" VALUE="$TextColor" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td>���� ����</td><td><INPUT TYPE="TEXT" NAME="BGColor" VALUE="$BGColor" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td>���� ���������� ������ ����</td><td><INPUT TYPE="TEXT" NAME="MenuSelectedColor" VALUE="$MenuSelectedColor" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td>���� �������� ���� ��� ��������� �� ���� ������� �����</td><td><INPUT TYPE="TEXT" NAME="MenuOverColor" VALUE="$MenuOverColor" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td>���������, � ������� �������� ������ ������� ����� ������������ (��� META)</td><td><INPUT TYPE="TEXT" NAME="WebCodepage" VALUE="$WebCodepage" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td>�������� CellPaddind ��� ������� ���� </td><td><INPUT TYPE="TEXT" NAME="MenuCellPadding" VALUE="$MenuCellPadding" $InputParam></td></tr>
		<tr bgcolor=$TableColor><td colspan=2>����� ���� �������� � ���� ����� �������<BR><TEXTAREA NAME="BodyTDStyle" ROWS=5 COLS=65 $InputParam>$BodyTDStyle</TEXTAREA></td></tr>
		<tr bgcolor=$TableColor><td colspan=2>����� ������ (A:LINK � A:VISITED)<BR><TEXTAREA NAME="ALink" ROWS=5 COLS=65 $InputParam>$ALink</TEXTAREA></td></tr>
		<tr bgcolor=$TableColor><td colspan=2>����� ��������� ������<BR><TEXTAREA NAME="AHover" ROWS=5 COLS=65 $InputParam>$AHover</TEXTAREA></td></tr>
		<tr bgcolor=$TableColor><td colspan=2 align="right"><INPUT TYPE="SUBMIT" VALUE="��������� ���������" $InputParam></td></tr>
		</TABLE>
		</FORM>|;
	&PrintFooter; exit;
};