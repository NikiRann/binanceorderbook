#pragma once

#include <boost/asio/strand.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

#include <boost/certify/extensions.hpp>
#include <boost/certify/https_verification.hpp>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

#include "../include/depthupdate.hpp"
#include "../include/orderbook.hpp"

using tcp = boost::asio::ip::tcp;

void
on_error(boost::beast::error_code ec, char const* what);

std::vector<boost::json::value> 
parse_multiple_json_objects(std::string const& input);

static std::unordered_map<std::string, order_book> order_book_by_ticker; 

class binance_wss_session : public std::enable_shared_from_this<binance_wss_session>
{
    tcp::resolver resolver_;
    
    boost::beast::websocket::stream<
        boost::beast::ssl_stream<boost::beast::tcp_stream>> ws_;
    
    boost::beast::multi_buffer buffer_;

    std::string host_;
    
    std::vector<std::string> const* tickers;
public:
    explicit binance_wss_session(boost::asio::io_context& ioc, boost::asio::ssl::context& ctx)
        : resolver_(boost::asio::make_strand(ioc))
        , ws_(boost::asio::make_strand(ioc), ctx)
    {
    }

    void run(char const* host,
             char const* port,
             std::vector<std::string> const& tickers);

    void on_resolve(boost::beast::error_code ec,
                    tcp::resolver::results_type results);

    void on_connect(boost::beast::error_code ec,
                    tcp::resolver::results_type::endpoint_type ep);

    void on_ssl_handshake(boost::beast::error_code ec);

    void on_handshake(boost::beast::error_code ec);

    void on_read(boost::beast::error_code ec,
                 std::size_t bytes_transferred);
    
    void parse_diff();
    
    ~binance_wss_session();
};