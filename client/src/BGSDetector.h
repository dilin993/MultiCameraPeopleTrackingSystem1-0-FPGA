//
// Created by dilin on 10/23/17.
//

#ifndef TRACK_BGSDETECTOR_H
#define TRACK_BGSDETECTOR_H
#include <opencv2/opencv.hpp>
#include <opencv2/bgsegm.hpp>
#include "Detector.h"
#include "Histogram.h"

#define BGS_DEBUG_MODE

class Blob
{
public:
    // member variables ///////////////////////////////////////////////////////////////////////////
    Blob(std::vector<cv::Point> _contour);
    std::vector<cv::Point> currentContour;

    cv::Rect currentBoundingRect;

    std::vector<cv::Point> centerPositions;

    double dblCurrentDiagonalSize;
    double dblCurrentAspectRatio;

    bool blnCurrentMatchFoundOrNewBlob;

    bool blnStillBeingTracked;

    int intNumOfConsecutiveFramesWithoutAMatch;

    cv::Point predictedNextPosition;
};

enum
{
    BGS_MOVING_AVERAGE=1,
    BGS_GMM=2
};

class BGSDetector : public Detector
{
public:
    explicit BGSDetector(double TH=15,
                         int method=BGS_GMM,
                         bool doGammaCorrection=false);
    std::vector<cv::Rect> detect(cv::Mat &img);

private:
    void backgroundSubstraction(cv::Mat &frame0, cv::Mat &frame1, cv::Mat &frame2
            , cv::Mat &bgModel, cv::Mat &mask, double TH=15);
    cv::Mat frames[3];
    cv::Mat bgModel;
    cv::Mat mask;
    uint8_t frameCount;
    double TH;
    cv::Ptr<cv::BackgroundSubtractor> pMOG2; //MOG2 Background subtractor
    void GammaCorrection(cv::Mat& src, cv::Mat& dst, float fGamma);
    int method;
    bool doGamaCorrection;
};


#endif //TRACK_BGSDETECTOR_H
