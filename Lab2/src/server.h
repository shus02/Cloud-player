#ifndef SERVER_H
#define SERVER_H

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <cmath>
#include <iostream>
#include <sys/epoll.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <unordered_map>

#define DEBUG true
#define EPOLL_MAX 2048
#define READ_BUF_LEN 256
#define STATIC_DIR "./static"
extern unsigned short port;
extern unsigned short thread_num;
extern struct in_addr ip;
extern char *proxy,ip_str[16] ;

bool server_run();
void server_exit(int status);

#endif // SERVER_H