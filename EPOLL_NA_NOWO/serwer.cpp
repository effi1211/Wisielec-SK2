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
    std::string odgadywane_haslo;
    int points = 0;
    int trys=0;
};

Client clients[100];
int numClients = 0;
int end_scorse = 0;
int active_fd[100];

void addClient(int fd)
{
    clients[fd].fd = fd;
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

int sprawdzenie(std::string zgadywana_litera, std::string poprawne_haslo,Client &client)
{
    int pasujace = 0;
    int dlugosc = poprawne_haslo.length();
    for (int i = 0; i < dlugosc; i++)
    {

        if (zgadywana_litera[0] == client.odgadywane_haslo[i])
            return 0;

        if (zgadywana_litera[0] == poprawne_haslo[i])
        {
            client.odgadywane_haslo[i] = zgadywana_litera[0];
            pasujace+=1;
        }
    }
    clients[client.fd].trys++;
    return pasujace;
}

void send_to_all_active(std::string message,std::string type)
{
    for(int j=5; j<100;j++)
    {
        if(active_fd[j]==1)
        {
            write_message(message,type,j);
        }
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
    std::string zakodowane_haslo = "____";
    


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
                active_fd[cliFD]=1;


            }

            if(events[i].events & EPOLLIN)
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
                        write_message("Witaj w Wisielcu",ST,events[i].data.fd);
                    }
                    if(mess_cli.type == "READY")
                    {   
                        std::cout<<mess_cli.type<<"\n";
                        write_message(kategoria,READY,events[i].data.fd);  
                    }
                    if(mess_cli.type == "HASLO")
                    {
                        std::cout<<mess_cli.type<<"\n";
                        write_message(zakodowane_haslo,"HASLO",events[i].data.fd); 
                        clients[events[i].data.fd].odgadywane_haslo=zakodowane_haslo;
                    }
                    if(mess_cli.type == "GAME")
                    {   
                        write_message("x","GAME",events[i].data.fd);
                        
                        //std::cout<<"tu lecimy z grą, litera odebrana, sprawdzenie czzy jest litera,dodanie pkt,itd";
                        
                    }
                    if(mess_cli.type == "PYTLIT")
                    {
                        
                        std::cout<<mess_cli.type<<"\n";

                        clients[events[i].data.fd].points += 
                        sprawdzenie(mess_cli.wiadomosc,haslo,clients[events[i].data.fd]);
                        
                        std::cout<<clients[events[i].data.fd].points<<std::endl;
                        
                        if(clients[events[i].data.fd].trys >10)
                        {
                                end_scorse++;
                                write_message("Skonczyly ci sie proby,poczekaj na reszte\n","WAIT",events[i].data.fd);
                        }
                        else{
                        write_message(clients[events[i].data.fd].odgadywane_haslo,"GAME",events[i].data.fd);
                        }
                    }
                    if(end_scorse==numClients)
                    {
                        std::cout<<"KONIEC GRY";
                    }
                    
                }   
                
            }
        


            if(events[i].events & (EPOLLRDHUP | EPOLLHUP)) //wylogowanie klienta
            {
                std::cout<<"connection closed with fd: "<<events[i].data.fd<<"\n";
                epoll_ctl(epollFd,EPOLL_CTL_DEL,events[i].data.fd,NULL); //usuniecie z epolli
                close(events[i].data.fd); //zamkniecie zeby nie smiecic
                active_fd[events[i].data.fd]=0;
                numClients--;
                if(numClients<2)
                {
                    std::cout<<"za duzo uczestnikow wyszlo\n";
         
                    send_to_all_active("ZOSTALES SAM :((( ","BREAK");
                }
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