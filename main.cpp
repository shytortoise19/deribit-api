// main.cpp
// Entry point of the application. Initializes the system and starts the main process.

#include <bits/stdc++.h>
#include <curl/curl.h>
#include "deribit_func.hpp"
#include "credential.hpp"
#include <nlohmann/json.hpp>
#include <chrono>
using namespace std;
using json = nlohmann::json;
using namespace chrono;

// Function Prototypes
void displayMenu();
void handleUserChoice(int choice, const string& accessToken);

/**
 * Main function to display the trading menu and handle user interaction.
 */
int main() {
    string accessToken = authenticate();
    if (accessToken.empty()) {
        printColored("Failed to authenticate. Exiting.", "\033[1;31m");
        return 1;
    }

    while (true) {
        displayMenu();
        int choice;
        cout << "Enter your choice: ";
        cin >> choice;
        if (choice == 6) {
            printColored("Exiting.", "\033[1;32m");
            break;
        }
        handleUserChoice(choice, accessToken);
    }
    return 0;
}

/**
 * Display the trading menu.
 */
void displayMenu() {
    cout << "\n====================================\n";
    cout << "        Deribit Trading Menu         \n";
    cout << "====================================\n";
    cout << "1. Place Order\n";
    cout << "2. Cancel Order\n";
    cout << "3. Modify Order\n";
    cout << "4. Get Order Book\n";
    cout << "5. View Current Positions\n";
    cout << "6. Exit\n";
}

/**
 * Handle user choice by invoking appropriate functions and measuring latencies.
 */
void handleUserChoice(int choice, const string& accessToken) {
    switch (choice) {
        case 1: {
            string instrumentName, type;
            int amount,price=0;

            cout << "Enter instrument name (e.g., BTC-PERPETUAL): ";
            cin >> instrumentName;
            cout << "Enter amount: ";
            cin >> amount;
            cout << "Enter type (e.g., market, limit): ";
            cin >> type;
            cout<<"Enter Price(for market enter 0): ";
            cin>>price;

            auto start = high_resolution_clock::now();
            placeOrder(accessToken, instrumentName, amount,type,price);
            auto end = high_resolution_clock::now();

            auto orderPlacementLatency = duration_cast<milliseconds>(end - start).count();
            cout << "Order placement latency: " << orderPlacementLatency << " ms\n";
            break;
        }
        case 2: {
            string orderId;
            cout << "Enter Order ID to cancel: ";
            cin >> orderId;

            auto start = high_resolution_clock::now();
            cancelOrder(accessToken, orderId);
            auto end = high_resolution_clock::now();

            auto cancellationLatency = duration_cast<milliseconds>(end - start).count();
            cout << "Order cancellation latency: " << cancellationLatency << " ms\n";
            break;
        }
        case 3: {
            string orderId;
            int newAmount;

            cout << "Enter Order ID to modify: ";
            cin >> orderId;
            cout << "Enter new amount: ";
            cin >> newAmount;

            auto start = high_resolution_clock::now();
            modifyOrder(accessToken, orderId, newAmount);
            auto end = high_resolution_clock::now();

            auto modificationLatency = duration_cast<milliseconds>(end - start).count();
            cout << "Order modification latency: " << modificationLatency << " ms\n";
            break;
        }
        case 4: {
            string instrumentName;
            cout << "Enter instrument name to get order book: ";
            cin >> instrumentName;

            auto start = high_resolution_clock::now();
            getOrderBook(instrumentName);
            auto end = high_resolution_clock::now();

            auto marketDataLatency = duration_cast<milliseconds>(end - start).count();
            cout << "Market data processing latency: " << marketDataLatency << " ms\n";
            break;
        }
        case 5: {
            auto start = high_resolution_clock::now();
            viewPositions(accessToken);
            auto end = high_resolution_clock::now();

            auto positionLatency = duration_cast<milliseconds>(end - start).count();
            cout << "Position viewing latency: " << positionLatency << " ms\n";
            break;
        }
        default:
            printColored("Invalid choice. Please try again.", "\033[1;31m");
    }
}
