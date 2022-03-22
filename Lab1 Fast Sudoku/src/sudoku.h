#ifndef SUDOKU_H
#define SUDOKU_H

#define BUF_SIZE 100
#define THREAD_NUM 12

const bool DEBUG_MODE = false;
enum { ROW=9, COL=9, N = 81, NEIGHBOR = 20 };
const int NUM = 9;
struct Sudoku
{
    unsigned long long id;
    char str[81];
};


extern sem_t fullSlots; 
extern sem_t emptySlots; 
extern pthread_mutex_t mutex;

extern list<Sudoku> buffer;
extern int neighbors[N][NEIGHBOR];
extern int board[N];
extern int spaces[N];
extern int nspaces;
extern int (*chess)[COL];

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
