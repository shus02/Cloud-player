#include <iostream>
#include <string>
#include <list>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include "sudoku.h"

using namespace std;


list<Sudoku> buffer;
void* loadSodoku(void *arg)
{
    string file;

    while (getline(cin, file))
    {
        long long id = 0;
        FILE *fp = fopen(file.c_str(), "r");
        while (true)
        {
            Sudoku tmp;
            tmp.id = id++;
            if (fgets(tmp.str, sizeof(tmp.str), fp) != NULL)
                break;
            //临界区
            sem_wait(&emptySlots); 
            pthread_mutex_lock(&mutex);
            buffer.push_back(tmp);
            pthread_mutex_unlock(&mutex);
            sem_post(&fullSlots);
        }
    }
}
