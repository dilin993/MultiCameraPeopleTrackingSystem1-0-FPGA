//
// Created by dilin on 11/11/17.
//

#include <iostream>
#include <thread>
#include "Server.h"
#include "pugixml.hpp"
#include "CameraConfig.h"
#include "DataAssociation.h"
#include "graph.h"

#define DISPLAY_FLAG 1

using namespace std;

unsigned short port = 8080;

void server_thread_func(FrameQueue & frames)
{
    boost::asio::io_service io_service;
    Server server(io_service,port,frames);
    io_service.run();
}

int main(int argc, const char * argv[])
{
    try
    {
        FrameQueue frames;
        thread server_thread(server_thread_func,ref(frames));
        for(;;)
        {
            if(!frames.is_empty())
            {
                Frame frame = frames.dequeue();
                frame.print();
            }
        }

        server_thread.join();
    }
    catch (std::exception& e)
    {
        cerr << e.what() << endl;
    }

    return(0);
}