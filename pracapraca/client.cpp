#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <error.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>

bool czy_znak_jest_litera(char*);


ssize_t readData(int fd, char * buffer, ssize_t buffsize){
    auto ret = read(fd, buffer, buffsize);
    if(ret==-1) error(1,errno, "read failed on descriptor %d", fd);
    return ret;
}

void writeData(int fd, char * buffer, ssize_t count){
    auto ret = write(fd, buffer, count);
    if(ret==-1) error(1, errno, "write failed on descriptor %d", fd);
    if(ret!=count) error(0, errno, "wrote less than requested to descriptor %d (%ld/%ld)", fd, count, ret);
}

struct Handler {
    virtual bool handleEvent(uint32_t events) = 0;
};

struct ReaderWriter : Handler {
    int from, to;
    ReaderWriter(int from, int to): from(from), to(to) {}
    virtual bool handleEvent(uint32_t events) override {
        if(events & EPOLLIN) {
            char buffer[255];
            ssize_t count = readData(from, buffer, 255);
            std::cout<<"endl";
            writeData(to, buffer, count);
        }
        if(events & ~EPOLLIN)
            return false;
        return true;
    }
};

int main(int argc, char ** argv){
    if(argc!=3) error(1,0,"Need 2 args");
    
    addrinfo *resolved, hints={.ai_flags=0, .ai_family=AF_INET, .ai_socktype=SOCK_STREAM};
    int res = getaddrinfo(argv[1], argv[2], &hints, &resolved);
    if(res || !resolved) error(1, 0, "getaddrinfo: %s", gai_strerror(res));
    
    int sock = socket(resolved->ai_family, resolved->ai_socktype, 0);
    if(sock == -1) error(1, errno, "socket failed");
    
    res = connect(sock, resolved->ai_addr, resolved->ai_addrlen);
    if(res) error(1, errno, "connect failed");
    
    freeaddrinfo(resolved);
    
    int efd = epoll_create1(0);
    
    epoll_event ee;
    ee.events = EPOLLIN|EPOLLRDHUP;
    
    ReaderWriter stdinToSocket(STDIN_FILENO, sock);
    ee.data.ptr = &stdinToSocket;
    epoll_ctl(efd, EPOLL_CTL_ADD, STDIN_FILENO, &ee);
    
    ReaderWriter socketToStdout(sock, STDOUT_FILENO);
    ee.data.ptr = &socketToStdout;
    epoll_ctl(efd, EPOLL_CTL_ADD, sock, &ee);
    
    while(1){
        if(-1 == epoll_wait(efd, &ee, 1, -1)) {
            shutdown(sock, SHUT_RDWR);
            close(sock);
            error(1,errno,"epoll_wait failed");
        }
        
        Handler * handler = (Handler*)(ee.data.ptr);
        if(!handler->handleEvent(ee.events))
            break;
    }
    
    shutdown(sock, SHUT_RDWR);
    close(sock);
    return 0;
}



