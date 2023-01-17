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
#define INFO "INFO"
#define ST "START"
#define READY "READY"

//#include "./include/message.h"

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



void write_message(std::string message,std::string type, int sock)
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

int main(int argc, char ** argv){
    if(argc!=3) error(1,0,"Need 2 args");
    
    // Resolve arguments to IPv4 address with a port number
    addrinfo *resolved, hints={.ai_flags=0, .ai_family=AF_INET, .ai_socktype=SOCK_STREAM};
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
    
    int piszesz_odczytujesz = 0 ;
/****************************/

    Message mess,mess_zwr;
    /*mess.type=ST;
    mess.wiadomosc="\n";

    std::string str_mess=kodowanie_waid(mess);
    int cnt = str_mess.length();
    memcpy(buffer,str_mess.data(),str_mess.size());
    writeData(sock,buffer,cnt);*/

    write_message("",ST,sock);
    
    while(true){
        ssize_t bufsize = 255, received;
        char buffer[bufsize];
        received = readData(sock, buffer, bufsize);
        if(received>0)
        {
            //ODBIERA W KOLKO WIADOMOSCI OD SERWERA W ZALEZNOSCI JAKI PREFIX TO COS BD ROBIC
           // std::cout<<buffer<<"\n";
            std::string str_mess = std::string(buffer);
            //std::cout<<mess_zwr.type<<"\n";
            mess_zwr=odkodowanie_waid(str_mess);
            
          //  std::cout<<mess_zwr.wiadomosc;
            if(mess_zwr.type == ST) // odebranie wiadomosci startowej i wyslanie stanu gotowosci
            {
                std::cout<<mess_zwr.wiadomosc<<"\n";
                write_message("a",READY,sock);

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

            
        }
        memset(buffer,0,255);
    }

    
/****************************/

}