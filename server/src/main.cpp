//
// Created by dilin on 11/11/17.
//

#include <iostream>
#include "Server.h"
#include "pugixml.hpp"
#include "CameraConfig.h"
#include "DataAssociation.h"
#include "graph.h"

#define DISPLAY_FLAG 1

using namespace std;

int main(int argc, const char * argv[])
{
    try
    {

        unsigned short num_nodes = 0;
        unsigned short WIDTH = 0;
        unsigned short HEIGHT = 0;
        vector<CameraConfig> cameraConfigs;
        double DIST_TH;

        if(argc>1)
        {
            pugi::xml_document doc;
            pugi::xml_parse_result result = doc.load_file(argv[1]);
            if(result)
            {
                pugi::xml_node config = doc.child("configuration");
                num_nodes = (unsigned short)config.child("main").attribute("num_nodes").as_int();
                WIDTH = (unsigned short)config.child("main").attribute("width").as_int();
                HEIGHT = (unsigned short)config.child("main").attribute("height").as_int();
                DIST_TH = config.child("main").attribute("dist_th").as_double();

                for(unsigned short n=0;n<num_nodes;n++)
                {
                    string camName = "camera" + to_string(n);
                    string configPath = config.child(camName.c_str()).attribute("config").as_string();

                    cameraConfigs.push_back(CameraConfig());

                    cameraConfigs[n].load(configPath);
                }
            }
        }

        vector<Server> servers(num_nodes);
        vector<Frame> frames(num_nodes);
        vector<string> windowNames(num_nodes);
        Graph graph(DIST_TH);

#ifdef DISPLAY_FLAG
        vector<Mat> imgs(num_nodes);
#endif
        vector<DataAssociation> associations(num_nodes);
        for(unsigned short n=0;n<num_nodes;n++)
        {
            servers[n] = Server(cameraConfigs[n].getPort());
            cout << "Waiting for camera" << n << " to connect..." << endl;
            servers[n].acceptConnection();
            cout << "Camera" << n << " acquired connection!" << endl;
            windowNames[n] = "camera" + to_string(cameraConfigs[n].getCameraID()) + " tracks";
            associations[n] = DataAssociation(cameraConfigs[n].getTRACK_INIT_TH(),
                                              cameraConfigs[n].getREJ_TOL(),
                                              WIDTH,HEIGHT);
#ifdef DISPLAY_FLAG
            namedWindow(windowNames[n]);
#endif
        }

        char chCheckForEscKey = 0;
        bool isFrameOne = true;

        while (chCheckForEscKey != 27)
        {
            for(unsigned short n=0;n<num_nodes;n++)
            {
                frames[n] = servers[n].receive();
                frames[n].print();


                // calculate ground planed coordinates and normalized histograms
                vector<Point2f> detections;
                vector<MatND> histograms;
                int k=0;
                for (auto const bbox : frames[n].detections)
                {
                    Rect detection(bbox.x, bbox.y, bbox.width, bbox.height);

                    // track bottom middle point
                    detections.push_back(Point(bbox.x+bbox.width/2,bbox.y+bbox.height));

                    int sizes[3] = {8,8,8};
                    MatND histogram(3,sizes,CV_32F);
                    for(int i=0;i<512;i++)
                    {
                        histogram.at<float>(i) = (float)frames[n].histograms[k][i];
                    }
                    MatND normalizedHistogram;
                    normalize( histogram, normalizedHistogram, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());
                    histograms.push_back(histogram);
                    k++;
                }

                // get tracks
                associations[n].assignTracks(detections,histograms);

                vector<ParticleFilterTracker> &tracks = associations[n].getTracks();


#ifdef DISPLAY_FLAG
                // draw detections
                imgs[n] = Mat::zeros(HEIGHT,WIDTH,CV_8UC3);

                vector<TrackedPoint> groundPlanePoints;

                for(int i=0;i<tracks.size();i++)
                {
                    Point2f pos = tracks[i].getPos();
                    pos = cameraConfigs[n].convertToGround(pos);
                    drawMarker(imgs[n], pos,
                               tracks[i].color,
                               MarkerTypes::MARKER_CROSS, 30, 10);
                    groundPlanePoints.push_back(TrackedPoint(tracks[i].histogram,
                                                             pos));
                }



                imshow(windowNames[n],imgs[n]);
#endif


                // do correspondence estimation
                graph.addNodes((uint8_t)n,groundPlanePoints);

                vector<Point2f> uniquePoints = graph.getUniquePoints();
            }
#ifdef DISPLAY_FLAG
            chCheckForEscKey = waitKey(1);
#endif
        }
    }
    catch (std::exception& e)
    {
        cerr << e.what() << endl;
    }

    return(0);
}