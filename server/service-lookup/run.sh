ulimit -c unlimited
nohup ./lookup-server ./server.conf &
echo $! > my.pid
