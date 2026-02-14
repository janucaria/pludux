# Pludux

[![License](https://img.shields.io/badge/license-AGPL-blue.svg)](LICENSE.txt)  
**Pludux** is a free and open-source backtesting application built around a clean, modular architecture.

It allows traders to simulate single-asset strategies using historical data, configurable risk rules, exchange constraints, and broker fees — all through a structured GUI workflow.

Pludux is currently in its early public phase, focused on providing a stable and extensible foundation. The core architecture is solid, while more advanced features will evolve over time.

---

## ✨ Design Philosophy

Pludux separates a backtest into independent rulesets:

- **Asset** — historical data source  
- **Strategy** — trading logic (GUI-based, JSON import/export)  
- **Profile** — capital and risk configuration  
- **Market** — exchange constraints (minimum quantity, quantity step)  
- **Broker** — execution costs (fees)

This modular structure keeps the engine predictable, transparent, and extensible.

Instead of hardcoding everything inside a single engine layer, each component can evolve independently.

---

## 🚀 Current Capabilities

### Strategy
- GUI-based strategy creation and editing
- Import/export strategies as JSON
- Long/short configuration
- Entry and exit conditions
- Stop Loss (SL)
- Take Profit (TP)
- Trailing Stop Loss (TSL)
- Risk distance modes:
  - ATR-based
  - Percentage-based
  - Fixed value

### Market Rules
- Minimum quantity size
- Quantity step size

### Broker Rules
- Fee simulation

### Data
- Load historical OHLCV data from CSV
- Use additional CSV fields within strategy logic

### Reporting & Visualization
- Trade journal
- Summary metrics
- Candlestick chart with entry/exit markers
- Equity curve visualization

---

## 📌 Scope

Pludux currently focuses on:

- Single-asset backtesting  
- Deterministic bar-based execution  
- Clear trade lifecycle management  

More advanced simulation features — such as portfolio-level backtesting, pyramiding, intrabar modeling, and slippage simulation — are part of the long-term roadmap.

The current direction prioritizes stability and clean architecture.

---

## 🛣 Roadmap Direction

Pludux is designed as a foundation-first project. Future iterations aim to expand:

- Advanced exit management
- Position scaling and pyramiding
- Enhanced broker models
- Portfolio engine
- Strategy editor improvements
- Data integrations

The architecture is intentionally modular to support these extensions cleanly.

---

## 🤝 Contributing

Feedback, ideas, and contributions are welcome.

Please ensure your contributions align with the project’s coding style and include appropriate tests or examples.

Pludux is evolving — and the architecture is designed to grow thoughtfully rather than quickly.

---

## 📜 License

Pludux is released under the **AGPL license**. See [LICENSE.txt](LICENSE.txt) for more information.

---

**Happy Backtesting!** Enjoy exploring and testing your trading strategies with Pludux!

