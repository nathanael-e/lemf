#ifndef LEMF_H
#define LEMF_H

#include <iostream>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <sys/select.h>
#include "session.h"
#include <sstream>
#include<regex>

#define BUFFER_SIZE 4096

class LEMF
{
    public:
        LEMF();
        ~LEMF();
        bool start_server();

    private:
        bool init();
        void run();
        std::tuple<std::string, std::string> parse_ack(std::string ack);

        int server_socket, addrlen, new_socket,
            activity, sd, max_sd, on=1, valread;

        struct sockaddr_in address;
        char buffer[4096];
        std::thread server_thread;
};
#endif
