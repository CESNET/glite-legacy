/opt/glite/bin/glite-lb-bkserverd stop
userdel glite 
groupdel glite
rm -R -f /opt/glite/var
rm -f /etc/profile.d/glite-lb*
/usr/bin/mysql < /tmp/drop_table
rm -f /etc/my.cnf
rm -R -f /var/lib/mysql
for a in `ps -efwww | grep mysql | awk 'BEGIN { FS=" "}; $0 ~ // { print $2 }'`; do kill $a; done

