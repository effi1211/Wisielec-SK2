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
#include <iostream>
#include <fstream>
#include <cstring>

//-----------------------------

#define DL 25 //ilosc slow w kategorii
#define LP 10 //liczba prob (po ilu blednych literach przegrywasz)

char alfabet[26];
char litera;
char* kate;
char* haslo;
std::string hasla[DL];
std::string wisielec[50];

//--------------------------------

class Client;

int servFd;
int epollFd;
std::unordered_set<Client*> clients;

void ctrl_c(int);

void sendToAllBut(int fd, char * buffer, int count);

uint16_t readPort(char * txt);

void setReuseAddr(int sock);

//----------------------------------------------

void wczytanie_hasel(std::string nazwa_pliku) 
{
    std::fstream plik;
    plik.open("imie.txt");
    for (int i = 0; i < DL; i++) 
    {
        plik >> hasla[i];
    }
    plik.close();

}

void writeData(int fd, char * buffer, ssize_t count){
    auto ret = write(fd, buffer, count);
    if(ret==-1) error(1, errno, "write failed on descriptor %d", fd);
    if(ret!=count) error(0, errno, "wrote less than requested to descriptor %d (%ld/%ld)", fd, count, ret);
}

std::string kategoria()
{

    //losowanie kategorii
    srand(time(NULL));
    int plik_losowy = rand() % 9 + 1;
    std::string kategoria, nazwa_kategorii;
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

    return kategoria;

}

char* string_char(std::string str)
{
    const int lenght = str.length();
    char* array = new char[lenght+1];
    strcpy(array,str.c_str());
    return array;
}

ssize_t dlugosc_slowa(char* slowo)
{
    return sizeof(slowo);
}

void wczytanie_wisielca()
{
    std::fstream dane;
    dane.open("tworzenie_wisielca.txt");
    for (int i = 0; i < 50; i++)
    {
        getline(dane, wisielec[i]);
    }
    dane.close();
}

//-----------------------------------------------

struct Handler {
    virtual ~Handler(){}
    virtual void handleEvent(uint32_t events) = 0;
};

class Client : public Handler {
    int _fd;
public:
    Client(int fd) : _fd(fd) {
        epoll_event ee {EPOLLIN|EPOLLRDHUP, {.ptr=this}};
        epoll_ctl(epollFd, EPOLL_CTL_ADD, _fd, &ee);
    }
    virtual ~Client(){
        epoll_ctl(epollFd, EPOLL_CTL_DEL, _fd, nullptr);
        shutdown(_fd, SHUT_RDWR);
        close(_fd);
    }
    int fd() const {return _fd;}
    virtual void handleEvent(uint32_t events) override {
        if(events & EPOLLIN) {
            char buffer[256];
            ssize_t count = read(_fd, buffer, 256);
            if(count > 0)
                sendToAllBut(_fd, buffer, count);
            else
                events |= EPOLLERR;
        }
        if(events & ~EPOLLIN){
            remove();
        }
    }
    void write(char * buffer, int count){
        if(count != ::write(_fd, buffer, count))
            remove();
        
    }
    void remove() {
        printf("removing %d\n", _fd);
        clients.erase(this);
        delete this;
    }
};

class : Handler {
    public:
    virtual void handleEvent(uint32_t events) override {
        if(events & EPOLLIN){
            sockaddr_in clientAddr{};
            socklen_t clientAddrSize = sizeof(clientAddr);
            //akceptowanie nowego polaczenia
            auto clientFd = accept(servFd, (sockaddr*) &clientAddr, &clientAddrSize);
            if(clientFd == -1) error(1, errno, "accept failed");
            //wys kategori dla nowego klienta
            ssize_t count_bufser=dlugosc_slowa(kate);
            char bufser[static_cast<int>(count_bufser)];
            strcpy(bufser,kate);
            
            writeData(clientFd,bufser,count_bufser);

            //wys hasla dla nowego klienta
            count_bufser=dlugosc_slowa(haslo);
            bufser[static_cast<int>(count_bufser)];
            strcpy(bufser,haslo);
            
            writeData(clientFd,bufser,count_bufser);

            printf("new connection from: %s:%hu (fd: %d)\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), clientFd);
            
            clients.insert(new Client(clientFd));
        }
        if(events & ~EPOLLIN){
            error(0, errno, "Event %x on server socket", events);
            ctrl_c(SIGINT);
        }
    }
} servHandler;

int main(int argc, char ** argv){
    std::string pom = kategoria();
    wczytanie_hasel(pom);
    kate = string_char(pom);
    wczytanie_hasel(pom);
    
    wczytanie_wisielca();

    int wylosowany_numer_hasla = rand() % DL;

    std::string wylosowane_haslo = hasla[wylosowany_numer_hasla];

    //tworzenie ukrytego hasla i tabeli dostepnych liter

    std::string ukryte_haslo(wylosowane_haslo.length(), '_');
    haslo=string_char(ukryte_haslo);


    if(argc != 2) error(1, 0, "Need 1 arg (port)");
    auto port = readPort(argv[1]);
    
    servFd = socket(AF_INET, SOCK_STREAM, 0);
    if(servFd == -1) error(1, errno, "socket failed");
    

    signal(SIGINT, ctrl_c); //wyjscie z programu ctrl_c
    signal(SIGPIPE, SIG_IGN);//nie zasmiecaniie starymi socketami?
    
    setReuseAddr(servFd);
    //bind to any addres and port
    sockaddr_in serverAddr{.sin_family=AF_INET, .sin_port=htons((short)port), .sin_addr={INADDR_ANY}};
    int res = bind(servFd, (sockaddr*) &serverAddr, sizeof(serverAddr));
    if(res) error(1, errno, "bind failed");
    //nasłuch
    res = listen(servFd, 1);
    if(res) error(1, errno, "listen failed");

    epollFd = epoll_create1(0);
    
    epoll_event ee {EPOLLIN, {.ptr=&servHandler}};
    epoll_ctl(epollFd, EPOLL_CTL_ADD, servFd, &ee);
    
    while(true){
        if(-1 == epoll_wait(epollFd, &ee, 1, -1)) {
            error(0,errno,"epoll_wait failed");
            ctrl_c(SIGINT);
        }
        ((Handler*)ee.data.ptr)->handleEvent(ee.events);
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

void ctrl_c(int){
    for(Client * client : clients)
        delete client;
    close(servFd);
    printf("Closing server\n");
    exit(0);
}

void sendToAllBut(int fd, char * buffer, int count){
    auto it = clients.begin();
    while(it!=clients.end()){
        Client * client = *it;
        it++;
        if(client->fd()!=fd)
            client->write(buffer, count);
    }
}