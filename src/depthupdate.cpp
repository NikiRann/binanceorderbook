#include "../include/depthupdate.hpp"

void update_depth(std::string const& ticker) {
    boost::asio::io_context ioc;
    boost::asio::ssl::context ctx{boost::asio::ssl::context::tlsv12_client};

    ctx.set_verify_mode(boost::asio::ssl::verify_peer | boost::asio::ssl::context::verify_fail_if_no_peer_cert);
    ctx.set_default_verify_paths();
    
    boost::certify::enable_native_https_server_verification(ctx);

    tcp::resolver resolver(ioc);
    boost::beast::ssl_stream<boost::beast::tcp_stream> stream(ioc, ctx);

    if(! SSL_set_tlsext_host_name(stream.native_handle(), binanceApiHost)) {
        boost::beast::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
        throw boost::beast::system_error{ec};
    }

    auto const results = resolver.resolve(binanceApiHost, "443");

    boost::beast::get_lowest_layer(stream).connect(results);

    stream.handshake(boost::asio::ssl::stream_base::client);

    std::cout << boost::algorithm::to_upper_copy(ticker) << '\n';

    boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get, "/fapi/v1/depth?symbol=" + boost::algorithm::to_upper_copy(ticker) + "&limit=1000", 11};
    req.set(boost::beast::http::field::host, binanceApiHost);
    req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    boost::beast::http::write(stream, req);
    boost::beast::error_code ec;
    boost::beast::http::response<boost::beast::http::string_body> res;
    boost::beast::flat_buffer buffer;

    buffer.reserve(100000);

    boost::beast::http::read(stream, buffer, res, ec);

    if(ec) {
        std::cerr << "Error during read: " << ec.message() << std::endl;
    }
    
    boost::json::value depthInfo = boost::json::parse(res.body());

    auto& depth_info_by_ticker = ThreadSafeHashMap::getInstance();
    depth_info_by_ticker.insert(ticker, (uint64_t) depthInfo.at("lastUpdateId").as_int64());
}