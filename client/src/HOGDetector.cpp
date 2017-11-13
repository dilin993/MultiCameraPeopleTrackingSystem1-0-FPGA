//
// Created by dilin on 8/11/17.
//

#include "HOGDetector.h"

HOGDetector::HOGDetector()
{
    hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
}

std::vector<cv::Rect> HOGDetector::detect(cv::Mat &img)
{
    std::vector<cv::Rect> found,detections;
    hog.detectMultiScale(img, found, 0, cv::Size(8,8), cv::Size(32,32), 1.05, 2);
    size_t i, j;
    histograms.clear();
    for (i=0; i<found.size(); i++)
    {
        cv::Rect r = found[i];
        for (j=0; j<found.size(); j++)
            if (j!=i && (r & found[j])==r)
                break;
        if (j==found.size())
        {
            r.x += cvRound(r.width*0.1);
            r.width = cvRound(r.width*0.8);
            r.y += cvRound(r.height*0.07);
            r.height = cvRound(r.height*0.8);
            detections.push_back(r);
            Mat histogram;
            Mat shape = Mat::ones(img.rows,img.cols,CV_8UC1);
            Histogram::calcHist(img,shape,r,histogram);
            histograms.push_back(histogram);
        }

    }
    return detections;
}