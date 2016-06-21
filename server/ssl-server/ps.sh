ps aux | head -n 1
PID=`cat my.pid`
ps aux | grep -v grep | grep ./ssl-server | grep $PID
