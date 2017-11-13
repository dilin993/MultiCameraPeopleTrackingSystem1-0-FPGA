//
// Created by dilin on 8/11/17.
//

#ifndef PEOPLETECKERV2_DETECTOR_H
#define PEOPLETECKERV2_DETECTOR_H

#include <iostream>
#include <opencv2/opencv.hpp>

class Detector
{
public:
    virtual std::vector<cv::Rect> detect(cv::Mat &img) = 0;
    std::vector<cv::Mat> histograms;
};


#endif //PEOPLETECKERV2_DETECTOR_H
