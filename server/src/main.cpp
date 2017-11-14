//
// Created by dilin on 11/11/17.
//

#include <iostream>
#include "Server.h"

using namespace std;

int main(void)
{

    try
    {
        Server server1(8080);
        Server server2(8081);

        char chCheckForEscKey = 0;
        bool isFrameOne = true;

        while (chCheckForEscKey != 27)
        {

            try
            {
                Frame frame1 = server1.receive();
                cout << "################frame1################" << endl;
                frame1.print();
                isFrameOne = false;
                Frame frame2 = server2.receive();
                cout << "################frame2################" << endl;
                frame2.print();
                isFrameOne = true;
            }
            catch (ReceiveException& e)
            {
                if(e.error!=boost::asio::error::eof)
                    throw e;
                else
                {
                    if(isFrameOne)
                        server1.acceptConnection();
                    else
                        server2.acceptConnection();
                }
                continue;
            }
        }
    }
    catch (std::exception& e)
    {
        cerr << e.what() << endl;
    }

    return(0);
}