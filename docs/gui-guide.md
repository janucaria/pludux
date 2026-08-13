# GUI Guide

Pludux uses a docked desktop layout.

## Main windows

The dockspace opens these windows:

- Portfolios
- Backtests
- Assets
- Watchlists
- Strategies
- Markets
- Brokers
- Profiles
- Chart
- Trades
- Overview

## Assets window

Use this window to add, edit, import, or delete Assets. It supports CSV import
from file paths and text input. The editor includes:

- Asset Details
- CSV Column Mapping
- Price History

The importer expects exact OHLCV column headers.

## Strategies window

Use this window to create Strategies, edit them, duplicate them, and import or
export JSON. The UI also exposes built-in strategies and a node editor for
numeric expressions, conditions, named series, and portfolio comparators.

## Profiles window

Use this window to create reusable risk and capital-management policies. A
Profile can be named for a user-defined policy, such as **Conservative**,
**Moderate**, or **Aggressive**; these are not built-in presets. The editor
starts with risk-distance sizing and is where drawdown adjustment and
insufficient-cash settings are configured. Entry Filters are configured in the
Backtests window, not in Profiles.

## Backtests window

Use this window to bind a Watchlist, a Main setup, optional Failsafe setups, and
shared Strategy Performance settings.

The Backtest editor shows:

- Watchlist
- Strategy
- Strategy Inputs
- Profile
- Entry Filter
- Shared Strategy Performance Calculation

## Portfolios window

Use this window to configure shared capital, Market (the trading venue or
market context), Broker (the Market's broker or execution provider), capacity
limits, entry comparators, and ordered Backtests.

Entry comparator caveat: Current Close sees completed current-bar OHLCV and
custom DATA. Next Open sees the previous completed bar; use Requested Order
Price when you need the executable current open price. See
[Portfolio Backtesting](portfolio-backtesting.md) for authoritative comparator
behavior.

## Notes

- Strategy JSON import/export is verified; other file formats are not claimed
  here.
- When a label is not explicitly verified above, follow the current GUI prompt.

## Related docs

- [Getting Started](getting-started.md)
- [CSV Data](data-csv.md)
- [Portfolio Backtesting](portfolio-backtesting.md)
