#pragma once

#include <string>
#include <sstream>
#include <stdio.h>
#include <cstring>


#define INFO "INFO"

struct Message{
    std::string type;
    std::string od_kogo;
    std::string wiadomosc;
};

std::string kodowanie_waid(Message mess);

Message odkodowanie_waid(std::string wiad);

