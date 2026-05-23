FROM ghcr.io/userver-framework/ubuntu-24.04-userver:latest AS builder

WORKDIR /build

RUN apt-get update && apt-get install -y --no-install-recommends \
    libssl-dev \
    libcrypto++-dev \
    && rm -rf /var/lib/apt/lists/*

RUN git clone https://github.com/Thalhammer/jwt-cpp.git /tmp/jwt-cpp && \
    cd /tmp/jwt-cpp && \
    mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release -DJWT_BUILD_EXAMPLES=OFF -DJWT_BUILD_TESTS=OFF && \
    make install -j$(nproc)

COPY CMakeLists.txt ./
COPY src/ ./src/

RUN mkdir -p build && \
    cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc)

FROM ubuntu:24.04 AS runtime

RUN apt-get update && apt-get install -y --no-install-recommends \
    libssl3 \
    libcrypto++8 \
    libpq5 \
    libmongoc-1.0-0 \
    libbson-1.0-0 \
    libhiredis1.1.0 \
    libcurl4t64 \
    libev4 \
    libjemalloc2 \
    libboost-iostreams1.83.0 \
    libboost-filesystem1.83.0 \
    libboost-program-options1.83.0 \
    libboost-stacktrace1.83.0 \
    libre2-10 \
    libfmt9 \
    libcctz2 \
    libyaml-cpp0.8 \
    && rm -rf /var/lib/apt/lists/*

RUN ldconfig

WORKDIR /app

COPY --from=builder /build/build/delivery_api ./delivery_api
COPY ./configs/ ./configs/

# Создаем папку для логов и даем права
RUN mkdir -p /app/logs && chown -R 1000:1000 /app

USER 1000

EXPOSE 8080

CMD ["./delivery_api", "-c", "configs/static_config.yaml"]
