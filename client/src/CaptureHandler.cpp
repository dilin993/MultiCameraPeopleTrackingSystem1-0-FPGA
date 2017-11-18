//
// Created by dilin on 11/18/17.
//

#include "CaptureHandler.h"

CaptureHandler::CaptureHandler(string device,
                               unsigned int width,
                               unsigned int height) :
        img(height,
            width,
            CV_8UC3)
{
    /******************Initializing V4L2 Driver Starts Here**********************/
    // 1.  Open the device
    fd = open(device.c_str(), O_RDWR);
    if (fd < 0)
    {
        throw CaptureException("Failed to open device, OPEN");
    }

    // 2. Ask the device if it can capture frames
    v4l2_capability capability;
    if (ioctl(fd, VIDIOC_QUERYCAP, &capability) < 0)
    {
        // something went wrong... exit
        throw CaptureException("Failed to get device capabilities, VIDIOC_QUERYCAP");
    }

    // 3. Set Image format
    v4l2_format imageFormat;
    imageFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    imageFormat.fmt.pix.width = width;
    imageFormat.fmt.pix.height = height;
    imageFormat.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    imageFormat.fmt.pix.field = V4L2_FIELD_NONE;
    // tell the device you are using this format
    if (ioctl(fd, VIDIOC_S_FMT, &imageFormat) < 0)
    {
        throw CaptureException("Device could not set format, VIDIOC_S_FMT");
    }

    // 4. Request Buffers from the device
    v4l2_requestbuffers requestBuffer = {0};
    requestBuffer.count = 1; // one request buffer
    requestBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; // request a buffer wich we an use for capturing frames
    requestBuffer.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd, VIDIOC_REQBUFS, &requestBuffer) < 0)
    {
        throw CaptureException("Could not request buffer from device, VIDIOC_REQBUFS");
    }


    // 5. Quety the buffer to get raw data ie. ask for the you requested buffer
    // and allocate memory for it
    v4l2_buffer queryBuffer = {0};
    queryBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    queryBuffer.memory = V4L2_MEMORY_MMAP;
    queryBuffer.index = 0;
    if (ioctl(fd, VIDIOC_QUERYBUF, &queryBuffer) < 0)
    {
        throw CaptureException("Device did not return the buffer information, VIDIOC_QUERYBUF");
    }
    // use a pointer to point to the newly created buffer
    // mmap() will map the memory address of the device to
    // an address in memory
    buffer =
            (unsigned char *) mmap(NULL, queryBuffer.length, PROT_READ | PROT_WRITE,
                                   MAP_SHARED,
                                   fd,
                                   queryBuffer.m.offset);
    memset(buffer, 0, queryBuffer.length);


    // 6. Get a frame
    // Create a new buffer type so the device knows whichbuffer we are talking about
    memset(&bufferinfo, 0, sizeof(bufferinfo));
    bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo.memory = V4L2_MEMORY_MMAP;
    bufferinfo.index = 0;

    // Activate streaming
    type = bufferinfo.type;
    if (ioctl(fd, VIDIOC_STREAMON, &type) < 0)
    {
        throw CaptureException("Could not start streaming, VIDIOC_STREAMON");
    }
    /******************Initializing V4L2 Driver Ends Here**********************/
}

Mat &CaptureHandler::capture()
{
    // Queue the buffer
    if(ioctl(fd, VIDIOC_QBUF, &bufferinfo) < 0)
    {
        throw CaptureException("Could not queue buffer, VIDIOC_QBUF");
    }

    // Dequeue the buffer
    cout << "Waiting to dequeue the buffer..." << endl;
    if(ioctl(fd, VIDIOC_DQBUF, &bufferinfo) < 0)
    {
        throw CaptureException("Could not dequeue the buffer, VIDIOC_DQBUF");
    }
    cout << "Buffer dequeued" << endl;

    for(int k=0,p=0;(k<img.rows*img.cols);k+=2,p+=4)
    {
        img.at<Vec3b>(k)[0] = buffer[p];
        img.at<Vec3b>(k)[1] = buffer[p+1];
        img.at<Vec3b>(k)[2] = buffer[p+3];
        img.at<Vec3b>(k+1)[0] = buffer[p+2];
        img.at<Vec3b>(k+1)[1] = buffer[p+1];
        img.at<Vec3b>(k+1)[2] = buffer[p+3];
    }

    cvtColor(img,img,CV_YUV2BGR);

    return img;
}

CaptureHandler::~CaptureHandler()
{
    if(ioctl(fd, VIDIOC_STREAMOFF, &type) < 0)
    {
        perror("Could not end streaming, VIDIOC_STREAMOFF");
        return;
    }

    close(fd);
}
