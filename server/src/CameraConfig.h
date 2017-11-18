//
// Created by fyp-zc702 on 11/15/17.
//

#ifndef SERVER_CAMERACONFIG_H
#define SERVER_CAMERACONFIG_H

#include <opencv2/opencv.hpp>
#include "pugixml.hpp"

using namespace std;
using namespace cv;

struct FailedToLoadConfiguration : public exception
{
    FailedToLoadConfiguration(string configPath)
    {
        this->configPath = configPath;
    }

    const char * what () const throw ()
    {
        string msg="Failed to load the configuration file \"";
        msg += configPath;
        msg += "\".";
        return msg.c_str();
    }

    string configPath;
};

class CameraConfig
{
public:
    CameraConfig();
    Point2f convertToGround(Rect bbox);
    Point2f convertToGround(Point2f pos);
    void load(string configFilePath);
    bool isLoaded;
    unsigned short getCameraID() const;
    const Mat &getH() const;
    int getREJ_TOL() const;
    double getTRACK_INIT_TH() const;

private:
    unsigned short cameraID;
    Mat H;
    int REJ_TOL;
    double TRACK_INIT_TH;
};


#endif //SERVER_CAMERACONFIG_H
