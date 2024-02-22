#pragma once 

#include <boost/json.hpp>

#include <map>
#include <iostream>

#include "depthupdate.hpp"

class order_book {
    std::string_view const ticker;
    
    uint64_t last_update;
    // Quick lookup for best element using std::map
    // std::map uses red-black tree to keep the elements sorted
    std::map<long double, long double, std::greater<long double>> buy_orders; 
    std::map<long double, long double, std::less<long double>>sell_orders;

    // Quick lookup for duplicates and/or price using a hashmap
    std::unordered_map<long double, long double> buy_quick; 
    std::unordered_map<long double, long double> sell_quick;
public:
    order_book(std::string_view const& t);

    void update(boost::json::object const& newUpdate);

    long double get_best_buy() const;

    long double get_best_sell() const;

    std::string_view get_ticker() const;
};