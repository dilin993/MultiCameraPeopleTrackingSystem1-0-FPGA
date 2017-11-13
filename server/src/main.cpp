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
        Server server(8080);

        char chCheckForEscKey = 0;

        while (chCheckForEscKey != 27)
        {

            try
            {
                Frame frame = server.receive();
                frame.print();
            }
            catch (ReceiveException& e)
            {
                if(e.error!=boost::asio::error::eof)
                    throw e;
                else
                    server.acceptConnection();
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