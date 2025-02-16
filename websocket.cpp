// websocket.cpp
// Implements WebSocket communication logic for the application.

#include <websocketpp/config/asio_no_tls.hpp>  // Use non-TLS configuration
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <unordered_map>
#include <set>
#include <mutex>
#include <thread>
#include <random>
#include <atomic>
#include <csignal>
#include <chrono>
#include <curl/curl.h>
#include "websocket.hpp"
using namespace std;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

typedef websocketpp::server<websocketpp::config::asio> WebSocketServer;

 atomic<bool> running(true);  // Flag to track server running status

// Signal handler for Ctrl+C (SIGINT)
void signal_handler(int signum) {
     cout << "Signal " << signum << " received. Shutting down gracefully..." <<  endl;
    running = false;  // Set running flag to false to stop the server
}

int main() {
     signal(SIGINT, signal_handler);

    DeribitWebSocketServer server;
     cout << "Starting WebSocket server on port 9002..." <<  endl;

     thread server_thread([&]() {
        server.run(9002);
    });

    while (running) {
         this_thread::sleep_for( chrono::milliseconds(100));
    }

    server.stop();
    server_thread.join();

     cout << "Server shutdown complete." <<  endl;

    return 0;
}
