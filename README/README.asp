  ������ ����� �������� ���� ICQ Groupware server, ����������
����������� ����� (AVShutko@mail.khstu.ru) �� ����������.

  �� ������� ������ �������������� ��������� V3, V5 � V7.
����� �������������� WWP ���������. 

  ��� ��������� ������� ���������� ���������� ������ postgresql �
postgresql-server.

������(���������) ������� ������������ ����� ������� (�� root):
    /etc/rc.d/init.d/iserverd start
    /etc/rc.d/init.d/iserverd stop

����������������� ������ ������� ����� �������� ���������
(�� root ��� ������������� ������ iserverd)
    /usr/bin/iserver_server_status

�������, �������, ��������� ��� ������������ ���� �����
��������� ������ (�� postgres) db_manage.sh:
    /usr/bin/iserver_db_manage.sh create <DB_NAME>
    /usr/bin/iserver_db_manage.sh drop <DB_NAME>
    /usr/bin/iserver_db_manage.sh dump <DB_NAME> <TEMPFILE>
    /usr/bin/iserver_db_manage.sh restore <DB_NAME> <TEMPFILE>

��������� ���� �������� ���� ������, ���������� ��� ���������� ������ 
�������� � ���� (�� postgres)
    /usr/bin/iserver_convert_db.sh

�������� ��� ������� ������������ (�� iserverd):
   /etc/iserverd/db/iserver_icquser add <UIN>
   /etc/iserverd/db/iserver_icquser del <UIN>

����� �������� ����� ������������ � ���� (�� iserverd):
   /etc/iserverd/db/iserver_icquser search <UIN>

Andy Shevchenko <andriy@asplinux.ru>
ASPLinux Developers Team.

