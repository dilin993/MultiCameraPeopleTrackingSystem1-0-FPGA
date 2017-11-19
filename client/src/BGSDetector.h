//
// Created by dilin on 10/23/17.
//
#include <opencv2/opencv.hpp>
#include "headers/xparameters.h"
#include <chrono>
#include <string.h>
#include <fstream>
#include <string>
// #include <termios.h>
#include "curses.h"
#include "xbacksub.h"
#include "headers/xparameters.h"

#ifndef TRACK_BGSDETECTOR_H
#define TRACK_BGSDETECTOR_H

//#include <opencv2/bgsegm.hpp>
#include "Detector.h"
#include "Histogram.h"

#define BGS_DEBUG_MODE

#define TX_BASE_ADDR 0x01000000
#define DDR_RANGE 0x01000000
#define RX_BASE_ADDR 0x02000000

#define AXILITES_BASEADDR 0x43C00000
#define CRTL_BUS_BASEADDR 0x43C10000
#define AXILITE_RANGE 0xFFFF

#define WIDTH 320
#define HEIGHT 240
#define BUF_SIZE(W,H) W*H
#define N BUF_SIZE(WIDTH,HEIGHT)


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
    ~BGSDetector();
    std::vector<cv::Rect> detect(cv::Mat &img);

private:
    void backgroundSubstraction(cv::Mat &frame0, cv::Mat &frame1, cv::Mat &frame2
            , cv::Mat &bgModel, cv::Mat &mask, double TH=15);
    cv::Mat mask;
    Mat gray;
    uint8_t frameCount;
    double TH;
    void GammaCorrection(cv::Mat& src, cv::Mat& dst, float fGamma);

    //Hardware 
    int backsub_init(XBacksub * backsub_ptr);
    void backsub_rel(XBacksub * backsub_ptr);
    void backsub_config(bool ini);
    void print_config();
    void signalHandler( int signum );

    int method;
    bool doGamaCorrection;

    uint8_t * TX_BASE_PTR = (uint8_t *) (TX_BASE_ADDR);
    uint8_t * RX_BASE_PTR = (uint8_t *) (RX_BASE_ADDR);

    XBacksub backsub;
    uint8_t * src;
    uint8_t * dst;
    //uint8_t * ybuffer;
    int fdIP;
    bool isFirst =true;
};


#endif //TRACK_BGSDETECTOR_H
