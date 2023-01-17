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
#include "./include/wisielec.h"
//#include "./include/message.h"

#define BUF_SIZE    255
#define INFO "INFO"
#define ST "START"
#define READY "READY"
//clienci

struct Client{
    int fd;
    std::string wisielec;
    int points = 0;
};

Client clients[1000];
int numClients = 0;

void addClient(int fd)
{
    clients[numClients].fd = fd;
    numClients++;
}

//clienci end


//wiadomosci

struct Message{
    std::string type;
    std::string od_kogo;
    std::string wiadomosc;
};


std::string kodowanie_waid(Message mess){
    std::string messfull;
    messfull = mess.type + ';' + mess.od_kogo+ ';'+ mess.wiadomosc +';';
    return messfull;
}

Message odkodowanie_waid(std::string wiad){
    Message mess;
    //std::getline(wiad,mess.type,';');
   // std::getline(wiad,mess.od_kogo,';');
   // std::getline(wiad,mess.wiadomosc,';');



    int sep=0;
    int i=0;
    while (sep <3)
    {
        //std::cout<<wiad[i]<<"\n";
        if(wiad[i]==';')
        {
            sep++;
            i++;
            continue;
        }
        if(sep == 0)
        {
            mess.type+=wiad[i];
        }
        if(sep==1)
        {
            mess.od_kogo+=wiad[i];
        }
        if(sep==2)
        {
            mess.wiadomosc+=wiad[i];
        }
        i++;
    }


    return mess;
}
//waidomosci end
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

void write_message(std::string message,std::string type, int fd)
{
    char test[255];
    Message mess;
    mess.wiadomosc=message;
    mess.type=type;
    std::string str_mess = kodowanie_waid(mess);
    ssize_t cnt = str_mess.length();
    memcpy(test,str_mess.data(),str_mess.size());
    //std::cout<<test;
    
    writeData(fd,test,cnt);
    memset(test,0,255);
}

int servFd;
int epollFd;
int socklen;
char buf[BUF_SIZE];

//void ctrl_c(int);

void sendToAllBut(int fd, char * buffer, int count);

uint16_t readPort(char * txt);

void setReuseAddr(int sock);

char duza_litera(char znak);



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

    std::string haslo = "IMIE";
    std::string kategoria = "IMIE";
    


    epollFd = epoll_create1(0);
    
    struct epoll_event events[32];

    epoll_ctl_add(epollFd,servFd,(EPOLLIN | EPOLLOUT | EPOLLET | EPOLLHUP));
    
    Message mess;

    while(true){
        int wait=epoll_wait(epollFd, events, 32, -1);
        if(-1 == wait) {
            error(0,errno,"epoll_wait failed");
            //ctrl_c(SIGINT);
        }

        for(int i=0;i<wait;i++)
        {
            char test[255];
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
                addClient(cliFD);

            }

            if(events[i].events & EPOLLIN)// DZIALA ZLE
            {
                char buffer[255];
                //memset(buffer,0,255);

                ssize_t count = read(events[i].data.fd, buffer, 256);
                if(count > 0){ 
                    //std::cout<<buffer;
                    std::string str_mess = std::string(buffer);
                    Message mess_cli=odkodowanie_waid(str_mess);
                    std::cout<<"Client: "<<events[i].data.fd<<" Type: "<<mess_cli.type<<"\n";

                    if(mess_cli.type == ST)
                    {
/*                                        //wiadomosc powitalna nie
                        mess.wiadomosc="Witaj w Wisielcu";
                        mess.type=ST;
                        std::string str_mess = kodowanie_waid(mess);
                        int cnt = str_mess.length();
                        memcpy(test,str_mess.data(),str_mess.size());
                        //std::cout<<test;
                        
                        write(events[i].data.fd,test,cnt);
*/
                        write_message("Witaj w Wisielcu",ST,events[i].data.fd);
/*
                        mess.wiadomosc="nazwa";
                        mess.type="READY";
                        str_mess = kodowanie_waid(mess);
                        int cnt = str_mess.length();
                        memcpy(test,str_mess.data(),str_mess.size());
                        //std::cout<<test;
*/
                        //swrite(events[i].data.fd,test,cnt);
                        //write_message("kategoria",READY,events[i].data.fd);


                    }
                    if(mess_cli.type == "READY")
                    {   
                        std::cout<<mess_cli.type<<"\n";
                        write_message("imie",READY,events[i].data.fd);  
                    }
                    if(mess_cli.type == "HASLO")
                    {
                        std::cout<<mess_cli.type<<"\n";
                        write_message("_ _ _ _","HASLO",events[i].data.fd);  
                    }
                    if(mess_cli.type == "GAME")
                    {   
                        write_message("x","GAME",events[i].data.fd);

                        //std::cout<<"tu lecimy z grą, litera odebrana, sprawdzenie czzy jest litera,dodanie pkt,itd";
                        
                    }
                    if(mess_cli.type == "PYTLIT")
                    {
                        
                        
                        std::cout<<mess_cli.type<<"\n";
                        
                    }

                    //POTRZEBUJEMY JAKOS ZAPISYWAC CLIENTOW, tak

                }   
                
            }
        


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


char duza_litera(char znak) 
{
    if (znak >= 96) 
    {
        znak -= 32;
        return znak;
    }
    return znak;
}