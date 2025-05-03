FROM alpine:3.17.0 AS build

RUN apk update && \
    apk add --no-cache \
    build-base \
    cmake \
    boost1.80-dev=1.80.0-r3 \
    nlohmann-json \
    unixodbc-dev

WORKDIR /rest_sandbox

COPY source/ ./source/
COPY CMakeLists.txt .
COPY CMakePresets.json .

RUN cmake . && \
    cmake --build .