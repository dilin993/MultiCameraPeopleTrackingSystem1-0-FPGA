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
        vector<Server> servers(num_nodes);
        vector<Frame> frames(num_nodes);
        for(unsigned short n=0;n<num_nodes;n++)
        {
            servers[n] = Server(8080+n);
            servers[n].acceptConnection();
            cout << "Waiting for camera" << n << " to connect..." << endl;
            cout << "Camera" << n << " acquired connection!" << endl;
        }

        char chCheckForEscKey = 0;
        bool isFrameOne = true;

        while (chCheckForEscKey != 27)
        {
            for(unsigned short n=0;n<num_nodes;n++)
            {
                frames[n] = servers[n].receive();
                frames[n].print();
            }
        }
    }
    catch (std::exception& e)
    {
        cerr << e.what() << endl;
    }

    return(0);
}