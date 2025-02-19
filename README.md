# PluDux Backtesting Application

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)  
A web-based backtesting application written in C++ with [Dear ImGui](https://github.com/ocornut/imgui) and [ImPlot](https://github.com/epezent/implot), compiled to WebAssembly via [Emscripten](https://emscripten.org/). This tool allows you to load historical market data and custom trading strategy configurations, then run simulations to evaluate the performance of those strategies over time.

---

## Table of Contents

1. [Features](#features)  
2. [Getting Started](#getting-started)  
   - [Prerequisites](#prerequisites)  
   - [Build Instructions](#build-instructions)  
   - [Running the Application](#running-the-application)  
3. [Using the Application](#using-the-application)  
   - [Loading a Strategy Configuration File](#loading-a-strategy-configuration-file-json)  
   - [Loading Historical Data](#loading-historical-data-csv)  
   - [Viewing Results](#viewing-results)  
4. [Sample Files](#sample-files)  
5. [Trade Record Details](#trade-record-details)  
6. [Summary Window](#summary-window)  
7. [Project Structure](#project-structure)  
8. [Contributing](#contributing)  
9. [License](#license)

---

## Features

- **Strategy Backtesting**: Simulate how a trading strategy would have performed on historical data.  
- **Web-Based GUI**: Powered by **Dear ImGui** and **ImPlot**, compiled to WebAssembly for browser usage.  
- **Candlestick & Volume Charts**: Visualize market data with candlestick charts and volume bars.  
- **Trade Tracking**: Each executed trade is recorded and can be viewed in a table (trading journal).  
- **Performance Summary**: Key performance metrics (e.g., total profit/loss, winning trades, etc.) are shown.  
- **Configurable Strategies**: Use JSON files to define signals (entry, exit) and parameters such as stop loss and take profit rules.  

---

## Getting Started

### Prerequisites

1. **C++23 or later**: Required by the C++ codebase.  
2. **CMake**: Used to configure and build the project.  
3. **Emscripten SDK**: To compile and run in a web environment. Follow the [Emscripten Getting Started Guide](https://emscripten.org/docs/getting_started/index.html) to install and activate it.

### Build Instructions

1. **Clone the repository**:
   ```bash
   git clone /janucaria/pludux.git
   cd pludux
   # Make sure you are on the correct branch (if needed):
   git checkout main
   ```

2. **Configure with Emscripten**:
   ```bash
   emcmake cmake -B build
   ```

3. **Build**:
   ```bash
   cmake --build build
   ```

4. **Output**:  
   - The build process will create the **Wasm** and **HTML** files in the `build` directory (or wherever your build directory is).  
   - You can serve these files locally (e.g., using `emrun`) to run the web application in your browser.

### Running the Application

- **Local Host**:
  ```bash
  emrun --no_browser --port 8080 build/apps/backtest/gui/
  ```
  Then open `http://localhost:8080` in your web browser.

- Alternatively, you can use any simple HTTP server (like Python’s `http.server`) pointed to the build output directory and load the HTML file in your browser.

---

## Using the Application

Once the application is running in your browser, you will see a Dear ImGui-based interface. The primary menu items for backtesting are:

1. **File → Open Strategy (JSON)**  
2. **File → Open Data (CSV)**  

### Loading a Strategy Configuration File (JSON)

1. Click **File** → **Open Strategy (JSON)**.  
2. Select your strategy configuration file (e.g., `macd.json`).  
3. The application will parse the JSON to apply your strategy’s parameters:  
   - **Entry signal** logic  
   - **Exit signal** logic  
   - **Stop loss** threshold  
   - **Take profit** target  

### Loading Historical Data (CSV)

1. Click **File** → **Open Data (CSV)**.  
2. Select your historical data file (e.g., `BTC-USD.csv`).  
3. The application will parse each row, and then run the backtest with the previously loaded strategy.

### Viewing Results

- **Candlestick & Volume**: A chart displays price movements over time with corresponding volume bars.  
- **Trade Journal**: Shows each executed trade (entry and exit) in a table.  
- **Summary Window**: Provides metrics such as total profit, number of trades, average profit/loss, etc.

---

## Sample Files

Example configuration and historical data files are available under the [data/backtest/samples](data/backtest/samples) folder:

- **Strategies**:  
  - [MACD](data/backtest/samples/macd.json)  
  - [RSI](data/backtest/samples/rsi.json)  
  - [Bollinger Bands](data/backtest/samples/bollinger_bands.json)  

- **Historical Data**:  
  - [random_sample.csv](data/backtest/samples/random_sample.csv)

Feel free to modify these files or create your own.

---

## Trade Record Details

Each trade record contains crucial information about the simulation’s executed trades. Fields include:

- **Entry/Exit Time**  
- **Entry/Exit Price**  
- **Stop Loss / Take Profit Levels** 
- **Profit/Loss**  

These records are displayed in the application’s trading table, giving you a clear look at how your strategy performed on each trade.

---

## Summary Window

The application also provides a **Backtesting Summary** where you can see high-level metrics of your strategy’s performance. Metrics may include:

- Total Net Profit  
- Number of Trades  
- Percentage of Winning Trades  
- Average Win/Loss per Trade  
- Other performance indicators

---

## Project Structure

A simplified overview of the relevant directories:

```
pludux/
├── apps/
│   └── backtest/
│       ├── gui/
│       │   ├── src/
│       │   │   └── windows/          # ImGui window implementations
│       │   └── ...                   # Build logic & other source files
│       └── lib/
│           └── include/
│               └── pludux/backtest/  # Core backtest logic, trade_record.hpp, etc.
├── data/
│   └── backtest/
│       └── samples/                  # Sample strategy JSON & CSV data
└── CMakeLists.txt
```

- **apps/backtest/gui**: Contains the ImGui-based GUI front end.  
- **apps/backtest/lib**: Core backtesting logic and data structures.  
- **data/backtest/samples**: Sample strategy files (JSON) and sample historical data (CSV).

---

## Contributing

Contributions are welcome! If you find bugs, have features to propose, or want to improve documentation:

1. Fork the repository.  
2. Create a new feature branch (`git checkout -b feature/my-feature`).  
3. Commit your changes.  
4. Push to your fork and submit a pull request.  

Please ensure your contributions align with the project’s coding style and include appropriate tests or examples.

---

## License

Distributed under the **MIT License**. See [LICENSE.txt](LICENSE.txt) for more information.

---

**Happy Backtesting!**  
If you have questions or suggestions, feel free to open an issue or reach out. Enjoy exploring and testing your trading strategies with Pludux!

---

Thank you ChatGPT for this README :')
