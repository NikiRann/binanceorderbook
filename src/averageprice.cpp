#include "../include/averageprice.hpp"

void compute_moving_averages(std::vector<std::string> const& tickers) {
    auto& price_info_by_ticker = thread_safe_hashmap<std::string, price_info>::getInstance();
    
    std::unordered_map<std::string, std::queue<long double>> price_series;

    std::unordered_map<std::string, long double> running_sums;

    std::unordered_map<std::string, long double> sma;
    std::unordered_map<std::string, long double> ema;

    while (1) {
        for (auto const& t : tickers) {
            if (price_info_by_ticker.contains(t)) {
                price_info res;
                if (price_info_by_ticker.get(t, res)) { 
                    long double midpoint_price = (res.sell + res.buy) / 2.0;

                    if (price_series[t].size() == 60) {
                        running_sums[t] -= price_series[t].front();
                        price_series[t].pop();
                    }
                    price_series[t].push(midpoint_price);
                    running_sums[t] += midpoint_price;

                    sma[t] = running_sums[t] / price_series[t].size();

                    long double multiplier = 2.0L / (price_series[t].size() + 1);
                    if (ema.find(t) == ema.end()) { 
                        ema[t] = sma[t];
                    } else {
                        ema[t] = (midpoint_price - ema[t]) * multiplier + ema[t];
                    }

                    std::cout << ">> " << t << " SMA: " << sma[t] << ", EMA: " << ema[t] << '\n';
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1)); 
    }
}