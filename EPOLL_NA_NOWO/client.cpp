#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <error.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <poll.h>

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
    
    int esp = epoll_create1(0);
    epoll_event w;
   
    w.events = EPOLLIN;
    w.data.u64 = sock;
    epoll_ctl(esp,EPOLL_CTL_ADD,sock,&w);

    ssize_t received;
    char buffer[255];
    
    while(1){
        if( -1 == epoll_wait(esp, &w, 1, -1)) {
            shutdown(sock, SHUT_RDWR);
            close(sock);
            error(1,errno,"epoll_wait failed");
        }
        if(w.events & EPOLLIN){
            if(w.data.fd == sock){
                received = readData(0, buffer, 255);
                writeData(sock, buffer, received);
            }
        }

        }
    }



