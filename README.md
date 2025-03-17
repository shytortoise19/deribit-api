# Deribit Order Execution System

## Overview
The **Deribit Order Execution System** is a high-performance trading bot built in **C++**, designed to interact with the Deribit Test exchange. It provides essential trading functionalities, including placing, modifying, and canceling orders, retrieving order book data, and tracking open positions.

## Features
- **Order Management**: Place, modify, and cancel orders with low latency.
- **Market Data Retrieval**: Fetch order book data and monitor real-time price movements.
- **Position Tracking**: View and manage open positions across multiple trading instruments.
- **WebSocket Support**: A WebSocket server for real-time order book updates for subscribed symbols.
- **Multi-Asset Support**: Supports spot, futures, and options trading.
- **Secure Authentication**: Uses API keys for secure communication with Deribit.

## Installation
### Prerequisites
Ensure you have the following installed:
- **C++17 or later**
- **Boost Libraries** (for networking and async operations)
- **WebSocket++** (for WebSocket communication)
- **cURL** (for HTTP API requests)


## Usage
### Configure API Keys
1. Create a new account on [Deribit Test](https://test.deribit.com/).
2. Generate API keys and store them securely.
3. Update the `config.json` file with your API credentials.

## Advanced Features (Bonus)
- Implemented a **WebSocket server** to allow clients to subscribe to a symbol and receive continuous order book updates.
- Optimized for **low-latency execution**, ensuring real-time responsiveness for market data and trading actions.


