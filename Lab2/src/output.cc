#include "server.h"
#include "output.h"
using namespace std;

void output_setting()
{
    char proxy_close[] = "close";
    cout << "----Setting----\n"
            "ip: " << ip_str << "\n"
            "port: " << port << "\n"
            "proxy: " << ((proxy==nullptr)?proxy_close:proxy) << "\n"
            "thread number: " << thread_num << "\n";
}