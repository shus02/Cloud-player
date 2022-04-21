#include <iostream>
#include <thread>
#include<sys/types.h>
#include<sys/socket.h>

using namespace std;

const unsigned int THREAD_NUM = thread::hardware_concurrency();
const unsigned short SERVER_PORT = 80;
const unsigned int MAX_LINK = 10000;
//linux限制tcp最大数量修改："/etc/security/limits.conf", 不应超过最大端口数量65535
//1 *    soft    nofile 10000
//2 *    hard    nofile 10000
//-----------------------------------------------


int main()
{
    cout << "线程数:" << THREAD_NUM << endl;
    
}