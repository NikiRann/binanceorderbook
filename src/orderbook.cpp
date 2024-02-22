#include "../include/orderbook.hpp"

order_book::order_book(std::string_view const& t) : ticker(t), 
                                                    last_update(0) {}

void order_book::update(boost::json::value& new_update) {
    auto update_id = new_update.at("u").as_int64();
    auto previous_update_id = new_update.at("pu").as_int64();
    
    if (previous_update_id != last_update && last_update != 0) {
        // Reset, data loss and mismatched updates
        auto& depth_info_by_ticker = thread_safe_hashmap::getInstance();
        depth_info_by_ticker.insert(std::string(ticker), std::numeric_limits<uint64_t>::max());
        
        last_update = 0; // Not initialized

        buy_quick.clear();
        sell_quick.clear();

        buy_orders.clear();
        sell_orders.clear();

        std::thread depth_updater(update_depth, std::string(ticker));
        depth_updater.detach();
    } else {
        last_update = update_id;
        boost::json::array buys = new_update.as_object()["b"].as_array();
        boost::json::array sells = new_update.as_object()["s"].as_array();

        // Handle buy changes
        for (auto& buy_order : buys) {
            boost::json::array buy_details = buy_order.as_array();
            std::cout << "Price: " << buy_details[0].as_string() << ", Quantity: " << buy_details[1].as_string() << std::endl;
            
            double buy_price = buy_details[0].as_double(); 
            double buy_quantity = buy_details[1].as_double();

            if (buy_quick.find(buy_price) == buy_quick.end()) {
                if (buy_quantity != 0) {
                    buy_orders.insert(std::make_pair(buy_price, buy_quantity));
                    buy_quick[buy_price] = buy_quantity;
                }
            } else {
                if (buy_quantity == 0) {
                    buy_orders.erase(buy_price);
                    buy_quick.erase(buy_price);
                } else {
                    buy_orders[buy_price] = buy_quantity;
                    buy_quick[buy_price] = buy_quantity;
                }
            }
        }

        // Handle sales changes
        for (auto& sell_order : sells) {
            boost::json::array sell_details = sell_order.as_array();
            std::cout << "Price: " << sell_details[0].as_string() << ", Quantity: " << sell_details[1].as_string() << std::endl;
            
            double sell_price = sell_details[0].as_double(); 
            double sell_quantity = sell_details[1].as_double();

            if (sell_quick.find(sell_price) == sell_quick.end()) {
                if (sell_quantity != 0) {
                    sell_orders.insert(std::make_pair(sell_price, sell_quantity));
                    sell_quick[sell_price] = sell_quantity;
                }
            } else {
                if (sell_quantity == 0) {
                    sell_orders.erase(sell_price);
                    sell_quick.erase(sell_price);
                } else {
                    sell_orders[sell_price] = sell_quantity;
                    sell_quick[sell_price] = sell_quantity;
                }
            }
        }

    }
}

double order_book::get_best_buy() const {
    return (*buy_orders.begin()).first;
}

double order_book::get_best_sell() const {
    return (*sell_orders.begin()).first;
}

std::string_view order_book::get_ticker() const {
    return ticker;
}