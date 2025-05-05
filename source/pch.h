#define _WIN32_WINNT 0x0601

#include <iostream>
#include <thread>
#include <csignal>
#include <atomic>
#include <chrono>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <nlohmann/json.hpp>
