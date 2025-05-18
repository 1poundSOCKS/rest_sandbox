FROM alpine:3.17.0 AS build

RUN apk update && \
    apk add --no-cache \
    build-base \
    cmake \
    git \
    boost1.80-dev=1.80.0-r3 \
    nlohmann-json \
    postgresql \
    postgresql-client \
    libpq-dev

WORKDIR /libpqxx

RUN git clone https://github.com/jtv/libpqxx.git
RUN cmake -B build -S libpqxx -DCMAKE_INSTALL_PREFIX=/usr
RUN cmake --build build
RUN cmake --install build

WORKDIR /rest_sandbox

COPY source/ ./source/
COPY CMakeLists.txt .
COPY CMakePresets.json .

RUN cmake --preset linux && \
    cmake --build .

COPY config.docker.json ./config.json

EXPOSE 8080

ENTRYPOINT ["bin/rest_sandbox"]
