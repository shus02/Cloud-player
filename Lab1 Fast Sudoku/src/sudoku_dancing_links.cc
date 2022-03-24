#include <assert.h>
#include <memory.h>
#include <map>
#include <vector>
#include <algorithm>
#include "sudoku.h"
using namespace std;

struct Node;
typedef Node Column;
struct Node
{
    Node *left;
    Node *right;
    Node *up;
    Node *down;
    Column *col;
    int name;
    int size;
};

const int kMaxNodes = 1 + 81 * 4 + 9 * 9 * 9 * 4;
const int kMaxColumns = 400;
const int kRow = 100, kCol = 200, kBox = 300;

struct Dance
{
    Column *root_;
    int *inout_;
    Column *columns_[400];
    vector<Node *> stack_;
    Node nodes_[kMaxNodes];
    int cur_node_;

    Column *new_column(int n = 0)
    {
        assert(cur_node_ < kMaxNodes);
        Column *c = &nodes_[cur_node_++];
        memset(c, 0, sizeof(Column));
        c->left = c;
        c->right = c;
        c->up = c;
        c->down = c;
        c->col = c;
        c->name = n;
        return c;
    }

    void append_column(int n)
    {
        assert(columns_[n] == NULL);

        Column *c = new_column(n);
        put_left(root_, c);
        columns_[n] = c;
    }

    Node *new_row(int col)
    {
        assert(columns_[col] != NULL);
        assert(cur_node_ < kMaxNodes);

        Node *r = &nodes_[cur_node_++];

        // Node* r = new Node;
        memset(r, 0, sizeof(Node));
        r->left = r;
        r->right = r;
        r->up = r;
        r->down = r;
        r->name = col;
        r->col = columns_[col];
        put_up(r->col, r);
        return r;
    }

    int get_row_col(int row, int val)
    {
        return kRow + row * 10 + val;
    }

    int get_col_col(int col, int val)
    {
        return kCol + col * 10 + val;
    }

    int get_box_col(int box, int val)
    {
        return kBox + box * 10 + val;
    }

    Dance(int inout[81]) : inout_(inout), cur_node_(0)
    {
        stack_.reserve(100);

        root_ = new_column();
        root_->left = root_->right = root_;
        memset(columns_, 0, sizeof(columns_));

        bool rows[N][10] = {false};
        bool cols[N][10] = {false};
        bool boxes[N][10] = {false};

        for (int i = 0; i < N; ++i)
        {
            int row = i / 9;
            int col = i % 9;
            int box = row / 3 * 3 + col / 3;
            int val = inout[i];
            rows[row][val] = true;
            cols[col][val] = true;
            boxes[box][val] = true;
        }

        for (int i = 0; i < N; ++i)
        {
            if (inout[i] == 0)
            {
                append_column(i);
            }
        }

        for (int i = 0; i < 9; ++i)
        {
            for (int v = 1; v < 10; ++v)
            {
                if (!rows[i][v])
                    append_column(get_row_col(i, v));
                if (!cols[i][v])
                    append_column(get_col_col(i, v));
                if (!boxes[i][v])
                    append_column(get_box_col(i, v));
            }
        }

        for (int i = 0; i < N; ++i)
        {
            if (inout[i] == 0)
            {
                int row = i / 9;
                int col = i % 9;
                int box = row / 3 * 3 + col / 3;
                // int val = inout[i];
                for (int v = 1; v < 10; ++v)
                {
                    if (!(rows[row][v] || cols[col][v] || boxes[box][v]))
                    {
                        Node *n0 = new_row(i);
                        Node *nr = new_row(get_row_col(row, v));
                        Node *nc = new_row(get_col_col(col, v));
                        Node *nb = new_row(get_box_col(box, v));
                        put_left(n0, nr);
                        put_left(n0, nc);
                        put_left(n0, nb);
                    }
                }
            }
        }
    }

    Column *get_min_column()
    {
        Column *c = root_->right;
        int min_size = c->size;
        if (min_size > 1)
        {
            for (Column *cc = c->right; cc != root_; cc = cc->right)
            {
                if (min_size > cc->size)
                {
                    c = cc;
                    min_size = cc->size;
                    if (min_size <= 1)
                        break;
                }
            }
        }
        return c;
    }

    void cover(Column *c)
    {
        c->right->left = c->left;
        c->left->right = c->right;
        for (Node *row = c->down; row != c; row = row->down)
        {
            for (Node *j = row->right; j != row; j = j->right)
            {
                j->down->up = j->up;
                j->up->down = j->down;
                j->col->size--;
            }
        }
    }

    void uncover(Column *c)
    {
        for (Node *row = c->up; row != c; row = row->up)
        {
            for (Node *j = row->left; j != row; j = j->left)
            {
                j->col->size++;
                j->down->up = j;
                j->up->down = j;
            }
        }
        c->right->left = c;
        c->left->right = c;
    }

    bool solve()
    {
        if (root_->left == root_)
        {
            for (size_t i = 0; i < stack_.size(); ++i)
            {
                Node *n = stack_[i];
                int cell = -1;
                int val = -1;
                while (cell == -1 || val == -1)
                {
                    if (n->name < 100)
                        cell = n->name;
                    else
                        val = n->name % 10;
                    n = n->right;
                }

                // assert(cell != -1 && val != -1);
                inout_[cell] = val;
            }
            return true;
        }

        Column *const col = get_min_column();
        cover(col);
        for (Node *row = col->down; row != col; row = row->down)
        {
            stack_.push_back(row);
            for (Node *j = row->right; j != row; j = j->right)
            {
                cover(j->col);
            }
            if (solve())
            {
                return true;
            }
            stack_.pop_back();
            for (Node *j = row->left; j != row; j = j->left)
            {
                uncover(j->col);
            }
        }
        uncover(col);
        return false;
    }

    void put_left(Column *old, Column *nnew)
    {
        nnew->left = old->left;
        nnew->right = old;
        old->left->right = nnew;
        old->left = nnew;
    }

    void put_up(Column *old, Node *nnew)
    {
        nnew->up = old->up;
        nnew->down = old;
        old->up->down = nnew;
        old->up = nnew;
        old->size++;
        nnew->col = old;
    }

    bool solved()
    {
        int(*chess)[COL] = (int(*)[COL])inout_;
        for (int row = 0; row < ROW; ++row)
        {
            // check row
            int occurs[10] = {0};
            for (int col = 0; col < COL; ++col)
            {
                int val = chess[row][col];
                assert(1 <= val && val <= NUM);
                ++occurs[val];
            }

            if (std::count(occurs, occurs + 10, 1) != NUM)
                return false;
        }

        for (int col = 0; col < COL; ++col)
        {
            int occurs[10] = {0};
            for (int row = 0; row < ROW; ++row)
            {
                int val = chess[row][col];
                // assert(1 <= val && val <= NUM);
                ++occurs[val];
            }

            if (std::count(occurs, occurs + 10, 1) != NUM)
                return false;
        }

        for (int row = 0; row < ROW; row += 3)
        {
            for (int col = 0; col < COL; col += 3)
            {
                int occurs[10] = {0};
                ++occurs[chess[row][col]];
                ++occurs[chess[row][col + 1]];
                ++occurs[chess[row][col + 2]];
                ++occurs[chess[row + 1][col]];
                ++occurs[chess[row + 1][col + 1]];
                ++occurs[chess[row + 1][col + 2]];
                ++occurs[chess[row + 2][col]];
                ++occurs[chess[row + 2][col + 1]];
                ++occurs[chess[row + 2][col + 2]];

                if (std::count(occurs, occurs + 10, 1) != NUM)
                    return false;
            }
        }
        return true;
    }
};

void *solve_sudoku_dancing_links(void *inputDone)
{
    //从buffer中读取
    while (true)
    {
        Sudoku tmp;
        //临界区
        sem_wait(&fullSlots); // buffer不为空
        pthread_mutex_lock(&mutex);
        tmp = buffer.front();
        buffer.pop(); //弹出队首
        pthread_mutex_unlock(&mutex);
        sem_post(&emptySlots);

        //计算结果
        ++total;
        Dance d(tmp.str);
        if (d.solve())
            ++total_solved;

        //添加到输出队列
        pthread_mutex_lock(&outMutex);
        output.push(tmp);
        pthread_mutex_unlock(&outMutex);

        //输入完成，且buffer为空，则结束
        if ((*((bool *)inputDone)) && buffer.empty()) 
            break;
    }
}
