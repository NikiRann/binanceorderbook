
#include "../include/session.hpp"

constexpr auto websocket_host = "fstream.binance.com";
constexpr auto websocket_port = "443";

// const std::vector<std::string> TICKERS {"btcusdt", "ethusdt"};
const std::vector<std::string> TICKERS {"btcusdt"};

int main(int argc, char** argv)
{
    auto& depth_info_by_ticker = ThreadSafeHashMap::getInstance();

    std::vector<std::thread> threads;

    for (auto& t : TICKERS) {
        depth_info_by_ticker.insert(t, std::numeric_limits<uint64_t>::max());
        threads.emplace_back(update_depth, t);
    }

    boost::asio::io_context ioc;

    boost::asio::ssl::context ctx{boost::asio::ssl::context::tlsv12_client};

    ctx.set_verify_mode(boost::asio::ssl::verify_peer | boost::asio::ssl::context::verify_fail_if_no_peer_cert);
    ctx.set_default_verify_paths();
    
    boost::certify::enable_native_https_server_verification(ctx);
    
    std::make_shared<BinanceWssSession>(ioc, ctx)->run(websocket_host, websocket_port, TICKERS);
    
    ioc.run(); 

    for (std::thread& th : threads) {
        if (th.joinable()) {
            th.join();
        }
    }

    return EXIT_SUCCESS;
}