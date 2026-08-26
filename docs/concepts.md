# Core Concepts

## Repository terms

- **Portfolio**: a shared-capital account that owns ordered Systems and runs
  the simulation.
- **Backtest**: an actual System × Asset simulation run and its result.
- **System**: a reusable Watchlist plus one Main Strategy, ordered Failsafe
  Strategy bindings and activation rules, and one shared Model Performance
  configuration.
- **Strategy**: a named, reusable stored resource made of one Model, one
  Profile, model input overrides, and an Entry Filter.
- **Model**: deterministic trading rules that produce indicators, conditions,
  entries, exits, plots, and inputs. It is not necessarily forecasting or
  machine learning.
- **Profile**: a reusable sizing and capital policy. It defines position sizing,
  drawdown adjustment, and insufficient-cash handling.
- **Watchlist**: an ordered set of Assets used by a System.
- **Asset**: market data and its field mapping.
- **Market**: a trading venue or market context (for example, IDX, NASDAQ,
  forex, or crypto) that holds venue-specific rules such as minimum order
  quantity and quantity step.
- **Broker**: the broker or execution provider for the Market, including its
  fee and execution-cost rules.

Strategies are first-class reusable stored resources. Editing a Strategy affects
every System that references it. Duplicate a Strategy when you need a variant.
If a referenced Strategy is deleted, the owning System becomes incomplete until
the reference is fixed.

## How the pieces connect

The GUI separates setup from simulation:

1. Asset data is loaded first.
2. Watchlists group Assets.
3. Models define trading rules.
4. Profiles define reusable sizing and capital policies through position sizing,
   drawdown adjustment, and insufficient-cash handling.
5. Strategies combine a Model and a Profile.
6. Systems bind Watchlists with one Main Strategy, ordered Failsafe Strategy
   bindings, activation rules, and shared Model Performance settings.
7. Markets and Brokers define venue and execution rules.
8. Portfolios run one or more Systems under shared capital.

The Portfolio decides whether the shared account can execute a request. The
Strategy decides what it wants to request.

You can name Profiles for the policy they represent, such as **Conservative**,
**Moderate**, or **Aggressive**. These are user-defined configurations, not
built-in presets. A Profile does not define Model signals, Market rules, or
Broker fees.

## Verified GUI structure

The main dockspace opens these windows: Assets, Watchlists, Models, Profiles,
Strategies, Systems, Markets, Brokers, Portfolios, Backtests, Chart, Trades,
and Overview.

## Data and execution rules

- Portfolio backtests process the union of asset timelines.
- Missing candles are not synthesized.
- Last-known prices are used only for valuation where the design permits it.
- Model Performance is tracked separately from executed portfolio results.

See [Portfolio Backtesting](portfolio-backtesting.md) for the authoritative
portfolio semantics.

## Advanced references

- [Portfolio Backtesting](portfolio-backtesting.md)
- [Turtle Trading Guidelines for Pludux](turtle-trading.md)
- [Model Performance Bayesian Kelly](strategy-performance-bayesian-kelly.md)
