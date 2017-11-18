//
// Created by dilin on 11/11/17.
//

#include <iostream>
#include "Server.h"
#include "pugixml.hpp"
#include "CameraConfig.h"
#include "DataAssociation.h"
#include "graph.h"

#define DISPLAY_FLAG 1

using namespace std;

int main(int argc, const char * argv[])
{
    try
    {

        unsigned short port = 8080;

        boost::asio::io_service io_service;
        Server server(io_service,port);
        io_service.run();


    }
    catch (std::exception& e)
    {
        cerr << e.what() << endl;
    }

    return(0);
}