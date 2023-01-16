#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <error.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <unordered_set>
#include <signal.h>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <iostream>

#define BUF_SIZE    255


enum MessageType{
    UNKNOWN = 0,
    INFO = 1
};

struct Message{
    MessageType type;
    int fd_client;
    std::string mess;
};

char* string_char(std::string str)
{
    const int lenght = str.length();
    char* array = new char[lenght+1];
    strcpy(array,str.c_str());
    return array;
}

static void epoll_ctl_add(int epfd, int fd, uint32_t events)
{
	struct epoll_event ev;
	ev.events = events;
	ev.data.fd = fd;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		perror("epoll_ctl()\n");
		exit(1);
	}
}




int servFd;
int epollFd;
int socklen;
char buf[BUF_SIZE];

//void ctrl_c(int);

void sendToAllBut(int fd, char * buffer, int count);

uint16_t readPort(char * txt);

void setReuseAddr(int sock);


int main(int argc, char ** argv){
    if(argc != 2) error(1, 0, "Need 1 arg (port)");
    auto port = readPort(argv[1]);
    
    servFd = socket(AF_INET, SOCK_STREAM, 0);
    if(servFd == -1) error(1, errno, "socket failed");
    
    //signal(SIGINT, ctrl_c);
    signal(SIGPIPE, SIG_IGN);
    
    setReuseAddr(servFd);
    
    sockaddr_in serverAddr{.sin_family=AF_INET, .sin_port=htons((short)port), .sin_addr={INADDR_ANY}};
    int res = bind(servFd, (sockaddr*) &serverAddr, sizeof(serverAddr));
    if(res) error(1, errno, "bind failed");
    
    res = listen(servFd, 1);
    if(res) error(1, errno, "listen failed");

    ssize_t n;


    epollFd = epoll_create1(0);
    
    struct epoll_event events[32];

    epoll_ctl_add(epollFd,servFd,(EPOLLIN | EPOLLOUT | EPOLLET | EPOLLHUP));
    
    while(true){
        int wait=epoll_wait(epollFd, events, 32, -1);
        if(-1 == wait) {
            error(0,errno,"epoll_wait failed");
            //ctrl_c(SIGINT);
        }

        for(int i=0;i<wait;i++)
        {
            if(events[i].data.fd == servFd) // nowi klienci
            {
                sockaddr_in clientAddr{};
                socklen_t clientAddrSize = sizeof(clientAddr);
                auto cliFD = accept(servFd,(sockaddr*)&clientAddr,&clientAddrSize); //fd klienta
                if(cliFD == -1)
                {
                    printf("error");
                }
                epoll_ctl_add(epollFd,cliFD,EPOLLIN | EPOLLET | EPOLLRDHUP |
					      EPOLLHUP); //dodajemy go z eventami od niego

                std::cout<<"Nowy client, fd: "<<cliFD<<" !\n";
            }

         /*   if(events[i].events & EPOLLIN)
            {
                char buffer[256];
                ssize_t count = read(_fd, buffer, 256);
                if(buffer[0]=='R')
                    printf("asdasdad");
                else
                    events |= EPOLLERR;
            }
        */


            if(events[i].events & (EPOLLRDHUP | EPOLLHUP)) //wylogowanie klienta
            {
                std::cout<<"connection closed with fd: "<<events[i].data.fd<<"\n";
                epoll_ctl(epollFd,EPOLL_CTL_DEL,events[i].data.fd,NULL); //usuniecie z epolli
                close(events[i].data.fd); //zamkniecie zeby nie smiecic
                continue;
            }
        }

        
    }
}

uint16_t readPort(char * txt){
    char * ptr;
    auto port = strtol(txt, &ptr, 10);
    if(*ptr!=0 || port<1 || (port>((1<<16)-1))) error(1,0,"illegal argument %s", txt);
    return port;
}

void setReuseAddr(int sock){
    const int one = 1;
    int res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if(res) error(1,errno, "setsockopt failed");
}


