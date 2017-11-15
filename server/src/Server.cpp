//
// Created by dilin on 10/18/17.
//

#include "Server.h"

Server::Server(int port) :
acceptor(io_service),
socket(acceptor.get_io_service())
{
    this->port = port;
}

void Server::acceptConnection()
{
    if(socket.is_open())
        return;
    //socket.close();
    tcp::endpoint endpoint(tcp::v4(), port);
    acceptor.open(endpoint.protocol());
    acceptor.bind(endpoint);
    acceptor.listen(1);
    acceptor.accept(socket);
}

Frame Server::receive()
{
    boost::system::error_code error;
    uint32_t net_len;
    size_t len = boost::asio::read(socket,
            boost::asio::buffer( reinterpret_cast<char*>(&net_len), 4),
            error );
    if(error || len!=4)
    {
        throw ReceiveException(len,error);
    }

    char *inbound_data_ = new char[net_len];

    size_t msg_len = boost::asio::read(socket,
                                       boost::asio::buffer(inbound_data_,net_len),
                                       error);

    if(error || net_len!=msg_len)
    {
        throw ReceiveException(msg_len,error);
    }

    std::string archive_data(&inbound_data_[0], net_len);
    std::istringstream archive_stream(archive_data);
    boost::archive::text_iarchive archive(archive_stream);
    Frame t;
    archive >> t;

    return t;
}

Server::Server() :
        acceptor(io_service),
        socket(acceptor.get_io_service())
{
    port = 0;
}

Server &Server::operator=(const Server &other)
{
    this->port = other.port;
    return *this;
}
