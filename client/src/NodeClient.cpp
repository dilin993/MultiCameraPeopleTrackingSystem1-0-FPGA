
#include "NodeClient.h"

NodeClient::NodeClient(boost::asio::io_service &io_service,
                       string ip,
                       unsigned short port,
                       unsigned int width,
                       unsigned int height,
                       string videoSource,
                       uint8_t cameraID):
connection_(io_service),
width(width),
height(height),
cap(videoSource),
cameraID(cameraID),
detector()
{
    frameNo = 0;

    if(!cap.isOpened())  // check if we succeeded
        throw runtime_error("Vide source failed to open.");

    cap.set(CV_CAP_PROP_FRAME_WIDTH,width);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT,height);

    boost::asio::ip::tcp::resolver resolver(io_service);
    tcp::resolver::query query(ip, to_string(port));
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator =
            resolver.resolve(query);

    // Start an asynchronous connect operation.
    boost::asio::async_connect(connection_.socket(), endpoint_iterator,
                               boost::bind(&NodeClient::handle_connect, this,
                                           boost::asio::placeholders::error));
}

void NodeClient::handle_connect(const boost::system::error_code& e)
{
    if (!e)
    {
        capture_frame();
        connection_.async_write(frame,
                                boost::bind(&NodeClient::handle_write,
                                            this,
                                            boost::asio::placeholders::error));
    }
    else
    {
        std::cerr << e.message() << std::endl;
    }
}

void NodeClient::handle_write(const boost::system::error_code &e)
{
    if(!e)
    {
        frame.print();
    }
    else
    {
        std::cerr << e.message() << std::endl;
        if(e==boost::asio::error::connection_refused ||
                e==boost::asio::error::broken_pipe)
            return;
    }

    capture_frame();
    connection_.async_write(frame,
                            boost::bind(&NodeClient::handle_write,
                                        this,
                                        boost::asio::placeholders::error));
}

void NodeClient::capture_frame()
{
    try
    {
        cap >> img;


        //imshow("Image",img);

        vector<Rect> detections = detector.detect(img);

        frame.frameNo = frameNo;
        frame.cameraID = cameraID;
        frame.detections.clear();
        frame.histograms.clear();
        for(int q=0;q<detections.size();q++)
        {
            BoundingBox bbox;
            bbox.x = detections[q].x;
            bbox.y = detections[q].y;
            bbox.width = detections[q].width;
            bbox.height = detections[q].height;
            frame.detections.push_back(bbox);

            vector<uint16_t> histogram(512);
            for(int r=0;r<512;r++)
            {
                histogram[r] = (uint16_t)detector.histograms[q].at<short>(r);
            }
            frame.histograms.push_back(histogram);
        }
        frame.set_now();
        frameNo++;

        //waitKey(1);
    }
    catch(exception &e)
    {
        cerr << "Capture Error: " << e.what() << endl;
    }


}


