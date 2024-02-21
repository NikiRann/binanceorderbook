CC=g++
LIB=-L/home/rnikolay/boost/boost_1_84_0/stage/lib -L/usr/lib/openssl
INC=-I/home/rnikolay/boost/boost_1_84_0 -I/usr/include/openssl
CXXFLAGS= -O3 -Wall

LDFLAGS= -lcrypto -lssl
BOOST_LIBS= -lboost_system -lboost_thread -lboost_json

SRCS := $(wildcard src/*.cpp)
OBJS := $(SRCS:src/%.cpp=%.o)

default: all

all: binance

binance: $(OBJS)
		echo $(OBJS)
		$(CC) $(OBJS) -o $@ $(LIB) $(INC) $(CXXFLAGS) $(BOOST_LIBS) $(LDFLAGS)

%.o: src/%.cpp
		$(CC) $(CXXFLAGS) $(INC) -c $< -o $@

clean:
		rm -f $(OBJS) binance