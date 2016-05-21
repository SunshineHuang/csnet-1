ulimit -c unlimited
./midd-server ./server.conf &
echo $! > my.pid
