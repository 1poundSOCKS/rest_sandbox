#define _WIN32_WINNT 0x0601

#define _UNICODE

#include <iostream>
#include <fstream>
#include <thread>
#include <csignal>
#include <atomic>
#include <chrono>
#include <unistd.h>
#include <sstream>
#include <map>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

#include <nlohmann/json.hpp>
#include <pqxx/pqxx>
