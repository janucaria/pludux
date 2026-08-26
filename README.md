# Pludux

[![License](https://img.shields.io/badge/license-AGPL-blue.svg)](LICENSE.txt)

**Pludux** is a free and open-source desktop GUI application for deterministic
portfolio simulation.

Start here:

- [Documentation home](docs/index.md)
- [Quick start](docs/getting-started.md)

## Overview

Pludux separates simulation into these repository terms:


- **Backtest**: an actual System × Asset simulation run and its result
- **Portfolio**: shared-capital account that runs ordered Systems
- **Market**: a trading venue or market context (for example, IDX, NASDAQ,
  forex, or crypto) that holds venue-specific rules such as minimum order
  quantity and quantity step
- **Broker**: the broker or execution provider used for the Market, including
  its fee and execution-cost rules
- **System**: a reusable Watchlist plus one Main Strategy, ordered Failsafe
  Strategy bindings and activation rules, and one shared Model Performance
  configuration
- **Strategy**: a named, reusable stored configuration of one Model, one
  Profile, model input overrides, and an Entry Filter
- **Profile**: sizing and capital policy for position sizing, drawdown
  adjustment, and insufficient-cash handling
- **Model**: deterministic trading rules, including indicators, conditions,
  entries/exits, plots, and inputs
- **Watchlist**: an ordered set of Assets used by a System
- **Asset**: market data and field mapping

## Verified capabilities

- GUI-based Portfolio, Market, Broker, System, Strategy, Profile, Model, Watchlist, 
  and Asset management
- Model creation/editing in the GUI, including node-based editing,
  duplicate, and JSON import/export
- Strategy creation/editing in the GUI, including duplicate-and-reuse
  workflows
- System creation/editing in the GUI, including Watchlist binding, a Main
  Strategy, ordered Failsafe Strategy bindings, activation rules, and shared
  Model Performance settings
- Portfolio simulations with shared capital across ordered Systems
- Deterministic execution for identical inputs
- Union processing of asset timelines in Portfolio backtests
- Missing candles are not synthesized
- Last-known prices are used only for valuation where the design permits it
- Market venue-rule handling and Broker fee simulation
- Summary metrics, charting, and Trades view in the desktop UI

## Scope and non-goals

- Focus: portfolio simulation with shared capital, ordered Systems, and
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
