#define _WIN32_WINNT 0x0601

#include <iostream>
#include <fstream>
#include <thread>
#include <csignal>
#include <atomic>
#include <chrono>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <nlohmann/json.hpp>

#include <bsoncxx/builder/stream/document.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
