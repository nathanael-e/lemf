#include "session.h"

Session::client_connection::client_connection(int _sock_, SSL* _ssl_):
    sock(_sock_), ssl(_ssl_){}

Session::Session(std::string _user_1, std::string _user_2)
    : user_1(_user_1), user_2(_user_2)
{}

Session::~Session()
{
    if(session_thread.joinable())
        session_thread.join();
}

bool Session::init()
{
    /*
     * Initialize the SSL context.
     */ 

    SSL_load_error_strings();

    OpenSSL_add_ssl_algorithms();

    if(!create_ssl_context())
    {
        std::cout<<"LEMF failed to create SSL context"<<std::endl;
        return false;
    }

    if(!configure_ssl_context())
    {
        std::cout<<"LEMF failed to configure SSL context"<<std::endl;
        return false;
    }
    
    return true;
}

bool Session::create_ssl_context()
{
    const SSL_METHOD* method;
    method = TLSv1_2_client_method();
    ctx = SSL_CTX_new(method);

    if(!ctx)
    {
        return false;
    }

    return true;
}

bool Session::new_connect(std::string ip)
{
    int sock;

    /*
     * Initialize the socket.
     */
    if((sock = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        std::cout<<"Failed to initialize LEMP socket"<<std::endl;
        return false;
    }

    /*
     * Set the address.
     */
     address.sin_addr.s_addr = inet_addr(ip.c_str());
     address.sin_family = AF_INET;
     address.sin_port = htons(port); 

    /*
     * Connect to LEMP.
     */
    if (connect(sock, (struct sockaddr *)&address , sizeof(address)) < 0)
    {
        std::cout<<"Failed to connect to LEMP"<<std::endl;
        return false;
    }
 
    /*
     * Create a new SSL connection.
     */ 
    SSL* ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock);

    if (SSL_connect(ssl) == -1)
    {
        ERR_print_errors_fp(stderr);
        return false;
    }

    connections.push_back(client_connection(sock, ssl));

    return true; 
}

bool Session::start_session()
{
    session_thread = std::thread(([this](){this->run();})); 
    return session_thread.joinable();
}

bool Session::configure_ssl_context()
{
    if((SSL_CTX_use_certificate_file(ctx, "/home/kamailio/cert.pem", SSL_FILETYPE_PEM) <= 0) ||
       (SSL_CTX_use_PrivateKey_file(ctx, "/home/kamailio/key.pem", SSL_FILETYPE_PEM) <= 0 ))
    {
       return false;
    }

    return true;
}


void Session::run()
{
    if(!init())
    {
        return;
    }

    if(!new_connect(user_1))
    {
        std::cout<<"Failed to establish connection to caller: "<<user_1<<std::endl;
    }

    if(!new_connect(user_2))
    {
        std::cout<<"Failed to establish connection to the called party: "<<user_2<<std::endl;   
    }
      
    while(1)
    {
        max_sd = 0;  
        FD_ZERO(&readfds);
          
        for(const auto& connection:connections)
        {
            if(connection.sock > 0)  
                FD_SET(connection.sock, &readfds);

            std::cout<<"Socket number is: "<<connection.sock<<std::endl;
            
            if(connection.sock > max_sd)
                max_sd = connection.sock;
        }

        if(max_sd == 0)
        {
            std::cout<<"Session has ended"<<std::endl;
            return;
        }

        activity = select(max_sd + 1 , &readfds , NULL , NULL , NULL);

        if(activity < 0)
            std::cout<<"LEMF select error"<<std::endl;
        else
        {
            for(auto& connection:connections)
            {
                if(FD_ISSET(connection.sock, &readfds)) 
                {
                    SSL_set_fd(connection.ssl, connection.sock);

                    if((valread = SSL_read(connection.ssl, buffer, 4095) <= 0))
                    {
                       std::cout<<"Lost connection to the client"<<std::endl; 
                    }
                    else
                    {
                        std::cout<<buffer<<std::endl;
                        memset(buffer, 0, sizeof buffer);
                    }
                    
                    SSL_free(connection.ssl);
                    close(connection.sock);
                    connection.ssl = NULL;
                    connection.sock = 0; 
                }
            }
        }
    } 
}
