ulimit -c unlimited
./ssl-server ./server.conf &
echo $! > my.pid
