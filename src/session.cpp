#include "session.h"

Session::Session(std::string _user_1, std::string _user_2)
    : user_1(_user_1), user_2(_user_2), connections(0)
{
    init();
}

Session::~Session()
{
    if(session_thread.joinable())
        session_thread.join();
}

bool Session::init()
{
    /*
     * Sleep 2 seconds to make sure the clients have initilized their sockets.
     */ 
    
    sleep(2);

    /*
     * Initialize the socket to the caller.
     */

    if((sock_user1 = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        std::cout<<"Failed to initialize LEMP socket"<<std::endl;
        return false;
    }

     /*
     * Adjust the address to the caller.
     */

     address.sin_addr.s_addr = inet_addr(user_1.c_str());
     address.sin_family = AF_INET;
     address.sin_port = htons(port); 

    /*
     * Connect to the callers LEMP.
     */
    
    if (connect(sock_user1 , (struct sockaddr *)&address , sizeof(address)) < 0)
    {
        std::cout<<"Failed to connect to LEMP"<<std::endl;
        return false;
    }

    connections.push_back(Connection(&sock_user1));

    /*
     * Initialize the socket to the caller.
     */
    
    if((sock_user2 = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        std::cout<<"Failed to initialize LEMP socket"<<std::endl;
       return false;
    }

    /*
     * Initialize the address to the callee.
     */

    address.sin_addr.s_addr = inet_addr(user_2.c_str());
    address.sin_family = AF_INET;
    address.sin_port = htons(port); 

    /*
     * Initialize the socket to the callee.
     */

    if (connect(sock_user2 , (struct sockaddr *)&address , sizeof(address)) < 0)
    {
        std::cout<<"Failed to connect to LEMP"<<std::endl;
        return false;
    }

    connections.push_back(Connection(&sock_user2));

    return true;
}

bool Session::start_session()
{
    session_thread = std::thread(([this](){this->run();})); 
    return session_thread.joinable();
}

void Session::run()
{
   while(1)
   {
      max_sd = 0;
       
      FD_ZERO(&readfds);
      
      for(const auto& connection:connections)
      {
        if(*(connection.sock) > 0)  
            FD_SET(*(connection.sock), &readfds);

        std::cout<<"Socket number: "<<*(connection.sock)<<std::endl;
        
        if(*(connection.sock) > max_sd)
            max_sd = *(connection.sock);
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
              if(FD_ISSET(*(connection.sock), &readfds)) 
              { 
                  if((valread = read(*(connection.sock), buffer, 4095) <= 0))
                  {
                      std::cout<<"Session ended prematurly"<<std::endl;
                      close(*(connection.sock));   
                  }
                  else
                  {
                      std::cout<<buffer<<std::endl;
                  }
                
                  *(connection.sock) = 0;
              }
          }
      }
   } 
}
