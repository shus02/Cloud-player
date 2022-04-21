#include "server.h"
#include "threadpool.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpConn.h"
using namespace std;

typedef struct
{
    int ID;
    int len;
} DataInfo;

std::unique_ptr<ThreadPool> threadpool;
std::unordered_map<int, HttpConn> clients;

void close_conn(int epfd, int sfd)
{
    epoll_event ev = {0};
    if (epoll_ctl(epfd, EPOLL_CTL_DEL, sfd, &ev) < 0)
    {
        perror("epoll_ctl delete fd error");
    }
}

int sendAllChunk(int sock, char *buf, int chunkSize)
{
    int sentBytes = 0;
    int len;
    while (1)
    {
        if (chunkSize - sentBytes == 0) // This data chunk has all been sent
            break;
        len = send(sock, buf + sentBytes, chunkSize - sentBytes, 0);
        if (len < 0)
        {
            perror("TCP send");
            return -1; // Error
        }
        sentBytes = sentBytes + len;
    }
    return 0; // Success
}

int recvAllChunk(int sock, char *buf, int chunkSize)
{
    int receivedBytes = 0;
    int len;
    while (1)
    {
        if (chunkSize - receivedBytes == 0) // This data chunk has all been received
            break;
        len = recv(sock, buf + receivedBytes, chunkSize - receivedBytes, 0);
        if (len <= 0)
        {
            perror("TCP recv");
            return -1; // Error
        }
        receivedBytes = receivedBytes + len;
    }
    return 0; // Success
}

void receiveData(int sock, int bytesToReceive)
{
    int totalReceivedBytes = 0;
    int totalReceivedTimes = 0;
    char buf[BUFSIZ];
    int chunkSize;
    while (1)
    {
        if (totalReceivedBytes >= bytesToReceive)
            break;
        if (bytesToReceive - totalReceivedBytes >= BUFSIZ)
            chunkSize = BUFSIZ;
        else
            chunkSize = bytesToReceive - totalReceivedBytes;
        if (recvAllChunk(sock, buf, chunkSize) == -1)
            exit(1);

        printf("[%7d, %9d] Received: ", totalReceivedTimes, totalReceivedBytes);
        for (int i = 0; i < chunkSize; i++)
            printf("%c", buf[i]);
        printf("\n");

        totalReceivedTimes++;
        totalReceivedBytes = totalReceivedBytes + chunkSize;
    }

    printf("Received %d B in total!\n", totalReceivedBytes);
}

int init_listen(sockaddr_in my_addr, int epfd)
{
    int server_sockfd;

    if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        server_exit(1);
    }

    int on = 1;
    if ((setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) < 0)
    {
        perror("setsockopt failed");
        server_exit(1);
    }

    if (bind(server_sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0)
    {
        perror("bind");
        server_exit(1);
    }

    socklen_t sin_size = sizeof(struct sockaddr_in);
    struct sockaddr_in remote_addr;
    if (listen(server_sockfd, 5) < 0)
    {
        perror("listen error");
        server_exit(1);
    }
    cout << "Listening on " << port << endl;

    return server_sockfd;
}

void serve_client(int epfd, int cfd)
{
    // Read
    int readErrno = 0;
    if (clients[cfd].read(&readErrno) <= 0 && readErrno != EAGAIN)
    {
        close_conn(epfd, cfd);
        return;
    }
    // parse http & generate response pack
    clients[cfd].process();

    // Send response
    int writeErrno = 0;
    while (clients[cfd].write(&writeErrno) < 0)
    {
        if (clients[cfd].ToWriteBytes() == 0) //传输完成
        {
            // if keepAlive
            if (clients[cfd].IsKeepAlive())
                return;
            break;//close connection
        }
        if(writeErrno != EAGAIN) {//else loop to complete
            //send error
            printf("send to client(%s:%d) failed\n", clients[cfd].GetIP(), clients[cfd].GetPort());
            break;//close connection
        }
    }
    close_conn(epfd, cfd);
    clients[cfd].Close();
}

int SetFdNonblock(int fd)
{
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}

// core
bool server_run()
{
    struct sockaddr_in my_addr = {};
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr = ip;
    my_addr.sin_port = htons(port);
    HttpConn::userCount = 0;
    HttpConn::srcDir = STATIC_DIR;
    HttpConn::isET = true; //*****
    threadpool = std::move(unique_ptr<ThreadPool>(new ThreadPool(thread_num)));
    struct epoll_event all_events[EPOLL_MAX]; //用event数组接收监听事件，为wait的传出做准备

    if (DEBUG)
        cout << "my_addr.addr=" << my_addr.sin_addr.s_addr
             << " port=" << my_addr.sin_port << endl;

    // 创建一个epoll监听树根
    int epfd = epoll_create(EPOLL_MAX);
    if (epfd == -1)
    {
        perror("epoll_create error");
        server_exit(1);
    }

    int lfd = init_listen(my_addr, epfd);

    // lfd 添加到 epoll 上
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = lfd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev) < 0)
    {
        perror("epoll_ctl add lfd error");
        server_exit(1);
    }
    SetFdNonblock(lfd);

    int inComeConnNum = 0;
    while (true)
    {
        int count = epoll_wait(epfd, all_events, EPOLL_MAX, -1);
        if (count < 0)
        {
            perror("epoll_wait error");
            server_exit(1);
        }
        for (int i = 0; i < count; i++)
        {
            uint32_t events = all_events[i].events;
            // IP地址缓存
            char host_buf[NI_MAXHOST];
            // PORT缓存
            char port_buf[NI_MAXSERV];

            int __result;
            // 判断epoll是否发生错误
            if (events & EPOLLERR || events & EPOLLHUP || (!events & EPOLLIN))
            {
                printf("Epoll has error\n");
                close(all_events[i].data.fd);
                continue;
            }
            else if (lfd == all_events[i].data.fd) // accept
            {
                struct sockaddr in_addr = {0};
                socklen_t in_addr_len = sizeof(in_addr);
                int accp_fd = accept(lfd, &in_addr, &in_addr_len);
                if (-1 == accp_fd)
                {
                    perror("Accept");
                    continue;
                }
                __result = getnameinfo(&in_addr, sizeof(in_addr),
                                       host_buf, sizeof(host_buf) / sizeof(host_buf[0]),
                                       port_buf, sizeof(port_buf) / sizeof(port_buf[0]),
                                       NI_NUMERICHOST | NI_NUMERICSERV);

                if (!__result)
                {
                    inComeConnNum++;
                    // add http clients
                    clients[accp_fd].init(accp_fd, (struct sockaddr_in &)(in_addr));
                    printf("[%d connections accepted] from client %s:%s\n", inComeConnNum, host_buf, port_buf);
                }
                else
                {
                    perror("Client address");
                    continue;
                }

                ev.data.fd = accp_fd;
                ev.events = EPOLLIN | EPOLLET;
                // 为新accept的 file describe 设置epoll事件
                __result = epoll_ctl(epfd, EPOLL_CTL_ADD, accp_fd, &ev);

                if (-1 == __result)
                {
                    perror("epoll_ctl");
                    return 0;
                }
                SetFdNonblock(accp_fd);
            }
            else // use threadpool
            {
                threadpool->AddTask(std::bind(serve_client, epfd, int(all_events[i].data.fd)));
            }
        }
    }
    close(lfd); // close listen
}

// abandoned
void do_accept(int lfd, int epfd)
{
    struct sockaddr_in clt_addr;
    socklen_t clt_addr_len = sizeof(clt_addr);

    int cfd = accept(lfd, (struct sockaddr *)&clt_addr, &clt_addr_len);
    if (cfd == -1)
    {
        perror("accept error");
        exit(1);
    }
    char client_ip[64] = {0};
    printf("New Client IP: %s, Port: %d, cfd = %d\n",
           inet_ntop(AF_INET, &clt_addr.sin_addr.s_addr, client_ip, sizeof(client_ip)),
           ntohs(clt_addr.sin_port), cfd);

    // 设置 cfd 非阻塞
    int flag = fcntl(cfd, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(cfd, F_SETFL, flag);

    // 将新节点cfd 挂到 epoll 监听树上
    struct epoll_event ev;
    ev.data.fd = cfd;

    // 边沿非阻塞模式
    ev.events = EPOLLIN | EPOLLET;

    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
    if (ret == -1)
    {
        perror("epoll_ctl add cfd error");
        server_exit(1);
    }
}