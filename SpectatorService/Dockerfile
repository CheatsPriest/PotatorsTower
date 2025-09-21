FROM gcc:latest as build

RUN apt update && \
    apt install -y cmake 
RUN apt install -y build-essential
RUN apt install -y git
RUN apt install -y pkg-config
RUN apt install -y autoconf libtool

WORKDIR /deps

RUN git clone --recurse-submodules -b v1.75.0 --depth 1 --shallow-submodules https://github.com/grpc/grpc

RUN mkdir -p /deps/grpc/build && \
    cd /deps/grpc/build && \
    cmake -DgRPC_INSTALL=ON \
      -DgRPC_BUILD_TESTS=OFF \
      .. && \
    make -j12 install

RUN wget https://github.com/jtv/libpqxx/archive/refs/tags/7.10.0.tar.gz \
    && tar -xzf 7.10.0.tar.gz \
    && cd libpqxx-7.10.0 \
    && mkdir build \
    && cd build \
    && cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    && make -j$(nproc) install

RUN apt install -y \
    libssl-dev \
    libbrotli-dev

ADD . /app

WORKDIR /app/build

RUN cmake .. && \
    cmake --build .

FROM ubuntu:latest

RUN apt update \
    && apt install -y \
    libpq5 \
    libbrotli1

WORKDIR /app

COPY --from=build /app/build/spectator_service .

ENTRYPOINT ["./spectator_service"]
