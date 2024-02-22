#pragma once 

#include <boost/json.hpp>

#include <map>
#include <iostream>

#include "depthupdate.hpp"

class order_book {
    uint64_t last_update;

    std::string_view const ticker;

    // Quick lookup for best element using std::map
    // std::map uses red-black tree to keep the elements sorted
    std::map<double, double, std::greater<double>> buy_orders; 
    std::map<double, double, std::less<double>>sell_orders;

    // Quick lookup for duplicates and/or price using a hashmap
    std::unordered_map<double, double> buy_quick; 
    std::unordered_map<double, double> sell_quick;
public:
    order_book(std::string_view const& t);

    void update(boost::json::value& newUpdate);

    double get_best_buy() const;

    double get_best_sell() const;

    std::string_view get_ticker() const;
};