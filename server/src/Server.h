//
// Created by dilin on 10/18/17.
//

#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H

#include <iostream>
#include<string>
#include<opencv2/opencv.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <vector>
#include "connection.hpp" // Must come before boost/serialization headers.
#include <boost/serialization/vector.hpp>
#include "Frame.h"
#include "FrameQueue.h"

using namespace std;
using namespace cv;
using boost::asio::ip::tcp;


class Server
{
public:
    Server(boost::asio::io_service& io_service,
           unsigned short port,
           FrameQueue & frames);

private:
    boost::asio::ip::tcp::acceptor acceptor_;
    void handle_accept(const boost::system::error_code& e, connection_ptr conn);
    void handle_read(const boost::system::error_code& e, connection_ptr conn);
    Frame frame;
    FrameQueue & frames;

};


#endif //SERVER_SERVER_H
