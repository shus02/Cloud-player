#include <cstdlib>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <pthread.h>
#include <assert.h>
#include <semaphore.h>
#include <queue>
#include "sudoku.h"
using namespace std;

sem_t fullSlots; 
sem_t emptySlots;
sem_t outfullSlots;
pthread_mutex_t mutex;
pthread_mutex_t outMutex;
unsigned long long total = 0; 
unsigned long long total_solved = 0; 

int64_t now()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

int main(int argc, char *argv[])
{
    //init
    //init_neighbors();
    sem_init(&fullSlots, 0, 0); 
    sem_init(&emptySlots, 0, BUF_SIZE); 
    sem_init(&outfullSlots, 0, 0); 
    pthread_mutex_init(&mutex,NULL);
    pthread_mutex_init(&outMutex,NULL);
    
    #if DEBUG_MODE
    char cwd[80];
    getcwd(cwd,sizeof(cwd));
    cout << "当前工作目录：" << cwd << endl;
    #endif

    char puzzle[128];

    pthread_t producer,consumers[THREAD_NUM];
    pthread_t outputThread;

    bool inputDone = false;
    //创建线程读取数据到buffer
    pthread_create(&producer, NULL, loadSodoku, &inputDone);

    //创建算法线程，线程将会等待buffer输入
    for(int i = 0;i < THREAD_NUM;i++)
        pthread_create(&consumers[i], NULL, solve_sudoku_dancing_links, &inputDone);
    
    bool calcuDone = false;
    //输出线程
    pthread_create(&outputThread, NULL,outputResult, &calcuDone);  

    pthread_join(producer,nullptr);         //等待读取完成

    for(int i = 0;i < THREAD_NUM;i++)       //等待计算完成
        pthread_join(consumers[i],nullptr);
    calcuDone = true;                       //完成标记，告知输出线程

    pthread_join(outputThread,NULL);        //等待输出线程
    return 0;
}