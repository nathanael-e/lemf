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
#include <string.h>
/* SSL Headers */

#include <openssl/ssl.h>
#include <openssl/err.h>

class Session
{
    public:
        Session(std::string user_1, std::string user_2);
        ~Session();
        bool start_session();
    
    private:

        struct client_connection
        {
           client_connection(int sock, SSL* ssl);
           int sock;
           SSL* ssl;
        };

        bool create_ssl_context();
        bool configure_ssl_context();
        bool new_connect(std::string ip);

        bool init();
        void run();
        
        std::string user_1, user_2;
        int sock_user1, sock_user2, max_sd = 0, activity, valread;
        int port = 5000;
        struct sockaddr_in address;
        char buffer[4096];

        std::vector<client_connection> connections;

        fd_set readfds;

        std::thread session_thread; 

        SSL_CTX* ctx;
        std::string ack = "ACK";
};
#endif
