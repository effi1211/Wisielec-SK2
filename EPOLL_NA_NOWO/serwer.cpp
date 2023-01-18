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
#include <fstream>
#include <string.h>
#include <iostream>
//#include "./include/wisielec.h"
//#include "./include/message.h"

#define BUF_SIZE    255
#define INFO "INFO"
#define ST "START"
#define READY "READY"
//clienci

struct Client{
    int fd;
    std::string wisielec;
    std::string name;
    std::string odgadywane_haslo;
    int points = 0;
    int trys=0;
    int acctive=0;
};

Client clients[100];
int numClients = 0;
int end_scorse = 0;

void addClient(int fd)
{
    clients[fd].fd = fd;
    clients[fd].name="";
    clients[fd].points=0;
    clients[fd].trys=0;
    clients[fd].acctive=0;

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

//wisielec 
std::string hasla[25];
std::string haslo;
std::string kategoria;
std::string zakodowane_haslo;

void wczytanie_hasel(std::string nazwa_pliku) 
{
    std::fstream plik;
    plik.open(nazwa_pliku);
    for (int i = 0; i < 25; i++) 
    {
        plik >> hasla[i];
    }
    plik.close();

}


std::string kategorie()
{

    //losowanie kategorii
    srand(time(NULL));
    int plik_losowy = rand() % 9 + 1;
    std::string nazwa_kategorii;
    switch (plik_losowy)
    {
    case 1:
        nazwa_kategorii = "imie.txt";
        kategoria = "IMIE";
        break;
    case 2:
        nazwa_kategorii = "zawod.txt";
        kategoria = "ZAWOD";
        break;
    case 3:
        nazwa_kategorii = "zwierze.txt";
        kategoria = "ZWIERZE";
        break;
    case 4:
        nazwa_kategorii = "panstwo.txt";
        kategoria = "PANSTWO";
        break;
    case 5:
        nazwa_kategorii = "miasto.txt";
        kategoria = "MIASTO";
        break;
    case 6:
        nazwa_kategorii = "jedzenie.txt";
        kategoria = "JEDZENIE";
        break;
    case 7:
        nazwa_kategorii = "roslina.txt";
        kategoria = "ROSLINA";
        break;
    case 8:
        nazwa_kategorii = "sport.txt";
        kategoria = "SPORT";
        break;
    case 9:
        nazwa_kategorii = "rzecz.txt";
        kategoria = "RZECZ";
        break;
    default:
        nazwa_kategorii = "imie.txt";
        kategoria = "IMIE";
    }

    return nazwa_kategorii;

}

void przygotowanie_gry()
{
    std::string kategoria_plik="./files/";
    kategoria_plik+=kategorie();

    wczytanie_hasel(kategoria_plik);

    int wylosowany_numer_hasla = rand() % 25;
    haslo = hasla[wylosowany_numer_hasla];

    for(int x=0;x<haslo.length();x++)
    {
        zakodowane_haslo+="_";
    }
    

}

//wisielec end
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
    if(pasujace == 0)
    {
        clients[client.fd].trys++;
    }
    
    return pasujace;
}

void send_to_all_active(std::string message,std::string type)
{
    for(int j=0; j<100;j++)
    {
        if(clients[j].acctive==1)
        {
            write_message(message,type,j);
        }
    }
}

int servFd;
int epollFd;
int socklen;
char buf[BUF_SIZE];
int winner=0;


void ctrl_c(int);

void sendToAllBut(int fd, char * buffer, int count);

uint16_t readPort(char * txt);

void setReuseAddr(int sock);

char duza_litera(char znak);



int main(int argc, char ** argv){
    if(argc != 2) error(1, 0, "Need 1 arg (port)");
    auto port = readPort(argv[1]);
    
    servFd = socket(AF_INET, SOCK_STREAM, 0);
    if(servFd == -1) error(1, errno, "socket failed");
    
    signal(SIGINT, ctrl_c);
    signal(SIGPIPE, SIG_IGN);
    
    setReuseAddr(servFd);
    
    sockaddr_in serverAddr{.sin_family=AF_INET, .sin_port=htons((short)port), .sin_addr={INADDR_ANY}};
    int res = bind(servFd, (sockaddr*) &serverAddr, sizeof(serverAddr));
    if(res) error(1, errno, "bind failed");
    
    res = listen(servFd, 1);
    if(res) error(1, errno, "listen failed");

    ssize_t n;

    przygotowanie_gry();
    std::cout<<"kategoria "<<kategoria<<"\n haslo "<<haslo<<"\nzakodowane haslo "<<zakodowane_haslo<<"\n";

    epollFd = epoll_create1(0);
    
    struct epoll_event events[32];

    epoll_ctl_add(epollFd,servFd,(EPOLLIN | EPOLLOUT | EPOLLET | EPOLLHUP));
    
    Message mess;

    while(true){
        if(end_scorse==numClients && end_scorse>0)
            {

                std::cout<<"KONIEC GRY\n"<<end_scorse<<" "<<numClients;
                std::string ranking="RANKING ";
                for(int j=0; j<100;j++)
                {
                    
                    if(clients[j].acctive == 1)
                    {   
                        //clients[j].name="\nx";
                        std::string sep = ": ";
                        ranking += clients[j].name ;
                        ranking += sep ;
                        std::string points = std::to_string(clients[j].points);
                        ranking += points;   
                        ranking += "\n";
                    }
                    //std::cout<<ranking<<"\n";
                }
                send_to_all_active(ranking,"BREAK");
                
                close(servFd);
                printf("Closing server\n");
                exit(0);
                
                
            }
        int wait=epoll_wait(epollFd, events, 32, -1);
        if(-1 == wait) {
            error(0,errno,"epoll_wait failed");
            ctrl_c(SIGINT);
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
                clients[cliFD].acctive=1;


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
                        write_message(": Witaj w Wisielcu",ST,events[i].data.fd);
                    }
                    if(mess_cli.type == "READY")
                    {   
                        std::cout<<mess_cli.type<<"\n";
                        clients[events[i].data.fd].name = mess_cli.wiadomosc;
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
                    }
                    if(mess_cli.type == "PYTLIT")
                    {
                        
                        std::cout<<mess_cli.type<<"\n";

                        clients[events[i].data.fd].points += 
                        sprawdzenie(mess_cli.wiadomosc,haslo,clients[events[i].data.fd]);
                        
                        std::cout<<clients[events[i].data.fd].points<<std::endl;
                        
                        if(clients[events[i].data.fd].trys ==10)
                        {
                            end_scorse++;
                            std::cout<<end_scorse<<"\n";
                            write_message("Skonczyly ci sie proby,poczekaj na reszte\n","WAIT",events[i].data.fd);
                        }
                        else if(clients[events[i].data.fd].odgadywane_haslo == haslo)
                        {   
                            if(winner==0)
                            {
                                end_scorse++;
                                clients[events[i].data.fd].points++;
                                write_message(clients[events[i].data.fd].odgadywane_haslo,"WIN",events[i].data.fd);
                                winner=events[i].data.fd;
                            }
                            else{
                                end_scorse++;
                                std::string end_not_first;
                                end_not_first = clients[events[i].data.fd].odgadywane_haslo + " Zgadles, ale nie pierwszy :(";
                                write_message(end_not_first,"WAIT",events[i].data.fd);
                            }
                        }
                        else{
                        write_message(clients[events[i].data.fd].odgadywane_haslo,"GAME",events[i].data.fd);
                        }
                    }
                    
                }   
            }
    
            if(events[i].events & (EPOLLRDHUP | EPOLLHUP)) //wylogowanie klienta
            {
                std::cout<<"connection closed with fd: "<<events[i].data.fd<<"\n";
                epoll_ctl(epollFd,EPOLL_CTL_DEL,events[i].data.fd,NULL); //usuniecie z epolli
                close(events[i].data.fd); //zamkniecie zeby nie smiecic
                clients[events[i].data.fd].acctive=0;
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

void ctrl_c(int){
    send_to_all_active("ZAMKNIECIE SERWERA","BREAK");
    close(servFd);
    printf("Closing server\n");
    exit(0);
}
