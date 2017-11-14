//
// Created by dilin on 10/18/17.
//

#include "NodeClient.h"

NodeClient::NodeClient(string ip,unsigned short port):
resolver(io_service),
socket(io_service)
{
    this->ip = ip;
    this->port = port;
}

void NodeClient::connect()
{
    tcp::resolver::query query(ip, to_string(port));
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    tcp::resolver::iterator end;
    boost::system::error_code error = boost::asio::error::host_not_found;

//    while (error && endpoint_iterator != end)
//    {
//        socket.close();
//        socket.connect(*endpoint_iterator++, error);
//    }

    boost::asio::connect(socket, endpoint_iterator, error);

    if (error)
        throw boost::system::system_error(error);
}

void NodeClient::send(Frame frame)
{
    std::ostringstream archive_stream;
    boost::archive::text_oarchive archive(archive_stream);
    archive << frame;
    std::string outbound_data_ = archive_stream.str();

    uint32_t net_len = (uint32_t)outbound_data_.length();
//    uint32_t net_len = htonl( len );
    unsigned char net_char_len[4];
    net_char_len[0] = (unsigned char)(net_len >> 0);
    net_char_len[1] = (unsigned char)(net_len >> 8);
    net_char_len[2] = (unsigned char)(net_len >> 16);
    net_char_len[3] = (unsigned char)(net_len >> 24);

    boost::system::error_code ignored_error;

    // write msg length
    boost::asio::write(socket, boost::asio::buffer( net_char_len, 4));
    // write the msg
    boost::asio::write(socket, boost::asio::buffer(outbound_data_),
                       boost::asio::transfer_all(), ignored_error);
}

string NodeClient::getIP()
{
    return ip;
}

unsigned short NodeClient::getPort()
{
    return port;
}
