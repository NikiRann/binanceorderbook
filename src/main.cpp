#include "../include/session.hpp"
#include "../include/averageprice.hpp"

constexpr auto websocket_host = "fstream.binance.com";
constexpr auto websocket_port = "443";

int main(int argc, char** argv)
{
    if (argc == 1) {
        std::cerr << "Example usage ./binance btcusdt ethusdt";
        return EXIT_FAILURE;
    }

    auto& depth_info_by_ticker = thread_safe_hashmap<std::string, uint64_t>::getInstance();

    std::vector<std::thread> threads;

    std::vector<std::string> tickers;

    for(int i = 1; i < argc; ++i) {
        tickers.push_back(argv[i]);
    }

    for (auto& t : tickers) {
        depth_info_by_ticker.insert(t, std::numeric_limits<uint64_t>::max());
        threads.emplace_back(update_depth, t);
    }
    
    threads.emplace_back(compute_moving_averages, tickers);
    boost::asio::io_context ioc;

    boost::asio::ssl::context ctx{boost::asio::ssl::context::tlsv12_client};

    ctx.set_verify_mode(boost::asio::ssl::verify_peer | boost::asio::ssl::context::verify_fail_if_no_peer_cert);
    ctx.set_default_verify_paths();
    
    boost::certify::enable_native_https_server_verification(ctx);
    
    std::make_shared<binance_wss_session>(ioc, ctx)->run(websocket_host, websocket_port, tickers);
    
    ioc.run(); 

    for (std::thread& th : threads) {
        if (th.joinable()) {
            th.join();
        }
    }

    return EXIT_SUCCESS;
}