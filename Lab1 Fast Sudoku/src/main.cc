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
pthread_mutex_t mutex;
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
    init_neighbors();
    sem_init(&fullSlots, 0, 0); 
    sem_init(&emptySlots, 0, BUF_SIZE); 
    pthread_mutex_init(&mutex,NULL);

    char puzzle[128];
    int64_t start = now();

    pthread_t producer,consumers[THREAD_NUM];
    bool inputDone = false;
    //创建线程读取数据到buffer
    pthread_create(&producer, NULL, loadSodoku, &inputDone);

    //创建算法线程，线程将会等待buffer输入
    for(int i = 0;i < THREAD_NUM;i++)
        pthread_create(&consumers[i], NULL, solve_sudoku_dancing_links, &inputDone);
    // while (fgets(puzzle, sizeof puzzle, fp) != NULL)
    // {
    //     if (strlen(puzzle) >= N)
    //     {
    //         ++total;
    //         input(puzzle);
    //         // init_cache();
    //         if (solve_sudoku_dancing_links(0))
    //         {
    //             ++total_solved;
    //             if (!solved())
    //                 assert(0);
    //         }
    //         else
    //         {
    //             printf("No: %s", puzzle);
    //         }
    //     }
    // }
    int64_t end = now();
    double sec = (end - start) / 1000000.0;
    printf("%f sec %f ms each %d\n", sec, 1000 * sec / total, total_solved);

    return 0;
}