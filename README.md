# Pludux

[![License](https://img.shields.io/badge/license-AGPL-blue.svg)](LICENSE.txt)

**Pludux** is a free and open-source desktop GUI application for deterministic
portfolio backtesting.

Start here:

- [Documentation home](docs/index.md)
- [Quick start](docs/getting-started.md)

## Overview

Pludux separates simulation into these repository terms:

- **Asset**: imported market data and field mapping
- **Strategy**: trading logic, conditions, optional plots, and the node editor
- **Profile**: reusable risk and capital-management policy for position sizing,
  drawdown adjustment, and insufficient-cash handling
- **Backtest**: reusable Watchlist plus ordered setups; each setup selects a
  Strategy and Profile, can override strategy inputs, and owns the Entry Filter.
  Backtests share one strategy-performance calculation policy.
- **Portfolio**: shared capital plus an ordered list of Backtests
- **Market**: a trading venue or market context (for example, IDX, NASDAQ,
  forex, or crypto) that holds venue-specific rules such as minimum order
  quantity and quantity step
- **Broker**: the broker or execution provider used for the Market, including
  its fee and execution-cost rules

## Verified capabilities

- GUI-based Asset, Strategy, Profile, Backtest, Portfolio, Market, and Broker
  management, plus Watchlists
- Strategy creation/editing in the GUI, including node-based editing,
  duplicate, and JSON import/export
- Portfolio backtests with shared capital across ordered Backtests
- Deterministic execution for identical inputs
- Union processing of asset timelines in Portfolio backtests
- Missing candles are not synthesized
- Last-known prices are used only for valuation where the design permits it
- Market venue-rule handling and Broker fee simulation
- Summary metrics, charting, and Trades view in the desktop UI

## Scope and non-goals

- Focus: portfolio backtesting with shared capital, ordered Backtests, and
  deterministic execution.
- Profile does not define execution filtering or built-in risk presets.
- Not included: live trading, margin, leverage models, currency conversion,
  correlation limits, risk parity, slippage, or deeper intrabar simulation.

See [Core Concepts](docs/concepts.md), [Limitations](docs/limitations.md), and
[Portfolio Backtesting](docs/portfolio-backtesting.md) for current semantics.

## How to start

1. Read [Documentation home](docs/index.md).
2. Follow [Quick start](docs/getting-started.md).
3. For portfolio behavior, read [Portfolio Backtesting](docs/portfolio-backtesting.md).

## Contributing

Feedback, ideas, and contributions are welcome. Contributions should align with
the project style and include appropriate tests or examples.

---

## License

Pludux is released under the **AGPL license**. See [LICENSE.txt](LICENSE.txt) for
more information.

Happy backtesting!
