#include <iostream>
#include <string>
#include <list>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <assert.h>
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
            Sudoku tmp = output.top();
            for(int i = 0;i < N; i++)
                cout << char('0'+tmp.str[i]);
            cout << '\n';
            //临界区
            pthread_mutex_lock(&outMutex);
            output.pop();
            pthread_mutex_unlock(&outMutex);
            id++;
        }
    }
    
}

void* loadSodoku(void* inputDone)
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
            char s[90];
            Sudoku tmp;
            tmp.id = id++;
            if (fgets(s, sizeof s, fp) != NULL)//按行读取文件
                break;
            for (int cell = 0; cell < N; ++cell) {
                tmp.str[cell] = s[cell] - '0';
                assert(0 <= tmp.str[cell] && tmp.str[cell] <= NUM);
            }
            //临界区
            sem_wait(&emptySlots); 
            pthread_mutex_lock(&mutex);
            buffer.push(tmp);                               //写入buffer
            pthread_mutex_unlock(&mutex);
            sem_post(&fullSlots);
        }
    }
    (*((bool*)inputDone)) = true;
}