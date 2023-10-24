  Данный пакет содержит клон ICQ Groupware server, написанный
Александром Шутко (AVShutko@mail.khstu.ru) из Хабаровска.

  На текущий момент поддерживаются протоколы V3, V5 и V7.
Также поддерживаются WWP сообщения. 

  Для установки сервера необходимо установить пакеты postgresql и
postgresql-server.

Запуск(остановка) сервера производится таким образом (от root):
    /etc/rc.d/init.d/iserverd start
    /etc/rc.d/init.d/iserverd stop

Проконтролировать работу сервера можно запустив программу
(от root или пользователей группы iserverd)
    /usr/bin/server_status

Создать, удалить, сохранить или восстановить базу можно
используя скрипт (от postgres) db_manage.sh:
    /usr/bin/db_manage.sh create <DB_NAME>
    /usr/bin/db_manage.sh drop <DB_NAME>
    /usr/bin/db_manage.sh dump <DB_NAME> <TEMPFILE>
    /usr/bin/db_manage.sh restore <DB_NAME> <TEMPFILE>

Поскольку база поменяла свой формат, необходимо при обновлении пакета 
обновить и базу (от postgres)
    /usr/bin/convert_db.sh

Добавить или удалить пользователя (от iserverd):
   /etc/iserverd/db/icquser add <UIN>
   /etc/iserverd/db/icquser del <UIN>

Также добавлен поиск пользователя в базе (от iserverd):
   /etc/iserverd/db/icquser search <UIN>

Andy Shevchenko <andriy@asplinux.ru>
ASPLinux Developers Team.

