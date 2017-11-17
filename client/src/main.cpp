//
// Created by dilin on 11/10/17.
//
#include <iostream>
#include "pugixml.hpp"
#include <opencv2/opencv.hpp>
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
#include <csignal>
#include "BGSDetector.h"
#include "NodeClient.h"

using namespace std;
using namespace cv;

#define DISPLAY_MAIN "Display Window"

int fd; // A file descriptor to the video device
int type;

void signalHandler( int signum ) {
    cout << "Interrupt signal (" << signum << ") received.\n";

    // cleanup and close up stuff here
    // terminate program

    if(ioctl(fd, VIDIOC_STREAMOFF, &type) < 0)
    {
        perror("Could not end streaming, VIDIOC_STREAMOFF");
    }

    close(fd);
    exit(signum);
}

int main(int argc, const char * argv[])
{

#ifndef DISPLAY_MAIN
    signal(SIGINT, signalHandler);
#endif

    string serverIP="localhost";
    unsigned short serverPort = 8080;
    string videoSource="/dev/video0";
    uint8_t cameraID=0;

    if(argc>1)
    {
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_file(argv[1]);
        if(result)
        {
            pugi::xml_node config = doc.child("configuration");
            serverIP = config.child("server").attribute("ip").as_string();
            videoSource = config.child("video").attribute("source").as_string();
            cameraID = (uint8_t)config.child("camera").attribute("id").as_int();
            serverPort = (unsigned short)config.child("server").attribute("port").as_int();
        }
    }

    NodeClient client(serverIP,serverPort);
    client.connect();
    cout << "Connected to " << client.getIP() << ":" << client.getPort() << " !" << endl;

    /******************Initializing V4L2 Driver Starts Here**********************/
    // 1.  Open the device
    fd = open(videoSource.c_str(),O_RDWR);
    if(fd < 0){
        perror("Failed to open device, OPEN");
        return 1;
    }

    // 2. Ask the device if it can capture frames
    v4l2_capability capability;
    if(ioctl(fd, VIDIOC_QUERYCAP, &capability) < 0){
        // something went wrong... exit
        perror("Failed to get device capabilities, VIDIOC_QUERYCAP");
        return 1;
    }

    // 3. Set Image format
    v4l2_format imageFormat;
    imageFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    imageFormat.fmt.pix.width = 360;
    imageFormat.fmt.pix.height = 288;
    imageFormat.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    imageFormat.fmt.pix.field = V4L2_FIELD_NONE ;
    // tell the device you are using this format
    if(ioctl(fd, VIDIOC_S_FMT, &imageFormat) < 0){
        perror("Device could not set format, VIDIOC_S_FMT");
        return 1;
    }

    // 4. Request Buffers from the device
    v4l2_requestbuffers requestBuffer = {0};
    requestBuffer.count = 1; // one request buffer
    requestBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; // request a buffer wich we an use for capturing frames
    requestBuffer.memory = V4L2_MEMORY_MMAP;

    if(ioctl(fd, VIDIOC_REQBUFS, &requestBuffer) < 0){
        perror("Could not request buffer from device, VIDIOC_REQBUFS");
        return 1;
    }


    // 5. Quety the buffer to get raw data ie. ask for the you requested buffer
    // and allocate memory for it
    v4l2_buffer queryBuffer = {0};
    queryBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    queryBuffer.memory = V4L2_MEMORY_MMAP;
    queryBuffer.index = 0;
    if(ioctl(fd, VIDIOC_QUERYBUF, &queryBuffer) < 0){
        perror("Device did not return the buffer information, VIDIOC_QUERYBUF");
        return 1;
    }
    // use a pointer to point to the newly created buffer
    // mmap() will map the memory address of the device to
    // an address in memory
    unsigned char* buffer = (unsigned char*)mmap(NULL, queryBuffer.length, PROT_READ | PROT_WRITE, MAP_SHARED,
                               fd, queryBuffer.m.offset);
    memset(buffer, 0, queryBuffer.length);


    // 6. Get a frame
    // Create a new buffer type so the device knows whichbuffer we are talking about
    v4l2_buffer bufferinfo;
    memset(&bufferinfo, 0, sizeof(bufferinfo));
    bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo.memory = V4L2_MEMORY_MMAP;
    bufferinfo.index = 0;

    // Activate streaming
    type = bufferinfo.type;
    if(ioctl(fd, VIDIOC_STREAMON, &type) < 0)
    {
        perror("Could not start streaming, VIDIOC_STREAMON");
        return 1;
    }
    /******************Initializing V4L2 Driver Ends Here**********************/


    /***************************** Begin looping here *********************/
    Mat img(imageFormat.fmt.pix.height,
            imageFormat.fmt.pix.width,
            CV_8UC3);
    BGSDetector detector;


#ifdef DISPLAY_MAIN
    namedWindow(DISPLAY_MAIN);
#endif
    int p,k;
    uint16_t frameNo=0;

    for (;;)
    {
        // Queue the buffer
        // auto begin = std::chrono::high_resolution_clock::now();
        if(ioctl(fd, VIDIOC_QBUF, &bufferinfo) < 0){
            perror("Could not queue buffer, VIDIOC_QBUF");
            return 1;
        }

        // Dequeue the buffer
        if(ioctl(fd, VIDIOC_DQBUF, &bufferinfo) < 0){
            perror("Could not dequeue the buffer, VIDIOC_DQBUF");
            return 1;
        }

        for(k=0,p=0;(k<img.rows*img.cols);k+=2,p+=4)
        {
            img.at<Vec3b>(k)[0] = buffer[p];
            img.at<Vec3b>(k)[1] = buffer[p+1];
            img.at<Vec3b>(k)[2] = buffer[p+3];
            img.at<Vec3b>(k+1)[0] = buffer[p+2];
            img.at<Vec3b>(k+1)[1] = buffer[p+1];
            img.at<Vec3b>(k+1)[2] = buffer[p+3];
        }

        cvtColor(img,img,CV_YUV2BGR);

        vector<Rect> detections = detector.detect(img);

        Frame frame;
        frame.frameNo = frameNo;
        frame.cameraID = cameraID;
        for(int q=0;q<detections.size();q++)
        {
            BoundingBox bbox;
            bbox.x = detections[q].x;
            bbox.y = detections[q].y;
            bbox.width = detections[q].width;
            bbox.height = detections[q].height;
            frame.detections.push_back(bbox);

            vector<uint16_t> histogram(512);
            for(int r=0;r<512;r++)
            {
                histogram[r] = (uint16_t)detector.histograms[q].at<short>(r);
            }
            frame.histograms.push_back(histogram);
        }
        frameNo++;

        client.send(frame);

#ifdef DISPLAY_MAIN
        imshow(DISPLAY_MAIN,img);
        if(waitKey(1)>0)
            break;
#endif


    }

    /***************************** End looping here *********************/
#ifdef DISPLAY_MAIN
    if(ioctl(fd, VIDIOC_STREAMOFF, &type) < 0)
    {
        perror("Could not end streaming, VIDIOC_STREAMOFF");
        return 1;
    }

    close(fd);

    return 0;
#endif

}

