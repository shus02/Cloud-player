#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <fcntl.h>  
#include <unistd.h>  
#include <stdio.h>  
#include <errno.h>  
#include <strings.h>
#include <stdlib.h>
#include <string.h>
#include <iostream> 
#include <getopt.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "server.h"
#include "output.h"
using namespace std;

//default setting

unsigned short port = 80;
unsigned short thread_num = 12;
struct in_addr ip;
char ip_str[16]="127.0.0.1",*proxy = nullptr;

bool checkip(char *address);
void server_exit(int status);

int main(int argc,char *argv[])
{
    //Process parameters
    static struct option long_options[] = {
        {"ip", required_argument, NULL, 'i'},
        {"port", required_argument, NULL, 'p'},
        {"proxy", required_argument, NULL, 'P'},
        {"threads", required_argument, NULL, 't'},
        {0,0,0,0}
    };
    int opt_index = 0;
    while(true)
    {
        int c = getopt_long(argc, argv, "i:p:P:t:", long_options, &opt_index);
        if(c == -1) break;
        switch (c)
        {
        case 'i':
            strcpy(ip_str,optarg);
            if(!inet_aton(optarg, &ip))
            {
                perror("inet_aton");
                server_exit(0);
            }
            break;
        case 'p':
            //port
            port = atoi(optarg);
            if(!(port>0 && port < 65535))
            {
                cout << "port error: out of range" << port << endl;
                server_exit(0);
            }
            break;
        case 'P':
            //PROXY
            proxy = new char[strlen(optarg)];
            strcpy(proxy,optarg);
            break;
        case 't':
            //thread
            thread_num = atoi(optarg);
            if(!(0 <thread_num && thread_num < 10000))
            {
                cout << "thread num error: out of range" << endl;
                server_exit(0);
            }
            break;
        default:
            cout << "parameter error: unknown parameter" << endl;
            server_exit(0);
        }
    }
    output_setting();
    
    //tcp server thread
    return server_run();
}

void server_exit(int status)
{
    delete[] proxy;
    exit(status);
}