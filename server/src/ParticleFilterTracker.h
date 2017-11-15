//
// Created by dilin on 11/3/17.
// Based on https://github.com/mabdh/mot-particle-filter
//

#ifndef TRACK_PARTICLEFILTERTRACKER_H
#define TRACK_PARTICLEFILTERTRACKER_H

#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <opencv2/opencv.hpp>

#define NUM_STATES 4
#define MAX_PAST 10

using namespace std;
using namespace cv;

struct State
{
public:
    State();
    State(double x,double y,double vx,double vy);
    void set(double x,double y,double vx,double vy);
    double x;
    double y;
    double vx;
    double vy;

};

class ParticleFilterTracker
{
public:
    explicit ParticleFilterTracker(Point2f detection,
                                   MatND histogram,
                                   double sigma_propagate[],
                          unsigned int noParticles=200,
                                   double alpha=0.4,
                                   double var_m=0.5,
                                   double hc=0.5);
    void assignDetection(Point2f detection, MatND histogram);
    void update();
    Point2f getPos();
    State getState();
    MatND histogram;
    long age=0;
    long totalVisibleCount=0;
    long consectiveInvisibleCount=0;
    void updateAssociation(bool detectionAssociated);
    cv::Scalar color;


protected:
    vector<State> particles;
    vector<State> particles_new;
    queue<State> pastStates;
    unsigned int noParticles;
    vector<double> weights;
    double sigma_propagate[NUM_STATES]={0.0, 0.0, 0.0, 0.0};
    double alpha; // color model importance factor
    double var_m; // variance of detection-tracker error
    double hc; // histogram accumulation term
    void predict();
    void measurement(Point2f detection, MatND histogram);
    void systematicResampling(vector<double>& w, vector<int>& indx);
    void initialize(Point2f detection);
    void updateState(vector<double> normalized_weights);
    bool prevDetectionAssociated= false;
    static RNG rng;

};


#endif //TRACK_PARTICLEFILTERTRACKER_H
