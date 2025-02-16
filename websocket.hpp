#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>
#include <mutex>
#include <thread>
#include <random>
#include <atomic>
#include <bits/stdc++.h>
#include <csignal>
#include <chrono>
#include <curl/curl.h>
#include <string>
using namespace std;

using json = nlohmann::json;


using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

typedef websocketpp::server<websocketpp::config::asio> WebSocketServer;
size_t writeCallback(void* contents, size_t size, size_t nmemb,  string* buffer) {
    size_t totalSize = size * nmemb;
    buffer->append((char*)contents, totalSize);
    return totalSize;
}



class DeribitWebSocketServer {
public:
    DeribitWebSocketServer() {
        server.init_asio();
        server.set_open_handler(bind(&DeribitWebSocketServer::onOpen, this, _1));
        server.set_close_handler(bind(&DeribitWebSocketServer::onClose, this, _1));
        server.set_message_handler(bind(&DeribitWebSocketServer::onMessage, this, _1, _2));
    }

    void run(uint16_t port) {
        server.listen(port);
        server.start_accept();
        server.run();
    //      thread serverThread([this]() { server.run(); });

    //     // Main loop to periodically fetch and broadcast order book updates
    //     while (running) {
    //         for (const auto& subscription : subscribedChannels) {
    //             const  string& symbol = subscription;
    //             sendOrderBookUpdates(symbol);
    //         }
    //          this_thread::sleep_for( chrono::seconds(5));  // 5 seconds delay
    //     }

    //     serverThread.join();
    }

    void stop() {
        server.stop_listening();  // Stop accepting new connections
        running = false;
         lock_guard< mutex> lock(connectionMutex);
        for (auto& [hdl, _] : subscriptions) {
            try {
                server.close(*hdl, websocketpp::close::status::normal, "Server shutting down");
            } catch (const websocketpp::exception& e) {
                 cerr << "Error closing connection during shutdown: " << e.what() <<  endl;
            }
        }
         this_thread::sleep_for( chrono::seconds(1));
    }

    void broadcastMessage(const  string& topic, const nlohmann::json& message) {
         lock_guard< mutex> lock(connectionMutex);
        for (const auto& [hdl, topics] : subscriptions) {
            if (topics.count(topic)) {
                try {
                    server.send(*hdl, message.dump(), websocketpp::frame::opcode::text);
                } catch (const websocketpp::exception& e) {
                     cerr << "Error broadcasting message to client: " << e.what() <<  endl;
                }
            }
        }
    }

private:
    WebSocketServer server;
     mutex connectionMutex;
    //string accessToken = authenticate();

     unordered_map< shared_ptr<websocketpp::connection_hdl>,  set< string>> subscriptions;
     set< string> subscribedChannels;
     atomic<bool> running = true;

    void onOpen(websocketpp::connection_hdl hdl) {
         lock_guard< mutex> lock(connectionMutex);
        subscriptions[ make_shared<websocketpp::connection_hdl>(hdl)];
         cout << "Connection opened: " << hdl.lock().get() <<  endl;
    }

    void onClose(websocketpp::connection_hdl hdl) {
         lock_guard< mutex> lock(connectionMutex);
        auto it = subscriptions.find( make_shared<websocketpp::connection_hdl>(hdl));
        if (it != subscriptions.end()) {
            for (const auto& symbol : it->second) {
                subscribedChannels.erase(symbol);
            }
            subscriptions.erase(it);
        }
         cout << "Connection closed: " << hdl.lock().get() <<  endl;
    }

    void onMessage(websocketpp::connection_hdl hdl, WebSocketServer::message_ptr msg) {
        try {
            auto jsonData = nlohmann::json::parse(msg->get_payload());

            if (jsonData.contains("action") && jsonData["action"] == "subscribe" && jsonData.contains("symbol")) {
                 string symbol = jsonData["symbol"];
                 lock_guard< mutex> lock(connectionMutex);
                if (subscribedChannels.find(symbol) != subscribedChannels.end()) {
                sendErrorMessage(hdl, "Symbol " + symbol + " is already subscribed.");
                }
                else{
                sendOrderBookUpdates(symbol,hdl);
                }
            } else if (jsonData.contains("action") && jsonData["action"] == "unsubscribe" && jsonData.contains("symbol")) {
                 string symbol = jsonData["symbol"];
                 lock_guard< mutex> lock(connectionMutex);
                if(subscribedChannels.find(symbol) != subscribedChannels.end())
                {
                    subscribedChannels.erase(symbol);
                }
                auto it = subscriptions.find( make_shared<websocketpp::connection_hdl>(hdl));
                if (it != subscriptions.end() && it->second.count(symbol)) {
                    it->second.erase(symbol);
                }
                 cout << "Client " << hdl.lock().get() << " unsubscribed from symbol: " << symbol <<  endl;
            } else {
                sendErrorMessage(hdl, "Invalid action or missing symbol");
            }
        } catch (const nlohmann::json::exception& e) {
            sendErrorMessage(hdl, "Invalid message format");
        }
    }


   void sendOrderBookUpdates(const  string& symbol, websocketpp::connection_hdl hdl) {
     thread([this, symbol, hdl]() {
        CURL* curl = curl_easy_init();
        if (!curl) {
             cerr << "CURL initialization failed" <<  endl;
            return;
        }

        int retryCount = 0;
        const int maxRetries = 3;

        do {
            try {
                 string url = "https://www.deribit.com/api/v2/public/get_order_book?instrument_name=" + symbol;
                 string responseBuffer;

                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);

                CURLcode res = curl_easy_perform(curl);
                if (res != CURLE_OK) {
                     cerr << "CURL error: " << curl_easy_strerror(res) <<  endl;

                    if (++retryCount > maxRetries) {
                         cerr << "Max retries reached. Stopping updates for symbol: " << symbol <<  endl;
                        break;
                    }

                     this_thread::sleep_for( chrono::seconds(2)); // Backoff before retry
                    continue;
                }

                nlohmann::json orderBook = nlohmann::json::parse(responseBuffer);

                if (orderBook.contains("error")) {
                     cerr << "Error for symbol " << symbol << ": "
                              << orderBook["error"]["message"].get< string>() <<  endl;

                    nlohmann::json errorMessage = {
                        {"error", orderBook["error"]},
                        {"symbol", symbol}
                    };

                    server.send(hdl, errorMessage.dump(), websocketpp::frame::opcode::text);
                    break;
                }

                retryCount = 0; // Reset retries on success

                {
                     lock_guard< mutex> lock(connectionMutex);
                    subscriptions[ make_shared<websocketpp::connection_hdl>(hdl)].insert(symbol);
                    subscribedChannels.insert(symbol);
                }

                nlohmann::json message = {
                    {"symbol", symbol},
                    {"orderBook", orderBook}
                };
                server.send(hdl, message.dump(), websocketpp::frame::opcode::text);

            } catch (const  exception& e) {
                 cerr << "Exception in sendOrderBookUpdates: " << e.what() <<  endl;
                break;
            }

             this_thread::sleep_for( chrono::seconds(1)); // Delay for next update
        } while (subscribedChannels.find(symbol) != subscribedChannels.end());

        curl_easy_cleanup(curl); // Ensure cleanup
    }).detach();
}


    void sendErrorMessage(websocketpp::connection_hdl hdl, const  string& errorMsg) 
    {
        nlohmann::json errorResponse = {
            {"error", errorMsg}
        };

        try {
            server.send(hdl, errorResponse.dump(), websocketpp::frame::opcode::text);
        } catch (const websocketpp::exception& e) {
             cerr << "Error sending error message: " << e.what() <<  endl;
        }
    }
};
