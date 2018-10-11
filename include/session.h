#ifndef SESSION_H
#define SESSION_H

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

class Session
{
    public:
        Session(std::string user_1, std::string user_2);
        ~Session();
        bool start_session();
    
    private:

        struct Connection
        {
           Connection(){}
           Connection(int* _sock_):
              sock(_sock_){}; 
           int* sock;
        };

        bool init();
        void run();
        
        std::string user_1, user_2;
        int sock_user1, sock_user2, max_sd = 0, activity, valread;
        int port = 5000;
        struct sockaddr_in address;
        char buffer[4096];

        std::vector<Connection> connections;

        fd_set readfds;

        std::thread session_thread; 
};
#endif
