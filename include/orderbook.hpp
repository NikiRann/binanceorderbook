#pragma once 

#include <boost/json.hpp>
#include <queue>

using order = std::pair<double, double>;

auto descending_compare = [](const order& a, const order& b) {
    return a.first < b.first;
};

auto ascending_compare = [](const order& a, const order& b) {
    return a.first < b.first;
};

class order_book {
    uint64_t last_update;

    const std::string_view ticker;

    std::priority_queue<order, std::vector<order>, decltype(descending_compare)> buy_orders;

    std::priority_queue<order, std::vector<order>, decltype(ascending_compare)> sell_orders;

    std::unordered_map<double, bool> buy_duplicates;
    std::unordered_map<double, bool> sell_duplicates;
public:
    void update(boost::json::value const& newUpdate);

    double get_best_buy() const;

    double get_best_sell() const;

    std::string_view get_ticker() const;
};