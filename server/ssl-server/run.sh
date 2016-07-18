ulimit -c unlimited
nohup ./ssl-server ./server.conf &
echo $! > my.pid
