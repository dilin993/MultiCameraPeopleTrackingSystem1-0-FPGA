//
// Created by dilin on 10/18/17.
//

#include "Server.h"

Server::Server(boost::asio::io_service &io_service,
               unsigned short port,
               FrameQueue & frames):
        acceptor_(io_service,
                  boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),
                                                 port)),
        frames(frames)
{
    connection_ptr new_conn(new connection(acceptor_.get_io_service()));
    acceptor_.async_accept(new_conn->socket(),
                           boost::bind(&Server::handle_accept, this,
                                       boost::asio::placeholders::error, new_conn));
}

void Server::handle_accept(const boost::system::error_code &e, connection_ptr conn)
{
    if (!e)
    {
        conn->async_read(frame,
                          boost::bind(&Server::handle_read, this,
                                      boost::asio::placeholders::error, conn));
    }

    // Start an accept operation for a new connection.
    connection_ptr new_conn(new connection(acceptor_.get_io_service()));
    acceptor_.async_accept(new_conn->socket(),
                           boost::bind(&Server::handle_accept, this,
                                       boost::asio::placeholders::error, new_conn));
}

void Server::handle_read(const boost::system::error_code &e, connection_ptr conn)
{
    if (!e)
    {
//        boost::chrono::high_resolution_clock::time_point stop =
//                boost::chrono::high_resolution_clock::now();
//        boost::chrono::high_resolution_clock::time_point start = frame.timeStamp;
//        std::cout
//                << "packet delay = "
//                << boost::chrono::duration_cast<boost::chrono::milliseconds>(stop-start).count()
//                << " ms"
//                << std::endl;
        frame.print();
        frames.enqueue(frame);

    }
    else
    {
        // An error occurred.
        std::cerr << e.message() << std::endl;
        if(e==boost::asio::error::eof)
            return;
    }

    conn->async_read(frame,
                     boost::bind(&Server::handle_read, this,
                                 boost::asio::placeholders::error, conn));
}
