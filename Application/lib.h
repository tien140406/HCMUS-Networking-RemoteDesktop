#pragma once
#include <iostream>
#include <string>
#include <curl/curl.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include <regex>
#include <windows.h>
#include <shellapi.h>
#include <sstream>
#include <fstream>
#include <windows.h>
#include <cstdint>
#include <cstdlib>
#include <algorithm>
#include <tlhelp32.h>
#include <openssl/evp.h>
#include <iomanip>
#include <algorithm>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>
#include <mutex>
#include <filesystem>
#pragma comment(lib, "ws2_32.lib")

using std::cerr, std::endl, std::cout;
using std::ifstream, std::ofstream;
using std::regex, std::smatch, std::sregex_iterator;
using std::string;
using std::vector, std::string, std::istringstream;