//
// Created by dilin on 11/11/17.
//

#include <iostream>
#include "Server.h"
#include "pugixml.hpp"

using namespace std;

int main(int argc, const char * argv[])
{
    unsigned short num_nodes = 0;

    if(argc>1)
    {
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_file(argv[1]);
        if(result)
        {
            pugi::xml_node config = doc.child("configuration");
            num_nodes = (unsigned short)config.child("main").attribute("num_nodes").as_int();
        }
    }

    try
    {
        Server servers[num_nodes];
        for(unsigned short n=0;n<num_nodes;n++)
        {
            servers[n] =
        }
        Server server1(8080);
        server1.acceptConnection();
        cout << "Camera1 aquired connection!" << endl;
        Server server2(8081);
        server2.acceptConnection();
        cout << "Camera2 aquired connection!" << endl;

        char chCheckForEscKey = 0;
        bool isFrameOne = true;

        while (chCheckForEscKey != 27)
        {

            try
            {
                Frame frame1 = server1.receive();
                cout << "################frame1################" << endl;
                frame1.print();
//                isFrameOne = false;
                Frame frame2 = server2.receive();
                cout << "################frame2################" << endl;
                frame2.print();
//                isFrameOne = true;
            }
            catch (ReceiveException& e)
            {
//                if(e.error!=boost::asio::error::eof)
//                    throw e;
////                else
////                {
////                    if(isFrameOne)
////                        server1.acceptConnection();
////                    else
////                        server2.acceptConnection();
////                }
//                else
//                    server1.acceptConnection();
//                continue;
                throw e;
            }
        }
    }
    catch (std::exception& e)
    {
        cerr << e.what() << endl;
    }

    return(0);
}