#include "HttpsClient.h"
#include <iostream>
#include <fstream>

HttpsClient::HttpsClient() {
	ctx = new boost::asio::ssl::context(boost::asio::ssl::context::tlsv12_client);
	ctx->add_certificate_authority(boost::asio::const_buffer(&ca.at(0), ca.size()));
	
	io_service = new boost::asio::io_service();  
}

HttpsClient::~HttpsClient() { 
    io_service->stop();
}

void HttpsClient::open(std::string host, std::string path) {
	boost::asio::ip::tcp::resolver::query query(host, "https");
	boost::asio::ip::tcp::resolver resolver(*io_service);
    HttpsClient::host = host;
    HttpsClient::path = path;

    status_code = 0;
    status_message.clear();
    headers.clear();
    body.str("");

    boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket(*io_service, *ctx);
    boost::system::error_code err;

    boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query, err);

    if (err) {
        status_message = "Error resolve: " + err.message();
        status_code = 700;
        return;
    }

    socket.set_verify_mode(boost::asio::ssl::verify_peer);
    boost::asio::connect(socket.lowest_layer(), endpoint_iterator, err);

    if (err) {
        status_code = 700;
        status_message = "Connect failed: " + err.message();
        return;
    }


    socket.handshake(boost::asio::ssl::stream_base::client, err);

    if (err) {
        status_code = 700;
        status_message = "Handshake failed: " + err.message();
        return;
    }

    boost::asio::streambuf request;
    std::ostream request_stream(&request);
    request_stream << "GET " << HttpsClient::path << " HTTP/1.0\r\n";
    request_stream << "Host: " << HttpsClient::host << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";

    boost::asio::write(socket, request, err);

    if (err) {
        status_code = 700;
        status_message = "Error write req: " + err.message();
        return ;
    }

    boost::asio::streambuf response;
    boost::asio::read_until(socket, response, "\r\n", err);

    if (err) {
        status_code = 700;
        status_message = "Error read status line: " + err.message();
        return;
    }

    std::istream response_stream(&response);
    std::string http_version;
    response_stream >> http_version;

    if (!response_stream || http_version.substr(0, 5) != "HTTP/")
    {
        status_code = 700;
        status_message = "Invalid response";
        return;
    }

    response_stream >> status_code;
    std::getline(response_stream, status_message);

    boost::asio::read_until(socket, response, "\r\n\r\n", err);
    if (err) {
        status_code = 700;
        status_message = "Error read headres: " + err.message();
        return;
    }

    std::string header, key, value;
    std::size_t pos;

    while (std::getline(response_stream, header) && header != "\r") {
        pos = header.find(':');
        key = header.substr(0, pos);
        value = header.substr(pos + 1);
        boost::trim(key);
        boost::trim(value);

        headers.insert(std::make_pair(key, value));
    }

    boost::asio::read(socket, response, err);

    do
    {
        body << &response;
        boost::asio::read(socket, response, err);
    } while (err != boost::asio::error::eof);

    socket.shutdown();
}
