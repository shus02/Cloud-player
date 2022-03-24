#ifndef SUDOKU_H
#define SUDOKU_H

#include <semaphore.h>
#include <list>
#include <queue>

#define BUF_SIZE 100
#define THREAD_NUM 12

const bool DEBUG_MODE = false;
enum { ROW=9, COL=9, N = 81, NEIGHBOR = 20 };
const int NUM = 9;
struct Sudoku
{
    unsigned long long id;
    int str[81];

    bool operator<(const Sudoku& a) const{return id > a.id;}//小顶堆
};


extern sem_t fullSlots; 
extern sem_t emptySlots; 
extern pthread_mutex_t mutex;
extern pthread_mutex_t outMutex;

extern std::queue<Sudoku> buffer;
extern std::priority_queue<Sudoku> output;
extern unsigned long long total; 
extern unsigned long long total_solved; 

// extern int neighbors[N][NEIGHBOR];
// extern int board[N];
// extern int spaces[N];
// extern int nspaces;
// extern int (*chess)[COL];

void* loadSodoku(void *);
void init_neighbors();
void sudoku_input(const char in[N]);
void init_cache();

bool available(int guess, int cell);

bool solve_sudoku_basic(int which_space);
bool solve_sudoku_min_arity(int which_space);
bool solve_sudoku_min_arity_cache(int which_space);
void* solve_sudoku_dancing_links(void*);
bool solved();
#endif
