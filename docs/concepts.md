# Core Concepts

## Repository terms

- **Portfolio**: a shared-capital account that owns ordered Systems and runs
  the simulation.
- **Backtest**: an actual System × Asset simulation run and its result.
- A **System** selects a Watchlist and one Profile and configures a Main setup
  and ordered Failsafes. Each setup selects a Strategy and owns its input
  overrides and Entry Filter. Each Failsafe also has an activation rule.
- A **Strategy** defines trading rules, named series, indicators, entries,
  exits, plots, and input defaults.
- **Profile**: a reusable sizing and capital policy. It defines position sizing,
  drawdown adjustment, and insufficient-cash handling.
- **Watchlist**: an ordered set of Assets used by a System.
- **Asset**: market data and its field mapping.
- **Market**: a trading venue or market context (for example, IDX, NASDAQ,
  forex, or crypto) that holds venue-specific rules such as minimum order
  quantity and quantity step.
- **Broker**: the broker or execution provider for the Market, including its
  fee and execution-cost rules.

Editing a Strategy changes its rules everywhere it is used. To try another
lookback or Entry Filter, edit the setup in its System. A System may use the
same Strategy in more than one setup, with different settings in each.
Deleting a referenced Strategy leaves the setup incomplete until repaired.

## How the pieces connect

The GUI separates setup from simulation:

1. Asset data is loaded first.
2. Watchlists group Assets.
3. Strategies define trading rules.
4. Profiles define reusable sizing and capital policies through position sizing,
   drawdown adjustment, and insufficient-cash handling.
5. Systems select a Watchlist and Profile, configure the Main and Failsafe
   setups, and select the Strategy Performance calculation settings.
6. Markets and Brokers define venue and execution rules.
7. Portfolios run one or more Systems under shared capital.

The Portfolio decides whether the shared account can execute a request. The
Strategy decides what it wants to request.

You can name Profiles for the policy they represent, such as **Conservative**,
**Moderate**, or **Aggressive**. These are user-defined configurations, not
built-in presets. A Profile does not define Strategy signals, Market rules, or
Broker fees.

## Verified GUI structure

The main dockspace opens these windows: Assets, Watchlists, Strategies, Profiles,
Systems, Markets, Brokers, Portfolios, Chart, Trades, Events, and Overview.

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
