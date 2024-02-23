# Task

We are looking to get the latest Market Data (in this case Orderbook Data - how much
we can buy/sell at certain price levels) from Binance, aggregate it and print it. While
dealing with Orderbook data we require the latest information published by the
exchange to reach us ASAP since we want to be in sync with the latest orderbook state.
We prefer to use websockets since they are the fastest available way to get updated
from the exchange.
We need to pick the right data structure that will suit our main goal which is to carry out
different aggregations and calculations on the available order book data.

# Binance websocket's

Using libssl, some help from boost and the requirements from Binance for mainting an orderbook I am sending update info from all symbols into each corresponding orderbook every 100ms. On initialization or on data loss an api call is being made to get the depth snapshot and synchronise the updates.

# Orderbook

The data strucure in which we store our orderbook is crucal to the performance of this task. In this implementation the orderbook consists of two key data structures. I use an **Red-Black tree** with std::map to ensure constant lookup **O(1)** of the highest buy order and the lowest sell order. The first tree is descending and the second ascending. To insert and remove from the tree the time complexity is **O(logN)**. But this is not enough because we are constantly aggregating upgrades and thus looking for our buy and sell orders to be unique. I achived this by including two Hashmaps with keys equal to the price of the corresponding bid (1st hashmap) or ask (2nd hashmap). This way the check if a price exists is decreased from **O(logN)** to **O(1)**.

```c++
class order_book {
    std::string_view const ticker;
    
    uint64_t last_update;
    // Quick lookup for best element using std::map
    // std::map uses red-black tree to keep the elements sorted
    std::map<long double, long double, std::greater<long double>> buy_orders; 
    std::map<long double, long double, std::less<long double>> sell_orders;

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
```

# Symbols

The solution accepts tickers in lowercase as cli arguments
```bash
./binance btcusdt ethusdt
```
There is no limit on them. 


# SMA and EMA

Both are being calculated **O(1)** for every ticker's midpoint price. This is done on a seperate thread every 1 second.

# Libraries used

**Boost 1.84.0** The solution is using mainly boost::beast for tcp wss connection and boost::json for aggregating the result. 

> [!NOTE]
> I am using an extension for boost to help with the SSL root certificates when doing a ssl handshake: **https://github.com/djarek/certify**

**libssl-dev** Used by boost for ssl interfaces

# Local build

After installing the above libriaries (boost, boost::certify, libssl-dev) and replacing the paths in the **Makefile** with your installation paths you can just run in the root dir:

```bash
make
./binance btcusdt ethusdt
``` 

# Docker

A docker image could be built from the solution an be ran with the corresponding tickers you want as cli arguments.

```bash
docker build -t binance-orderbook-app .
docker run --name my-binance-orderbook-app binance-orderbook-app btcusdt ethusdt 
```

The docker image can be found on:
https://hub.docker.com/repository/docker/rnikolay/binance-orderbook-app/

# Output

Please be aware that the 100ms updates produce output to the console extremly fast, especially when getting updates for two or more tickers. Running localy produces dynamic fast output with no problems, but running the docker container buffers some of the output and it is comming in bigger chunks due to I/O abstractions caused by docker. It is possible for a more detailed view to redirect the output into a file and selecting only one ticker for the ease of debugging:

```bash
docker run --name new-binance-orderbook-app binance-orderbook-app
```

Or

```bash
./binance btcusdt
```

An example output looks like this:
```
>> btcusdt SMA: 50956.65, EMA: 50956.65
>> BUY 1 btcusdt: 50956.60 | SELL 1 btcusdt: 50956.70
>> btcusdt 3.80
>> BUY 1 btcusdt: 50956.60 | SELL 1 btcusdt: 50956.70
>> btcusdt 3.80
>> BUY 1 btcusdt: 50956.60 | SELL 1 btcusdt: 50956.70
>> btcusdt 3.80
>> BUY 1 btcusdt: 50956.60 | SELL 1 btcusdt: 50956.70
>> btcusdt 4.30
>> BUY 1 btcusdt: 50956.60 | SELL 1 btcusdt: 50956.70
>> btcusdt 4.70
>> BUY 1 btcusdt: 50956.60 | SELL 1 btcusdt: 50956.70
>> btcusdt 4.80
>> BUY 1 btcusdt: 50956.60 | SELL 1 btcusdt: 50956.70
>> btcusdt 4.10
>> BUY 1 btcusdt: 50956.60 | SELL 1 btcusdt: 50956.70
>> btcusdt 4.70
>> BUY 1 btcusdt: 50956.60 | SELL 1 btcusdt: 50956.70
>> btcusdt 4.10
>> BUY 1 btcusdt: 50956.60 | SELL 1 btcusdt: 50956.70
>> btcusdt 3.90
>> btcusdt SMA: 50956.65, EMA: 50956.65
>> BUY 1 btcusdt: 50956.60 | SELL 1 btcusdt: 50956.70
>> btcusdt 4.20
```

Every 100 ms we get an update to the standard output for the best bid/ask for every ticker and the corresponding spread.
Every 1 second we get and update for the SMA and EMA for each symbol.

# Conclusion

I hope I have covered most of the functionallity in this README, but if any questons come up please feel free to email or call me. 