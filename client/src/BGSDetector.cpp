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
    Mat gray(240,320,CV_8UC1);
    cvtColor(img,gray,COLOR_BGR2GRAY);
     for(int k=0;(k<img.rows*img.cols);k+=1)
        {
            //ybuffer[k] = (img.at<Vec3b>(k)[0] + img.at<Vec3b>(k)[1] + img.at<Vec3b>(k)[3])/3;  
            ybuffer[k] = gray.at<unsigned char>(k);
            
        }

    memcpy(src,ybuffer,sizeof(uint8_t)*WIDTH*HEIGHT);
    if (isFirst){
        backsub_config(true);
        isFirst = false;
    }
    else{
        backsub_config(false);
    }

    XBacksub_Start(&backsub);

    while(!XBacksub_IsDone(&backsub));

    for (int i =0;i<320*240;i++){

        printf("%d\n",ybuffer[i]);
        mask.at<unsigned char>(i) = dst[i];
    }

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
doGamaCorrection(doGammaCorrection), mask(240,320,CV_8UC1)
{
    frameCount = 0;
    // if(method==BGS_GMM)
    //     pMOG2 = cv::bgsegm::createBackgroundSubtractorMOG();

    fdIP = open ("/dev/mem", O_RDWR);
    if (fdIP < 1) {
        perror("Error");
        return;
    }

    ybuffer = new uint8_t[WIDTH*HEIGHT];

    src = (uint8_t*)mmap(NULL, DDR_RANGE,PROT_READ|PROT_WRITE, MAP_SHARED, fdIP, TX_BASE_ADDR); 
    dst = (uint8_t*)mmap(NULL, DDR_RANGE,PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED, fdIP, RX_BASE_ADDR); 

    if(backsub_init(&backsub)==0) {
        printf("IP Core Initialized\n");
    }
}

BGSDetector::~BGSDetector(){
    
    backsub_rel(&backsub);

    munmap((void*)src, DDR_RANGE);
    munmap((void*)dst, DDR_RANGE);
    delete[] ybuffer;
    close(fdIP);
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

int BGSDetector::backsub_init(XBacksub * backsub_ptr){

    // fd = open ("/dev/mem", O_RDWR);
    // if (fd < 1) {
    //     printf("Dev mem open failed\n");
    //     return -1;
    // }

    backsub_ptr->Crtl_bus_BaseAddress = (u32)mmap(NULL, AXILITE_RANGE, PROT_READ|PROT_WRITE, MAP_SHARED, fdIP, CRTL_BUS_BASEADDR);
    backsub_ptr->Axilites_BaseAddress = (u32)mmap(NULL, AXILITE_RANGE, PROT_READ|PROT_WRITE, MAP_SHARED, fdIP, AXILITES_BASEADDR);
    backsub_ptr->IsReady = XIL_COMPONENT_IS_READY;
    return 0;
}

void BGSDetector::backsub_rel(XBacksub * backsub_ptr){
    munmap((void*)backsub_ptr->Crtl_bus_BaseAddress, AXILITE_RANGE);
    munmap((void*)backsub_ptr->Axilites_BaseAddress, AXILITE_RANGE);
}

void BGSDetector::backsub_config(bool ini) {

    XBacksub_Set_frame_in(&backsub,(u32)TX_BASE_PTR);
    XBacksub_Set_frame_out(&backsub,(u32)RX_BASE_PTR);
    XBacksub_Set_init(&backsub, ini);
}

void BGSDetector::print_config() {
    printf("Is Ready = %d \n", XBacksub_IsReady(&backsub));
    printf("Frame in = %X \n", XBacksub_Get_frame_in(&backsub));
    printf("Frame out = %X \n", XBacksub_Get_frame_out(&backsub));
    printf("Init = %d \n", XBacksub_Get_init(&backsub));
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
