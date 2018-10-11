#include "LEMF.h"

LEMF::LEMF()
{
    init();
}

LEMF::~LEMF()
{
    if(server_thread.joinable())
        server_thread.join();
}

bool LEMF::init()
{
    std::cout<<"Init new LEMF"<<std::endl;

    /*
     * Create the server socket.
     */

    if((server_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        std::cout<<"Failed to initialize LEMP socket"<<std::endl;
        return false;
    }

    /*
     * Adjust the server socket to allow multiple connections.
     */

    if( setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) < 0 )
    {
        std::cout<<"Failed to adjust the LEMP socket·"<<std::endl;
        return false;
    }
    
    /*
     * Set the socket type.
     */

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(5000);

    /*
     * Bind the socket.
     */

    if (bind(server_socket, (struct sockaddr *)&address, sizeof(address)) < 0) 
    {
        std::cout<<"Failed to bind the LEMP socket"<<std::endl;
        return false;
    }

    /*
     * Specify the number of simultanious server connections.
     */

    if (listen(server_socket, 3) < 0)
    {
        std::cout<<"Failed to specify the maximum number of pending connections."<<std::endl;
    }

    addrlen = sizeof(address);

    return true;
}

bool LEMF::start_server()
{
    server_thread = std::thread(([this](){this->run();}));
    return server_thread.joinable();    
}

void LEMF::run()
{
    while(1)
    {
        if ((new_socket = accept(server_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
        {
            std::cout<<"LEMF trigger accept error"<<std::endl;
        }

        std::cout<<"New trigger from Kamailio proxy. File descriptor is " << new_socket
        <<  " at address " << inet_ntoa(address.sin_addr) 
        << " on port " << ntohs(address.sin_port) <<"."<<std::endl;

        if((valread = recv(new_socket, buffer, BUFFER_SIZE, 0) <= 0))
        {
            std::cout<<"Read error"<<std::endl;
        }

        std::string peers = std::string(buffer);

        Session s("10.10.10.8", "10.10.10.4");
        s.start_session();
        close(new_socket);
    }
}
