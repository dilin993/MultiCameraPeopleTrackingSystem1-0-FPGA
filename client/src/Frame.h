//
// Created by dilin on 11/11/17.
//

#ifndef CLIENT_FRAME_H
#define CLIENT_FRAME_H

#include <boost/serialization/vector.hpp>
#include <iostream>
#include <opencv2/opencv.hpp>


using namespace std;
using namespace cv;


struct BoundingBox
{
    int x;
    int y;
    int width;
    int height;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & x;
        ar & y;
        ar & width;
        ar & height;
    }
};

class Frame
{
public:
    vector<BoundingBox> detections;
    vector<vector<uint16_t >> histograms;
    uint16_t frameNo;
    uint8_t cameraID;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & detections;
        ar & histograms;
        ar & frameNo;
        ar & cameraID;
    }

    void print()
    {
        cout << "Frame\n{";
        cout << "\tCameraID: " << (int)cameraID << "\n";
        cout << "\tFrameNO: " << frameNo << "\n";
        cout << "\tdetections=[";
        for(auto const detection:detections)
        {
            cout << "[";
            cout << detection.x << ",";
            cout << detection.y << ",";
            cout << detection.width << ",";
            cout << detection.height << "],";
        }
        cout << "]\n}\n";
    }

};


#endif //CLIENT_FRAME_H
