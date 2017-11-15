//
// Created by dilin on 8/13/17.
//

#ifndef PEOPLETRACKINGSYSTEM_DATAASSOCIATION_H
#define PEOPLETRACKINGSYSTEM_DATAASSOCIATION_H

#define VEL_TH 60
#include "ParticleFilterTracker.h"
#include<iostream>
#include <opencv2/opencv.hpp>
#include "hungarian.h"


using namespace std;
using  namespace cv;



class DataAssociation
{
public:
    DataAssociation(double TRACK_INIT_TH,int REJ_TOL,int WIDTH,int HEIGHT);
    DataAssociation();
    vector<ParticleFilterTracker> & getTracks();
    void assignTracks(vector<Point2f> detections,vector<MatND> histograms);
    ~DataAssociation();
    void setSize(int width,int height);
private:
    vector<ParticleFilterTracker> tracks;
    double TRACK_INIT_TH;
    int REJ_TOL;
    int WIDTH;
    int HEIGHT;
    double averageError(Point2f a, Point2f b);
    double averageError(Point2f a, Point2f b,MatND hisA,MatND histB);
    double sigma_propagate[NUM_STATES]={10.0, 10.0, 1.0, 1.0};
    double var_m=0.5;
    double alpha = 0.4;
};


#endif //PEOPLETRACKINGSYSTEM_DATAASSOCIATION_H
