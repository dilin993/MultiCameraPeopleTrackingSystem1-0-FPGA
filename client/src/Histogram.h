//
// Created by dilin on 11/6/17.
//

#ifndef TRACK_HISTOGRAM_H
#define TRACK_HISTOGRAM_H

#include <iostream>
#include <exception>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

#define NBINS 8





class Histogram
{
public:
    static void calcHist(Mat &img, Mat &mask, Rect &region, Mat &histogram);
    static void normalizeHist(Mat &histogram,Mat &normalized);
    float compareHist(Mat &histA, Mat &histB);
};


#endif //TRACK_HISTOGRAM_H
