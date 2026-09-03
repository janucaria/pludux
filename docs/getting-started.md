# Getting Started

This page gives you a practical first backtest path in the Pludux GUI.

## First backtest workflow

### 1) Import an Asset

Open **Assets** and add or import a CSV file. The importer uses the file name
as the Asset name when one is not provided. The editor exposes fields for the
Asset name and the OHLCV column mapping.

Verified field labels in the Asset editor include:

- Name
- Open price
- High price
- Low price
- Close price
- Volume

The CSV loader reads the first column as the date/time field and stores the
remaining columns as numeric data. If the loaded data is in reverse
chronological order, Pludux reverses it. It does not repair arbitrary
out-of-order timestamps, and portfolio timelines require finite, strictly
increasing timestamps.

### 2) Create a Watchlist

Open **Watchlists** and create a Watchlist that contains the Assets you want to
test together. The exact item labels follow the current GUI prompt.

### 3) Create a Model

Open **Models** and create a new Model, or import one from JSON. Model JSON
import/export is the documented exchange format for models. See
[Model JSON](model-json.md) for the identifier convention and schema reference.

For a first test, keep the logic simple. A Model can be built from the node
editor and can use named series, conditions, and plots.

### 4) Create a Profile

Open **Profiles** and create a Profile. A Profile is a reusable risk and
capital-management policy. The editor defaults to a risk-based position sizing
setup, which matches the UI's initial behavior.

Profiles control position sizing, drawdown adjustment, and insufficient-cash
handling. You can name your own policies **Conservative**, **Moderate**, or
**Aggressive**; these are not built-in presets. Profiles do not create trading
signals on their own. Configure an **Entry Filter** in the Strategy setup when
you need to decide whether a fresh initial Requested Order may enter Portfolio
ranking.

### 5) Create a Strategy

Open **Strategies** and create a reusable Strategy. The editor uses these
fields:

- Name
- Model
- Profile
- Model Inputs
- Entry Filter

Duplicate the Strategy if you want a variant without changing Systems that
already reference it.

### 6) Create a System

Open **Systems** and create a System. The System editor uses these fields:

- Name
- Watchlist
- Main Strategy
- Failsafe Strategies with activation rules
- Shared Model Performance calculation

For the first run, use one Asset in the Watchlist, one Model, one Profile, and
one Strategy.
Leave Failsafes out unless you need them.

### 7) Set Market and Broker

Open **Markets** and **Brokers** if you need venue-specific rules or execution
provider fees for the Portfolio.

### 8) Add the System to a Portfolio

Open **Portfolios** and create a Portfolio. The Portfolio editor includes:

- Name
- Initial capital
- Market
- Broker
- Maximum open trades
- Maximum combined layers
- Entry comparators
- Ordered Systems

Add your System to the Portfolio and keep the order stable.

### 9) Run and inspect

Run the Portfolio, then inspect:

- **Chart** for candles, positions, and markers
- **Trades** for actual and hypothetical trade lists
- **Overview** for summary metrics

## Practical notes

- If a label or field is not listed here, use the current GUI prompt.
- Model inputs come from the selected Model and can be overridden in the
  Strategy.
- Portfolio backtests share capital and process the union of asset timelines.

## See also

- [Core Concepts](concepts.md)
- [CSV Data](data-csv.md)
- [GUI Guide](gui-guide.md)
- [Portfolio Backtesting](portfolio-backtesting.md)
- [Resources and Workflows](resources-and-workflows.md)
- [Node and Method Reference](nodes.md)
