//
// Created by dilin on 10/18/17.
//

#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H

#include <iostream>
#include<string>
#include<opencv2/opencv.hpp>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include "Frame.h"
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

using namespace std;
using namespace cv;
using boost::asio::ip::tcp;

class ReceiveException : public exception
{
public:
    ReceiveException(size_t bytes_transferred,boost::system::error_code error)
    {
        this->bytes_transferred = bytes_transferred;
        this->error = error;
    }

    const char * what () const throw ()
    {
        string errorStr =  to_string(bytes_transferred) +
                " bytes transferred. error code: " +
                boost::system::system_error(error).what();
        return errorStr.c_str();
    }
    size_t bytes_transferred;
    boost::system::error_code error;
};

class Server
{
public:
    Server(int port);
    Server();
    Server& operator=(const Server& other);
    void acceptConnection();
    Frame receive();

private:
    boost::asio::io_service io_service;
    tcp::acceptor acceptor;
    tcp::socket socket;
    int port;
};


#endif //SERVER_SERVER_H
