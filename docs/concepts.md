# Core Concepts

## Repository terms

- **Asset**: imported market data and its field mapping.
- **Strategy**: trading logic that produces entries, exits, conditions, and
  optional plots.
- **Profile**: a reusable risk and capital-management policy. It defines
  position sizing, drawdown adjustment, and insufficient-cash handling.
- **Backtest**: a reusable Asset + Strategy + Profile configuration. Its setup
  owns the Entry Filter that decides whether a fresh initial Requested Order may
  enter Portfolio ranking.
- **Portfolio**: shared capital plus an ordered list of Backtests.
- **Market**: a trading venue or market context (for example, IDX, NASDAQ,
  forex, or crypto) that holds venue-specific rules such as minimum order
  quantity and quantity step.
- **Broker**: the broker or execution provider for the Market, including its
  fee and execution-cost rules.

## How the pieces connect

The GUI separates setup from execution:

1. Asset data is loaded first.
2. Strategies define trading logic.
3. Profiles define reusable risk and capital-management policies through
   position sizing, drawdown adjustment, and insufficient-cash handling.
4. Backtests bind Asset, Strategy, and Profile together, and their setups
   define Entry Filters.
5. Portfolios run one or more Backtests under shared capital.

The Portfolio decides whether the shared account can execute a request. The
Backtest decides what it wants to request.

You can name Profiles for the policy they represent, such as **Conservative**,
**Moderate**, or **Aggressive**. These are user-defined configurations, not
built-in presets. A Profile does not define Strategy signals, Market rules, or
Broker fees.

## Verified GUI structure

The main dockspace opens these windows: Portfolios, Backtests, Assets,
Watchlists, Strategies, Markets, Brokers, Profiles, Chart, Trades, and Overview.

## Data and execution rules

- Portfolio backtests process the union of asset timelines.
- Missing candles are not synthesized.
- Last-known prices are used only for valuation where the design permits it.
- Strategy Performance is tracked separately from executed portfolio results.

See [Portfolio Backtesting](portfolio-backtesting.md) for the authoritative
portfolio semantics.

## Advanced references

- [Portfolio Backtesting](portfolio-backtesting.md)
- [Turtle Trading Guidelines for Pludux](turtle-trading.md)
- [Strategy Performance Bayesian Kelly](strategy-performance-bayesian-kelly.md)
