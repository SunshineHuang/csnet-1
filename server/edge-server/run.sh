ulimit -c unlimited
nohup ./edge-server ./server.conf &
echo $! > my.pid
