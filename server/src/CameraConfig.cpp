//
// Created by fyp-zc702 on 11/15/17.
//

#include "CameraConfig.h"
#include <boost/algorithm/string.hpp>

Point2f CameraConfig::convertToGround(Rect bbox)
{
    cv::Mat p = Mat::ones(3,1,DataType<float>::type);
    p.at<float>(0,0) = (float)(bbox.x + bbox.width/2.0);
    p.at<float>(1,0) = bbox.y + bbox.height;

    p = H * p; // apply homography
    p.at<float>(0,0) /= p.at<float>(2,0);
    p.at<float>(1,0) /= p.at<float>(2,0);

    return Point2f(p.at<float>(0,0),p.at<float>(1,0));
}

void CameraConfig::load(string configFilePath)
{
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(configFilePath.c_str());
    if(result)
    {
        pugi::xml_node config = doc.child("configuration");

        cameraID= (unsigned short)config.child("camera").attribute("id").as_int();

        TRACK_INIT_TH= config.child("track").attribute("TRACK_INIT_TH").as_double();
        REJ_TOL = config.child("track").attribute("REJ_TOL").as_int();

        string hStr = config.child("homography").attribute("mat").as_string();
        hStr.erase(std::remove(hStr.begin(),hStr.end(),' '),hStr.end());
        std::vector<std::string> strs;
        boost::split(strs, hStr, boost::is_any_of(","));
        H = Mat(3, 3, CV_32FC1);

        for(int i=0;i<3;i++)
        {
            for(int j=0;j<3;j++)
            {
                H.at<float>(i,j) = stof(strs[i*3+j]);
            }
        }
    }
    else
    {
        throw FailedToLoadConfiguration(configFilePath);
    }
    isLoaded = true;
}

CameraConfig::CameraConfig()
{
    isLoaded = false;
}

unsigned short CameraConfig::getCameraID() const
{
    return cameraID;
}


const Mat &CameraConfig::getH() const
{
    return H;
}

int CameraConfig::getREJ_TOL() const
{
    return REJ_TOL;
}

double CameraConfig::getTRACK_INIT_TH() const
{
    return TRACK_INIT_TH;
}

Point2f CameraConfig::convertToGround(Point2f pos)
{
    cv::Mat p = Mat::ones(3,1,DataType<float>::type);
    p.at<float>(0,0) = pos.x;
    p.at<float>(1,0) = pos.y;

    p = H * p; // apply homography
    p.at<float>(0,0) /= p.at<float>(2,0);
    p.at<float>(1,0) /= p.at<float>(2,0);

    return Point2f(p.at<float>(0,0),p.at<float>(1,0));
}
