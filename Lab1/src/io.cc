#include <iostream>
#include <string>
#include <list>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <functional>
#include "sudoku.h"

using namespace std;


queue<Sudoku> buffer;
priority_queue<Sudoku,vector<Sudoku>> output;

void* outputResult(void * calDone) //STL queue 多线程读取是否安全？
{
    long long id = 0;
    while(!(*((bool*)calDone)))
    {
        sem_wait(&outfullSlots);
        while(output.top().id != id) //空转直到满足条件
        {
            usleep(1000);
            #if DEBUG_MODE
            cout << "output waiting..." << endl;
            cout << "当前id:" << output.top().id << '\t' << "期望id:" << id << endl;
            #endif
        }
        Sudoku tmp = output.top();
        #if DEBUG_MODE
        cout << tmp.id << ": " << endl;
        #endif
        for(int i = 0;i < N; i++)
            cout << char('0'+tmp.str[i]);
        cout << endl;
        //临界区
        pthread_mutex_lock(&outMutex);
        output.pop();
        pthread_mutex_unlock(&outMutex);
        id++;
    }
    return nullptr;
}

void* loadSodoku(void* inputDone)
{
    string file;
    unsigned long long id = 0;                              //一次最多处理longlong

    while (!cin.eof())                                      //每个文件名一行
    {
        getline(cin, file);
        FILE *fp = fopen(file.c_str(), "r");                //打开文件
        if(fp == nullptr)
        {
            cout << "open file error" << endl;
            break;
        }
        #if DEBUG_MODE
            cout << "reading file \"" << file << '"' << endl;
        #endif

        while (true)
        {
            char s[90];
            Sudoku tmp;
            if (fgets(s, sizeof s, fp) == nullptr)//按行读取文件
            {
                #if DEBUG_MODE
                cout << "读取完成" << endl;
                #endif
                fclose(fp);
                break;
            }
            tmp.id = id++;
            #if DEBUG_MODE
            cout << s << endl;
            #endif
            for (int cell = 0; cell < N; ++cell) {
                tmp.str[cell] = s[cell] - '0';
                assert(0 <= tmp.str[cell] && tmp.str[cell] <= NUM);
            }
            //临界区
            sem_wait(&emptySlots); 
            pthread_mutex_lock(&mutex);
            #if DEBUG_MODE
            cout << tmp.id << ": 进入buffer临界区(+)" << endl;
            #endif
            buffer.push(tmp);                               //写入buffer
            pthread_mutex_unlock(&mutex);
            sem_post(&fullSlots);
            #if DEBUG_MODE
            cout << tmp.id << ": 离开buffer临界区(+)" << endl;
            #endif
        }
    }
    (*((bool*)inputDone)) = true;
    return nullptr;
}