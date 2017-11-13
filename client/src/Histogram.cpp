//
// Created by dilin on 11/6/17.
//

#include "Histogram.h"


void Histogram::calcHist(Mat &img, Mat &mask, Rect &region, Mat &histogram)
{
    histogram = Mat::zeros(512,1,CV_16UC1);
    int index;

    for(int i=0;i<img.rows;i++)
    {
        for(int j=0;j<img.cols;j++)
        {
            uchar b = img.data[img.channels()*(img.cols*i + j) + 0];
            uchar g = img.data[img.channels()*(img.cols*i + j) + 1];
            uchar r = img.data[img.channels()*(img.cols*i + j) + 2];
            uchar l = mask.data[mask.cols*i+j];
            if (region.y <= i &&
                region.x <= j &&
                (region.y + region.height) >= i &&
                (region.x + region.width) >= j && l>0)
            {
                index = 64 * (r >> 5) + 8 * (g >> 5) + (b >> 5);
                histogram.at<short>(index) += 1;
            }
        }
    }
}

void Histogram::normalizeHist(Mat &histogram, Mat &normalized)
{
    int MAX = 0;
    int MIN = INT_MAX;

    for(int i=0;i<512;i++)
    {
        if(histogram.at<int>(i)>MAX)
            MAX = histogram.at<int>(0);
        if(histogram.at<int>(i)<MIN)
            MIN = histogram.at<int>(0);
    }

    MAX -= MIN;
    normalized =  histogram = Mat::zeros(512,1,CV_32FC1);
    for(int i=0;i<512;i++)
    {
        normalized.at<float>(i) = (float)(histogram.at<int>(i)-MIN)/(float)(MAX+1e-5);
    }
}

float Histogram::compareHist(Mat &histA, Mat &histB)
{
    float uA=0,uB=0,varA=0,varB=0,covAB=0;

    for(int i=0;i<512;i++)
    {
        uA += histA.at<float>(i);
        uB += histB.at<float>(i);
    }

    uA /= 512;
    uB /= 512;

    for(int i=0;i<512;i++)
    {
        varA += pow(histA.at<float>(i)-uA,2.0);
        varB += pow(histB.at<float>(i)-uB,2.0);
        covAB += (histA.at<float>(i)-uA)*(histB.at<float>(i)-uB);
    }
    return covAB/sqrt(varA*varB);
}
