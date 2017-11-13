//
// Created by dilin on 10/23/17.
//

#include "BGSDetector.h"

std::vector<cv::Rect> BGSDetector::detect(cv::Mat &img)
{
    std::vector<cv::Rect> found,detections;

    if(doGamaCorrection)
        GammaCorrection(img,img,2.0);

    histograms.clear();

    if(method==BGS_GMM)
    {
        pMOG2->apply(img,mask);
    }
    else
    {
        for(int i=2;i>0;i--)
        {
            frames[i] = frames[i-1].clone();
        }
        frames[0] = img.clone();

        if(frameCount<3)
        {
            frameCount++;
            return detections;
        }

        backgroundSubstraction(frames[0],frames[1],frames[2],
                               bgModel,mask,TH);
    }

#ifdef BGS_DEBUG_MODE
    cv::imshow("BackSub",mask);
#endif


    cv::Mat structuringElement = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7, 7));


    cv::Mat maskPost;
    cv::dilate(mask,maskPost, structuringElement);
    cv::dilate(maskPost, maskPost, structuringElement);
    cv::erode(maskPost, maskPost, structuringElement);



    std::vector<std::vector<cv::Point> > contours;

    cv::findContours(maskPost, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    cv::Mat shape(mask.rows,mask.cols,CV_8UC1,Scalar(0));



    for(int i = 0; i < contours.size(); i++)
    {
        cv::Scalar color(255);
        drawContours(shape, contours, i, color, CV_FILLED);
    }

#ifdef BGS_DEBUG_MODE
    cv::imshow("Shape",shape);
#endif

    std::vector<std::vector<cv::Point> > convexHulls(contours.size());

    for (unsigned int i = 0; i < contours.size(); i++)
    {
        cv::convexHull(contours[i], convexHulls[i]);
    }


    for (auto &convexHull : convexHulls) {
        Blob possibleBlob(convexHull);

        if (possibleBlob.currentBoundingRect.area() > 5000 &&
            possibleBlob.dblCurrentAspectRatio >= 0.2 &&
            possibleBlob.dblCurrentAspectRatio <= 1.25 &&
            possibleBlob.currentBoundingRect.width > 30 &&
            possibleBlob.currentBoundingRect.height > 40 &&
            possibleBlob.dblCurrentDiagonalSize > 30.0 &&
            (cv::contourArea(possibleBlob.currentContour) /
             (double)possibleBlob.currentBoundingRect.area()) > 0.5)
        {
            found.push_back(possibleBlob.currentBoundingRect);
        }
    }

    size_t i, j;

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
            Histogram::calcHist(img,shape,r,histogram);
            histograms.push_back(histogram);
        }

    }

    return detections;
}


void BGSDetector::backgroundSubstraction(cv::Mat &frame0, cv::Mat &frame1, cv::Mat &frame2
        , cv::Mat &bgModel, cv::Mat &mask, double TH)
{
    cv::Mat frame0g,frame1g,frame2g;

   // convert frames to gray
    cvtColor(frame0,frame0g,cv::COLOR_BGR2GRAY);
    cvtColor(frame1,frame1g,cv::COLOR_BGR2GRAY);
    cvtColor(frame2,frame2g,cv::COLOR_BGR2GRAY);

    cv::GaussianBlur(frame0g,frame0g,cv::Size(5, 5), 0);
    cv::GaussianBlur(frame1g,frame1g,cv::Size(5, 5), 0);
    cv::GaussianBlur(frame2g,frame2g,cv::Size(5, 5), 0);

    bgModel = 0.1*frame0g + 0.2*frame1g + 0.7*frame2g;

    cv::Mat diff;
    absdiff(frame0g,bgModel,diff);
    threshold(diff,mask,TH,255,cv::THRESH_BINARY);
}

BGSDetector::BGSDetector(double TH,
                         int method,
                         bool doGammaCorrection) :
TH(TH),
method(method),
doGamaCorrection(doGammaCorrection)
{
    frameCount = 0;
    if(method==BGS_GMM)
        pMOG2 = cv::bgsegm::createBackgroundSubtractorMOG();
}

void BGSDetector::GammaCorrection(cv::Mat &src, cv::Mat &dst, float fGamma)
{
    CV_Assert(src.data);

    // accept only char type matrices
    CV_Assert(src.depth() != sizeof(uchar));

    // build look up table
    unsigned char lut[256];
    for (int i = 0; i < 256; i++)
    {
        lut[i] = cv::saturate_cast<uchar>(pow((float)(i / 255.0), fGamma) * 255.0f);
    }

    dst = src.clone();
    const int channels = dst.channels();
    switch (channels)
    {
        case 1:
        {

            cv::MatIterator_<uchar> it, end;
            for (it = dst.begin<uchar>(), end = dst.end<uchar>(); it != end; it++)
                //*it = pow((float)(((*it))/255.0), fGamma) * 255.0;
                *it = lut[(*it)];

            break;
        }
        case 3:
        {

            cv::MatIterator_<cv::Vec3b> it, end;
            for (it = dst.begin<cv::Vec3b>(), end = dst.end<cv::Vec3b>(); it != end; it++)
            {

                (*it)[0] = lut[((*it)[0])];
                (*it)[1] = lut[((*it)[1])];
                (*it)[2] = lut[((*it)[2])];
            }

            break;

        }
    }

}

Blob::Blob(std::vector<cv::Point> _contour)
{
    currentContour = _contour;

    currentBoundingRect = cv::boundingRect(currentContour);

    cv::Point currentCenter;

    currentCenter.x = (currentBoundingRect.x + currentBoundingRect.x + currentBoundingRect.width) / 2;
    currentCenter.y = (currentBoundingRect.y + currentBoundingRect.y + currentBoundingRect.height) / 2;

    centerPositions.push_back(currentCenter);

    dblCurrentDiagonalSize = sqrt(pow(currentBoundingRect.width, 2) + pow(currentBoundingRect.height, 2));

    dblCurrentAspectRatio = (float)currentBoundingRect.width / (float)currentBoundingRect.height;

    blnStillBeingTracked = true;
    blnCurrentMatchFoundOrNewBlob = true;

    intNumOfConsecutiveFramesWithoutAMatch = 0;
}
