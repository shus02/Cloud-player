#include <iostream>
#include <string>
#include <unistd.h>
using namespace std;
#define FILES_NUM 100

void* f(void*){sleep(1000);return nullptr;}

int main()//多线程会导致输入无法结束（无法获取到eof），即使主线程结束，暂未找到解决方案
{
    string files[FILES_NUM];
    for(int i=0;getline(cin,files[i]);i++)
        cout << files[i] << endl;
    pthread_t p;
    pthread_create(&p,NULL,f,NULL);
    pthread_exit(nullptr);
}