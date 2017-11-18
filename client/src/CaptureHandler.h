//
// Created by dilin on 11/18/17.
//

#ifndef CLIENT_CAPTUREHANDLER_H
#define CLIENT_CAPTUREHANDLER_H

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/v4l2-common.h>
#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

class CaptureException : public exception {
public:
    CaptureException(string msg)
    {
        this->msg = msg;
    }

    const char *what() const throw()
    {
        return msg.c_str();
    }

    string msg;
};

class CaptureHandler
{
public:
    CaptureHandler(string device,
                   unsigned int width,
                   unsigned int height);
    Mat &capture();
    ~CaptureHandler();
    unsigned char* buffer = nullptr;
private:
    int fd; // A file descriptor to the video device
    int type;
    v4l2_buffer bufferinfo;
    Mat img;
};


#endif //CLIENT_CAPTUREHANDLER_H
