//
// Created by dilin on 10/18/17.
//

#ifndef NODE_NODECLIENT_H
#define NODE_NODECLIENT_H

#include <iostream>
#include<string>
#include<opencv2/opencv.hpp>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include "Frame.h"

using namespace std;
using namespace cv;
using boost::asio::ip::tcp;

class NodeClient
{
public:
    NodeClient(string ip,string port);
    void connect();
    void send(Frame frame);

private:
    boost::asio::io_service io_service;
    tcp::resolver resolver;
    tcp::socket socket;
    string ip;
    string port;
};


#endif //NODE_NODECLIENT_H
