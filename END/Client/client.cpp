#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <error.h>
#include <netdb.h>
#include <thread>
#include <cstdio>
#include <iostream>
#include <cstring>
#include <sys/epoll.h>
#include <unordered_set>
#include <signal.h>
#include <string.h>
#include <string>
#include <fstream>
#define INFO "INFO"
#define ST "START"
#define READY "READY"

std::string wisielec[50];
char alfabet[26];

void dostepne_litery() 
{
    for (int i = 0; i < 26; i++) 
    {
        alfabet[i] = i + 65;
    }
}

void dostepne_litery_aktualizacja(std::string znak) 
{
    for (int i = 0; i < 26; i++)
    {
        if (alfabet[i] == znak[0]) 
        {
            alfabet[i] = ' ';
        }
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



void write_message(const std::string& message,const std::string& type, int sock)
{
    char test[255];
    Message mess;
    mess.wiadomosc=message;
    mess.type=type;
    std::string str_mess = kodowanie_waid(mess);
    //std::cout<<str_mess<<"\n";
    ssize_t cnt = str_mess.length();
    memcpy(test,str_mess.data(),str_mess.size());
    //std::cout<<test;
    
    writeData(sock,test,cnt);
    memset(test,0,255);

}
bool czy_znak_jest_litera(std::string znak)
{
    if ( (znak[0] < 65) || (znak[0] > 91 && znak[0] < 96) || (znak[0] > 122) ) {
        return false;
    }
    else return true;
}

std::string duza_litera(std::string znak) 
{
    if (znak[0] >= 96) 
    {
        znak[0] -= 32;
        return znak;
    }
    return znak;
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
void wyswietl_wisielca(int wis)
{
    for (int i = 0; i < 5; i++)
    {
        std::cout<<wisielec[(wis * 5) + i]<<"\n";
    }
}
void odkoduj_ranking(const std::string& ranking)
{
    std::string name;
    int wis;
    std::string rank=ranking;
    int sep=0;

    for(int i=0;i<(int)rank.length();i++)
    {
        if(rank[i]==';')
        {
            break;
        }
        if(sep == 1)
        {
            wis=(int)rank[i] - 48;
            //std::cout<<" WIS: "<<wis<<"/n";
            sep++;
        }
        if(sep==2)
        {
            std::cout<<name<<" \n";
            wyswietl_wisielca(wis);
            std::cout<<"\n\n";
            wis=0;
            sep=0;
        }
        if(rank[i]=='=')
        {
            name="";
            continue;
        }
        if(rank[i]=='-')
        {
            sep++;
           // std::cout<<"\n";
        }
        if(sep==0)
        {   //std::cout<<rank[i];
            name+=rank[i];
        }

    }
}

void game(int sock,int wis)
{   
    wyswietl_wisielca(wis);
    std::string litera;
    std::cout<<"\nDostepne litery:\n"<<alfabet<<"\n";
    std::cout<<"Podaj Litere\n";
    std::cin>>litera;
    while(litera == "ZW")
    {
        write_message("","WISIELCE",sock);
        ssize_t bufsizew = 255, receivedw;
        char bufferw[bufsizew];
        receivedw = readData(sock, bufferw, bufsizew);
        if(receivedw>0)
        {
            std::string str_mess2 = std::string(bufferw);
            Message mess_zwr2=odkodowanie_waid(str_mess2);
            odkoduj_ranking(mess_zwr2.wiadomosc);
            //std::cout<<mess_zwr2.wiadomosc<<"\n";
        }
        std::cout<<"\nDostepne litery:\n"<<alfabet<<"\n";
        std::cout<<"Podaj Litere\n";
        std::cin>>litera;

    }
    while(!czy_znak_jest_litera(litera)){
        std::cout<<"Podaj LITERE\n";
        std::cin>>litera;
    }
    litera=duza_litera(litera);

    dostepne_litery_aktualizacja(litera);
    write_message(litera,"PYTLIT",sock);
}

int main(int argc, char ** argv){
    if(argc!=3) error(1,0,"Need 2 args");
    
    // Resolve arguments to IPv4 address with a port number
    addrinfo *resolved;
    addrinfo hints = {};
    hints.ai_flags = 0;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_addrlen = 0;
    hints.ai_addr = nullptr;
    hints.ai_canonname = nullptr;
    hints.ai_next = nullptr;
    

    int res = getaddrinfo(argv[1], argv[2], &hints, &resolved);
    if(res || !resolved) error(1, 0, "getaddrinfo: %s", gai_strerror(res));
    
    // create socket
    int sock = socket(resolved->ai_family, resolved->ai_socktype, 0);
    if(sock == -1) error(1, errno, "socket failed");
    
    // attept to connect
    res = connect(sock, resolved->ai_addr, resolved->ai_addrlen);
    if(res) error(1, errno, "connect failed");
    
    std::cout<<sock;
    // free memory
    freeaddrinfo(resolved);
    
/****************************/
    wczytanie_wisielca();
/****************************/

    Message mess,mess_zwr;
    write_message("",ST,sock);
    dostepne_litery();
    
    while(true){
        ssize_t bufsize = 255, received;
        char buffer[bufsize];
        received = readData(sock, buffer, bufsize);
        if(received>0)
        {
            //ODBIERA W KOLKO WIADOMOSCI OD SERWERA W ZALEZNOSCI JAKI PREFIX TO COS BD ROBIC
            //std::cout<<"BUFFER: "<<buffer<<"\n";
            std::string str_mess = std::string(buffer);
            //std::cout<<mess_zwr.type<<"\n";
            mess_zwr=odkodowanie_waid(str_mess);
            
          //  std::cout<<mess_zwr.wiadomosc;
            if(mess_zwr.type == ST) // odebranie wiadomosci startowej i wyslanie stanu gotowosci
            {
                
                std::cout<<mess_zwr.wiadomosc<<"\n";
                std::cout<<"PODAJ SWOJ NICK:\n";
                std::string nick;
                std::cin>>nick;
                write_message(nick,READY,sock);

            }

            if(mess_zwr.type == READY)
            {
                std::cout<<"Kategoria: "<<mess_zwr.wiadomosc<<"\n";
                write_message("a","HASLO",sock);
            }
            if(mess_zwr.type == "HASLO")
            {
                std::cout<<"Haslo: "<<mess_zwr.wiadomosc<<"\n";
                write_message("a","GAME",sock);
            }
            if(mess_zwr.type == "GAME")
            {
                std::string str=mess_zwr.wiadomosc;
                int wis = (int) str[0]-48;
                //std::cout<<wis<<"\n";
                str.erase(0,1);
                std::cout<<str<<"\n";
                game(sock,wis);
            
            }
            if(mess_zwr.type == "WISIELCE")
            {
                std::cout<<mess_zwr.wiadomosc<<"\n"; 
            }
            if(mess_zwr.type  == "WAIT")
            {
                wyswietl_wisielca(9);
                std::cout<<mess_zwr.wiadomosc<<"\n";
                mess_zwr.type="";
            }
            if(mess_zwr.type == "WIN")
            {
                std::string str=mess_zwr.wiadomosc;
                int wis = (int) str[0]-48;
                std::cout<<wis<<"\n";
                str.erase(0,1);
                std::cout<<str<<"\n";
                std::cout<<"ZGADLES HASLO JAKO PIERWSZY: "<<str<<"\n Twoje punkty:"<<mess_zwr.wiadomosc.length()<<"\n Teraz poczekaj na reszte ;)\n";
                mess_zwr.type == "";
                wyswietl_wisielca(wis);

            }
            if(mess_zwr.type == "END")
            {
            
                std::cout<<mess_zwr.wiadomosc<<"\n";
                
            }
            if(mess_zwr.type == "BREAK")
            {
                std::cout<<mess_zwr.wiadomosc<<"\n";
                break;
            }


            
        }
        memset(buffer,0,255);
    }

    
/****************************/

}