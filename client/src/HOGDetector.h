//
// Created by dilin on 8/11/17.
//

#ifndef PEOPLETECKERV2_HOGDETECTOR_H
#define PEOPLETECKERV2_HOGDETECTOR_H
#include <iostream>
#include <opencv2/opencv.hpp>
#include "Detector.h"
#include "Histogram.h"

class HOGDetector : public  Detector
{
public:
    HOGDetector();
    std::vector<cv::Rect> detect(cv::Mat &img);
private:
    cv::HOGDescriptor hog;
};


#endif //PEOPLETECKERV2_HOGDETECTOR_H
