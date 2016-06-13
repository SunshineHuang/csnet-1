#pragma once

int set_nonblocking(int sfd);

int listen_port(int port);

/* host can be a doname or an ip address  */
int blocking_connect(const char* host, int port);

/* host can be a doname or an ip address  */
int nonblocking_connect(const char* host, int port, int timeout /* milliseconds */);

void wait_milliseconds(int milliseconds);

void modin(int epfd, int socket, unsigned int sid);
void modout(int epfd, int socket, unsigned int sid);
void modinout(int epfd, int socket, unsigned int sid);
void modadd(int epfd, int socket, unsigned int sid);
void moddel(int epfd, int socket, unsigned int sid);

