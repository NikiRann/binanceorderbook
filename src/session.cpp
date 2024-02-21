#include "../include/session.hpp"

void
on_error(boost::beast::error_code ec, char const* what) {
    std::cerr << what << ": " << ec.message() << "\n";
}

void BinanceWssSession::run(char const* host,
                  char const* port,
                  std::vector<std::string> const& tickers) {    
    this->host_ = host;
    this->tickers = &tickers;

    resolver_.async_resolve(
        host,
        port,
        boost::beast::bind_front_handler(
            &BinanceWssSession::on_resolve,
            shared_from_this()));
}

void BinanceWssSession::on_resolve(boost::beast::error_code ec,
                         tcp::resolver::results_type results) {
    if(ec)
        return on_error(ec, "resolve");

    boost::beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

    boost::beast::get_lowest_layer(ws_).async_connect(
        results,
        boost::beast::bind_front_handler(
            &BinanceWssSession::on_connect,
            shared_from_this()));
}

void BinanceWssSession::on_connect(boost::beast::error_code ec, 
                         tcp::resolver::results_type::endpoint_type ep) {
    if(ec)
        return on_error(ec, "connect");

    boost::beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

    if(! SSL_set_tlsext_host_name(
            ws_.next_layer().native_handle(),
            host_.c_str()))
    {
        ec = boost::beast::error_code(static_cast<int>(::ERR_get_error()),
            boost::asio::error::get_ssl_category());
        return on_error(ec, "connect");
    }

    host_ += ':' + std::to_string(ep.port());
    
    ws_.next_layer().async_handshake(
        boost::asio::ssl::stream_base::client,
        boost::beast::bind_front_handler(
            &BinanceWssSession::on_ssl_handshake,
            shared_from_this()));
}

void BinanceWssSession::on_ssl_handshake(boost::beast::error_code ec) {
    if(ec)
        return on_error(ec, "ssl_handshake");

    boost::beast::get_lowest_layer(ws_).expires_never();

    ws_.set_option(
        boost::beast::websocket::stream_base::timeout::suggested(
            boost::beast::role_type::client));

    ws_.set_option(boost::beast::websocket::stream_base::decorator(
        [this](boost::beast::websocket::request_type& req)
        {
            req.set(boost::beast::http::field::user_agent,
                std::string(BOOST_BEAST_VERSION_STRING) +
                    " websocket-client-async-ssl");
        }));

    std::string target = "/stream?streams=";
    
    for (const auto& t : *tickers) {
        target.append(t + "@depth@100ms/");
    }
    
    target.pop_back();

    ws_.async_handshake(host_, target,
        boost::beast::bind_front_handler(
            &BinanceWssSession::on_handshake,
            shared_from_this()));
}

void BinanceWssSession::on_handshake(boost::beast::error_code ec) {
    if(ec)
        return on_error(ec, "handshake");

    ws_.async_read(
        buffer_,
        boost::beast::bind_front_handler(
            &BinanceWssSession::on_read,
            shared_from_this()));
}

void BinanceWssSession::on_read(boost::beast::error_code ec,
                      std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if(ec)
        return on_error(ec, "read");

    parse_diff();
    buffer_.consume(buffer_.size());

    ws_.async_read(
        buffer_,
        boost::beast::bind_front_handler(
            &BinanceWssSession::on_read,
            shared_from_this()));
}

// Helper function to find JSON objects in one string and parse them
std::vector<boost::json::value> parse_multiple_json_objects(const std::string& input) {
    std::vector<boost::json::value> jsonObjects;
    std::size_t startPos = 0;
    std::size_t openBrackets = 0;

    try {
        for (std::size_t i = 0; i < input.length(); ++i) {
            if (input[i] == '{') {
                if (openBrackets == 0) {
                    startPos = i;
                }
                ++openBrackets;
            } else if (input[i] == '}') {
                --openBrackets;
                if (openBrackets == 0 && startPos != std::string::npos) {
                    std::string jsonObjectStr = input.substr(startPos, i - startPos + 1);
                    jsonObjects.push_back(boost::json::parse(jsonObjectStr));
                    startPos = std::string::npos;
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception caught while parsing JSON: " << e.what() << '\n';
    }

    return jsonObjects;
}

void BinanceWssSession::parse_diff() {
    std::string input = boost::beast::buffers_to_string(buffer_.data());
    auto jsonObjects = parse_multiple_json_objects(input);

    for (const auto& orderBookUpdate : jsonObjects) {
        try {
            auto& data = orderBookUpdate.as_object().at("data").as_object();
            auto symbol = data.at("s").as_string();
            auto event = data.at("e").as_string(); 
            auto updateId = data.at("u").as_int64();
            auto firstUpdateId = data.at("U").as_int64();
            auto previousUpdateId = data.at("pu").as_int64();

            boost::algorithm::to_lower(symbol);
            auto& depth_info_by_ticker = ThreadSafeHashMap::getInstance();

            uint64_t current_depth_info;
            depth_info_by_ticker.get(symbol.c_str(), current_depth_info);

            if ((uint64_t)updateId < current_depth_info) {
                // std::cout << "DROPPED" << updateId << " " << current_depth_info << "\n";
                std::cout << "SUCCESS " <<  event << " " <<symbol << " " << updateId << " " << previousUpdateId << " " << current_depth_info << " " << firstUpdateId << "\n";
            
            } else {
                std::cout << "SUCCESS " <<  event << " " <<symbol << " " << updateId << " " << previousUpdateId << " " << current_depth_info << " " << firstUpdateId << "\n";
            }
        } catch (const std::exception& e) {
            std::cerr << "Exception caught while processing JSON object: " << e.what() << '\n';
        }
    }
}

BinanceWssSession::~BinanceWssSession() {
    ws_.close(boost::beast::websocket::close_code::normal);
}