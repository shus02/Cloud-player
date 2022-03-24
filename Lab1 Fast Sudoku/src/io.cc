#include <iostream>
#include <string>
#include <list>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <functional>
#include "sudoku.h"

using namespace std;


queue<Sudoku> buffer;
priority_queue<Sudoku,vector<Sudoku>> output;

void* outputResult(void * maxid) //STL queue 多线程读取是否安全？
{
    long long id = 0;
    while(id <= *((unsigned long long *)maxid))
    {
        if((!output.empty()) && output.top().id == id)//如果不为空，判断顶层是否为下一个输出
        {
            cout << output.top().str << "\n";
            //临界区
            pthread_mutex_lock(&outMutex);
            output.pop();
            pthread_mutex_unlock(&outMutex);
            id++;
        }
    }
    
}

void* loadSodoku(void *arg)
{
    string file;
    pthread_t outputThread;
    unsigned long long id = 0;                              //一次最多处理longlong

    pthread_create(&outputThread, NULL,outputResult, &id);  //输出线程

    while (getline(cin, file))                              //每个文件名一行
    {
        FILE *fp = fopen(file.c_str(), "r");                //打开文件
        
        while (true)
        {
            Sudoku tmp;
            tmp.id = id++;
            if (fgets(tmp.str, sizeof(tmp.str), fp) != NULL)//按行读取文件
                break;
            //临界区
            sem_wait(&emptySlots); 
            pthread_mutex_lock(&mutex);
            buffer.push(tmp);                               //写入buffer
            pthread_mutex_unlock(&mutex);
            sem_post(&fullSlots);
        }
    }
}