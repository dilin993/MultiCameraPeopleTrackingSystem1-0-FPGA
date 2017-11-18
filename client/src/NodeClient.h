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
#include "connection.hpp"
#include "CaptureHandler.h"
#include "BGSDetector.h"


using namespace std;
using namespace cv;
using boost::asio::ip::tcp;

class NodeClient
{
public:
    NodeClient(boost::asio::io_service& io_service,
               string ip,
               unsigned short port,
               unsigned int width,
               unsigned int height,
               string videoSource,
               uint8_t cameraID);


private:
    connection connection_;
    void handle_connect(const boost::system::error_code& e);
    void handle_write(const boost::system::error_code& e);
    Frame frame;
    unsigned int width;
    unsigned int height;
    VideoCapture cap;
    uint8_t cameraID;
    uint16_t frameNo;
    void capture_frame();
    BGSDetector detector;
    Mat img;

};


#endif //NODE_NODECLIENT_H
