#!/bin/sh

name="`basename $0`"
PSQL="/usr/bin/psql"
users_db="users_db"
db_check="/usr/bin/db_check"
users_info1_file=`/usr/bin/mktemp /tmp/tbl1XXXXXX`;
users_info2_file=`/usr/bin/mktemp /tmp/tbl2XXXXXX`;
users_mess_file=`/usr/bin/mktemp /tmp/tbl3XXXXXX`;

cat <<EOF
  -----------------------------------------------------------
  WARNING: This utitlity will convert your old style users_db 
  database to new format. Please make sure that you have backup 
  copy of this database !!! Some tables will be deleted !!!

  Use only if you upgraged from 0.x.x or from 1.4.12 to 1.5+.x
  Make sure that IServerd is stopped!.
 
  You can backup your old-style IServerd database with 
  "db_manage dump users_db dump.txt" command.
  -----------------------------------------------------------
  
EOF
echo -n "  Are you sure you want to process? (y/n) "
read confirm
if [ "$confirm" = "y" ] || [ "$confirm" = "Y" ]
	then
		echo "  ******> Deleting table Online_Users";
		$PSQL -q -t -c "DROP TABLE Online_Users" $users_db 2>/dev/null;

		echo "  ******> Deleting table Online_Contacts";
		$PSQL -q -t -c "DROP TABLE Online_Contacts" $users_db 2>/dev/null;

		echo "  ******> Deleting table Fragment_Storage";
		$PSQL -q -t -c "DROP TABLE Fragment_Storage" $users_db 2>/dev/null;

		echo "  ******> Deleting table Users_Lock";
		$PSQL -q -t -c "DROP TABLE Users_Lock" $users_db 2>/dev/null;

		echo "  ******> Deleting table Register_Requests";
		$PSQL -q -t -c "DROP TABLE Register_Requests" $users_db 2>/dev/null;

		echo "  ******> Deleting view users";
		$PSQL -q -t -c "DROP VIEW Users" $users_db 2>/dev/null;

		echo "  ******> Copying data from Users_Messages to file";
		$PSQL -q -t -c "COPY Users_Messages TO '$users_mess_file'" $users_db 2>/dev/null;

		echo "  ******> Copying data from Users_Info to file";
		$PSQL -q -t -c "COPY Users_Info TO '$users_info1_file'" $users_db 2>/dev/null;

		echo "  ******> Copying data from Users_Info_Ext to file";
		$PSQL -q -t -c "COPY Users_Info_Ext TO '$users_info2_file'" $users_db 2>/dev/null;

		echo "  ******> Deleting table Users_Info";
		$PSQL -q -t -c "DROP TABLE Users_Info" $users_db 2>/dev/null;

		echo "  ******> Deleting table Users_Messages";
		$PSQL -q -t -c "DROP TABLE Users_Messages" $users_db 2>/dev/null;

		echo "  ******> Deleting table Users_Info_Ext";
		$PSQL -q -t -c "DROP TABLE Users_Info_Ext" $users_db 2>/dev/null;

	        echo ""
		echo "  ----=====Creating new tables====-----"
		$db_check;
		
		echo "  ******> Copying data from file to Users_Messages";
		$PSQL -q -t -c "COPY Users_Messages FROM '$users_mess_file'" $users_db 2>/dev/null;

		echo "  ******> Copying data from file to Users_Info";
		$PSQL -q -t -c "COPY Users_Info FROM '$users_info1_file'" $users_db 2>/dev/null;

		echo "  ******> Copying data from file to Users_Info_Ext";
		$PSQL -q -t -c "COPY Users_Info_Ext FROM '$users_info2_file'" $users_db 2>/dev/null;
		
		echo "Convertation finished.... Now you can run IServerd 1.5+.0"
		rm -rf $users_info1_file;
		rm -rf $users_info2_file;
		rm -rf $users_mess_file;
	else 
		echo "  ******> User canceling"
fi
