#include "../include/message.h"
#include <iostream>
#include <cstring>
#include <string>
#include <sstream>
#include <stdio.h>

#include "../include/message.h"


std::string kodowanie_waid(Message mess){
    std::string messfull;
    messfull = mess.type + ';' + mess.od_kogo+ ';'+ mess.wiadomosc +';';
    return messfull;
};

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
};




int main()
{
    Message mess;
    mess.type=INFO;
    mess.od_kogo = "MONIA";
    mess.wiadomosc="siemanero chillero";
    
    std::string str = kodowanie_waid(mess);
    std::cout<<str<<"\n";
    Message mess_zwr;
    mess_zwr = odkodowanie_waid(str);
    
    std::cout<<mess_zwr.type<<" "<<mess_zwr.od_kogo<<" "<<mess_zwr.wiadomosc<<"\n";

}
