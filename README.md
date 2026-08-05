# Pludux

[![License](https://img.shields.io/badge/license-AGPL-blue.svg)](LICENSE.txt)

**Pludux** is a free and open-source portfolio backtesting application built
around a clean, modular architecture.

It combines reusable Asset + Strategy + Profile Backtest configurations in a
Portfolio with shared capital, market constraints, broker fees, and risk rules.

Pludux is currently in its early public phase, focused on providing a stable and
extensible foundation. The core architecture is solid, while more advanced
features will evolve over time.

---

## Design Philosophy

Pludux separates portfolio simulation into independent rulesets:

- **Asset** - historical data source
- **Strategy** - trading logic created in the GUI or imported from JSON
- **Profile** - backtest position sizing and execution filtering
- **Backtest** - reusable Asset + Strategy + Profile configuration
- **Portfolio** - shared capital and ordered Backtests
- **Market** - shared exchange constraints
- **Broker** - shared execution costs

This modular structure keeps the engine predictable, transparent, and
extensible. Each backtest can evolve without hardcoding every concern into one
engine layer.

---

## Current Capabilities

### Portfolio

- Shared capital across ordered Backtests
- Portfolio-level drawdown size adjustment
- Deterministic insufficient-cash handling
- Union timelines for assets with different calendars
- Last-known-price valuation without synthetic candles

### Strategy

- GUI-based strategy creation and editing
- Import and export strategies as JSON
- Long and short configuration
- Entry, pyramiding, and partial exit conditions
- Stop loss, take profit, and trailing stop loss
- ATR, percentage, and fixed-value risk distances

### Market and Broker Rules

- Minimum quantity and quantity-step constraints
- Percentage-notional and fixed fee simulation

### Data

- Historical OHLCV data loaded from CSV
- Additional CSV fields available to strategy logic

### Reporting and Visualization

- Trade journal for the selected Backtest
- Portfolio and backtest summary metrics
- Candlestick charts with entry and exit markers
- Portfolio equity and drawdown visualization

---

## Scope

Pludux currently focuses on:

- Portfolio backtesting with shared capital
- Reusable per-asset Backtest configurations
- Deterministic bar-based execution
- Clear trade lifecycle management

Portfolio backtests may have different timestamps. Pludux processes their
union timeline without manufacturing candles and carries last-known prices only
for valuation. See [Portfolio Backtesting](docs/portfolio-backtesting.md).

---

## Roadmap Direction

Future iterations aim to expand:

- Advanced portfolio allocation and exposure policies
- Enhanced broker and market models
- Slippage and deeper intrabar simulation
- Strategy editor improvements
- Data integrations

---

## Contributing

Feedback, ideas, and contributions are welcome. Contributions should align with
the project style and include appropriate tests or examples.

---

## License

Pludux is released under the **AGPL license**. See [LICENSE.txt](LICENSE.txt) for
more information.

Happy backtesting!
