//
// Created by dilin on 10/10/17.
//

#ifndef TRACK_GRAPH_H
#define TRACK_GRAPH_H

#include <iostream>
#include <opencv2/opencv.hpp>

struct TrackedPoint
{
public:
    TrackedPoint(cv::MatND histogram,cv::Point2f location);
    cv::MatND histogram;
    cv::Point2f location;
};


struct GraphNode
{
    uint8_t id;
    cv::Point2f location;
    cv::MatND histogram;

    GraphNode(uint8_t id,TrackedPoint trackedPoint);
};

class Graph
{
public:
    Graph(double DIST_TH);
    ~Graph();
    void addNode(uint8_t cameraID,TrackedPoint trackedPoint);
    void addNodes(uint8_t cameraID,std::vector<TrackedPoint> trackedPoints);
    std::vector<cv::Point2f> getUniquePoints();

private:
    std::vector<GraphNode> nodes;
    std::vector<std::vector<double>> cost;
    void calculateCosts();
    double DIST_TH;
};


#endif //TRACK_GRAPH_H
