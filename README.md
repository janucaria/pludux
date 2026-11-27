# Pludux

[![License](https://img.shields.io/badge/license-AGPL-blue.svg)](LICENSE.txt)

**Pludux** is a free and open-source desktop GUI application for deterministic
portfolio simulation.

Start here:

- [Documentation home](docs/index.md)
- [Quick start](docs/getting-started.md)

## Overview

Pludux uses the following resources and results.

- **Backtest**: an actual System × Asset simulation run and its result
- **Portfolio**: shared-capital account that runs ordered Systems
- **Market**: a trading venue or market context (for example, IDX, NASDAQ,
  forex, or crypto) that holds venue-specific rules such as minimum order
  quantity and quantity step
- **Broker**: the broker or execution provider used for the Market, including
  its fee and execution-cost rules
- A **Strategy** defines indicators, entry and exit rules, plots, and inputs
  with default values.
- A **Profile** defines position sizing, drawdown adjustment, and
  insufficient-cash handling.
- A **System** selects a Watchlist and one Profile for its Main setup and
  ordered alternate setups. Each setup selects a Strategy, supplies input
  overrides, and has its own Entry Filter. Each alternate also has an
  activation rule.
- **Watchlist**: an ordered set of Assets used by a System
- **Asset**: market data and field mapping
- **Calls**: a library of trade-call tables. Systems match their rows to
  Watchlist candles for Strategy entry and position rules.

## Verified capabilities

- Management of Portfolios, Markets, Brokers, Systems, Profiles, Strategies,
  Watchlists, and Assets in the GUI
- Strategy creation and editing in a node editor, duplication, and JSON import/export
- System editing with Watchlist and Profile selection, Main and alternate setups,
  Strategy Inputs, Entry Filters, and shared Strategy Performance settings
- Backtesting trade-call tables with Strategy sizing and exits under shared
  Portfolio capital, with CSV import available in Calls
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
3. Use the complete [Node and Method Reference](docs/nodes.md) while building a
   Strategy or comparator.
4. For simulation behavior, read [Execution and Position Lifecycle](docs/execution.md)
   and [Portfolio Backtesting](docs/portfolio-backtesting.md).
5. To backtest trade calls, follow the [worked Calls example](docs/data-csv.md#backtest-calls-with-two-stop-alternatives).

## Contributing

Feedback, ideas, and contributions are welcome. Contributions should align with
the project style and include appropriate tests or examples.
See [Building and Contributing](docs/development.md) for presets, architecture,
tests, and the domain-change checklist.

---

## License

Pludux is released under the **AGPL license**. See [LICENSE.txt](LICENSE.txt) for
more information.

Happy backtesting!
