#include<iostream>
#include<thread>

#include "LEMF.h"

int main()
{
    LEMF lemf;
    if(lemf.start_server())
        std::cout<<"LEMF server started"<<std::endl;
    return 0;
}
