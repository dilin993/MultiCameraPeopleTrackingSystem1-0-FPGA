//
// Created by dilin on 11/10/17.
//
#include <iostream>
#include "pugixml.hpp"
#include <opencv2/opencv.hpp>
#include "NodeClient.h"

using namespace std;
using namespace cv;






int main(int argc, const char * argv[])
{
    try
    {
        string serverIP="localhost";
        unsigned short serverPort = 8080;
        string videoSource="0";
        uint8_t cameraID=0;
        unsigned int width,height;

        if(argc>1)
        {
            pugi::xml_document doc;
            pugi::xml_parse_result result = doc.load_file(argv[1]);
            if(result)
            {
                pugi::xml_node config = doc.child("configuration");
                serverIP = config.child("server").attribute("ip").as_string();
                videoSource = config.child("video").attribute("source").as_string();
                width = (unsigned int)config.child("video").attribute("width").as_int();
                height = (unsigned int)config.child("video").attribute("height").as_int();
                cameraID = (uint8_t)config.child("camera").attribute("id").as_int();
                serverPort = (unsigned short)config.child("server").attribute("port").as_int();
            }
            else
            {
                cout << "Invalid configuration file." << endl;
                return -1;
            }
        }
        else
        {
            cout << "usage: ./client <configuration-file>" << endl;
            return -1;
        }

        boost::asio::io_service io_service;
        NodeClient client(io_service,
                          serverIP,
                          serverPort,
                          width,
                          height,
                          videoSource,
                          cameraID);
        io_service.run();

    }
    catch (exception &e)
    {
        cerr << "Error: " << e.what() << endl;
    }

}

