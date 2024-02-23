FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
    build-essential \
    g++ \
    libssl-dev \
    wget \
    git \
    && rm -rf /var/lib/apt/lists/*

RUN wget -qO- https://boostorg.jfrog.io/artifactory/main/release/1.84.0/source/boost_1_84_0.tar.gz | tar xvz -C /usr/local/ && \
    cd /usr/local/boost_1_84_0 && \
    ./bootstrap.sh && \
    ./b2 install

RUN git clone https://github.com/djarek/certify.git /usr/local/certify

WORKDIR /usr/src/myapp

COPY . .

ENV INC="-I/usr/local/boost_1_84_0 -I/usr/include/openssl -I/usr/local/certify/include"
ENV LIB="-L/usr/local/boost_1_84_0/stage/lib -L/usr/lib/openssl"

ENV LD_LIBRARY_PATH /usr/local/lib:${LD_LIBRARY_PATH}
RUN ldconfig

RUN g++ -O3 -Wall ${INC} src/*.cpp -o binance-orderbook ${LIB} -lcrypto -lssl -lboost_system -lboost_thread -lboost_json

ENTRYPOINT ["./binance-orderbook"]
# Set default command (can be overridden)
CMD ["btcusdt"]