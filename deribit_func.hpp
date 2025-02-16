#include <bits/stdc++.h>
#include "credential.hpp"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
using namespace std;
using json = nlohmann::json;

const int MAX_RETRIES = 3; // Maximum number of retries for network requests
const int RETRY_DELAY = 2; // Delay between retries in seconds

void printColored(const  string& message, const  string& color) {
     cout << color << message << "\033[0m" <<  endl;
}


// Function to handle JSON response
void handleResponse(const string& response, const function<void(const json&)>& onSuccess) {
    try {
        auto jsonResponse = json::parse(response);
        if (jsonResponse.contains("error")) {
            auto error = jsonResponse["error"];
            cerr << "Error: " << error["message"].get<string>() << " (" << error["code"].get<int>() << ")\n";
            return;
        }
        onSuccess(jsonResponse["result"]);
    } catch (const json::exception& e) {
        cerr << "JSON parsing error: " << e.what() << "\n";
    }
}


// Helper function for CURL callbacks
size_t WriteCallback(void* contents, size_t size, size_t nmemb,  string* userp) {
    size_t totalSize = size * nmemb;
    userp->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

// Function to initialize CURL with common settings
CURL* initializeCurl(const string& url, string& responseBuffer, const string& accessToken = "") {
    CURL* curl = curl_easy_init();
    if (!curl) return nullptr;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);
    curl_easy_setopt(curl, CURLOPT_CAINFO, CA_CERT_PATH.c_str());

    if (!accessToken.empty()) {
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }

    return curl;
}

// Function to authenticate
 string authenticate() {
     string responseBuffer;
     string url = API_BASE_URL + "/public/auth?client_id=" + CLIENT_ID + "&client_secret=" + CLIENT_SECRET + "&grant_type=client_credentials";

    int retries = 0;
    while (retries < MAX_RETRIES) {
        CURL* curl = initializeCurl(url, responseBuffer);
        if (!curl) return "";

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
             string accessToken;
            handleResponse(responseBuffer, [&accessToken](const json& result) {
                accessToken = result.value("access_token", "");
                printColored("Authentication successful.", "\033[1;32m");
            });

            curl_easy_cleanup(curl);
            return accessToken;
        } else {
            printColored("CURL error: " +  string(curl_easy_strerror(res)), "\033[1;31m");
            curl_easy_cleanup(curl);
            retries++;
            if (retries < MAX_RETRIES) {
                printColored("Retrying... (" +  to_string(retries) + "/" +  to_string(MAX_RETRIES) + ")", "\033[1;33m");
                 this_thread::sleep_for( chrono::seconds(RETRY_DELAY));
            }
        }
    }

    printColored("Failed to authenticate after " +  to_string(MAX_RETRIES) + " retries.", "\033[1;31m");
    return "";
}






void placeOrder(const  string& accessToken, const  string& instrumentName, int amount,const  string& type,int price) {
    string responseBuffer;
    string url = API_BASE_URL + "/private/buy?instrument_name=" + instrumentName + "&amount=" +  to_string(amount) + "&type=" + type ;
    if(type == "limit"){
        url+="&price="+to_string(price);
    }
    CURL* curl = initializeCurl(url, responseBuffer, accessToken);
    if (!curl) return;

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        cerr << "CURL error: " << curl_easy_strerror(res) << "\n";
        curl_easy_cleanup(curl);
        return;
    }

    handleResponse(responseBuffer, [](const json& result) {
        cout << "Order placed successfully: \n";
        cout << result.dump(4) << "\n";
    });

    curl_easy_cleanup(curl);
}


// Function to cancel an order
void cancelOrder(const string& accessToken, const string& orderId) {
    string responseBuffer;
    string url = API_BASE_URL + "/private/cancel?order_id=" + orderId;

    CURL* curl = initializeCurl(url, responseBuffer, accessToken);
    if (!curl) return;

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        cerr << "CURL error: " << curl_easy_strerror(res) << "\n";
        curl_easy_cleanup(curl);
        return;
    }

    handleResponse(responseBuffer, [](const json& result) {
        cout << "Order canceled successfully: \n";
        cout << result.dump(4) << "\n";
    });

    curl_easy_cleanup(curl);
}
void subscribeChannel(const string& accessToken, const string& channelId) {
    string responseBuffer;
    string url = API_BASE_URL + "/private/subscribe?channel_id=" + channelId;

    CURL* curl = initializeCurl(url, responseBuffer, accessToken);
    if (!curl) return;

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        cerr << "CURL error: " << curl_easy_strerror(res) << "\n";
        curl_easy_cleanup(curl);
        return;
    }

    curl_easy_cleanup(curl);
}
void unsubscribeChannel(const string& accessToken, const string& channelId) {
    string responseBuffer;
    string url = API_BASE_URL + "/private/unsubscribe?channel_id=" + channelId;

    CURL* curl = initializeCurl(url, responseBuffer, accessToken);
    if (!curl) return;

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        cerr << "CURL error: " << curl_easy_strerror(res) << "\n";
        curl_easy_cleanup(curl);
        return;
    }

    curl_easy_cleanup(curl);
}

// Function to modify an order
void modifyOrder(const string& accessToken, const string& orderId, int newAmount) {
    string responseBuffer;
    string url = API_BASE_URL + "/private/edit?order_id=" + orderId + "&amount=" + to_string(newAmount);

    CURL* curl = initializeCurl(url, responseBuffer, accessToken);
    if (!curl) return;

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        cerr << "CURL error: " << curl_easy_strerror(res) << "\n";
        curl_easy_cleanup(curl);
        return;
    }

    handleResponse(responseBuffer, [](const json& result) {
        cout << "Order modified successfully: \n";
        cout << result.dump(4) << "\n";
    });

    curl_easy_cleanup(curl);
}

// Function to get the order book
void getOrderBook(const string& instrumentName) {
    string responseBuffer;
    string url = API_BASE_URL + "/public/get_order_book?instrument_name=" + instrumentName;

    CURL* curl = initializeCurl(url, responseBuffer);
    if (!curl) return;

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        cerr << "CURL error: " << curl_easy_strerror(res) << "\n";
        curl_easy_cleanup(curl);
        return;
    }

    handleResponse(responseBuffer, [](const json& result) {
        cout << "Order book: \n";
        cout << result.dump(4) << "\n";
    });

    curl_easy_cleanup(curl);
}

// Function to view current positions
void viewPositions(const string& accessToken) {
    string responseBuffer;
    string url = API_BASE_URL + "/private/get_positions";

    CURL* curl = initializeCurl(url, responseBuffer, accessToken);
    if (!curl) return;

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        cerr << "CURL error: " << curl_easy_strerror(res) << "\n";
        curl_easy_cleanup(curl);
        return;
    }

    handleResponse(responseBuffer, [](const json& result) {
        cout << "Current positions: \n";
        cout << result.dump(4) << "\n";
    });

    curl_easy_cleanup(curl);
}
