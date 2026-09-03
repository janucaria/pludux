# GUI Guide

Pludux uses a docked desktop layout.

## Main windows

The dockspace opens these windows:

- Assets
- Watchlists
- Models
- Profiles
- Strategies
- Systems
- Markets
- Brokers
- Portfolios
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

## Models window

Use this window to create Models, edit them, duplicate them, and import or
export JSON. The UI also exposes built-in models and a node editor for
numeric expressions, conditions, named series, and portfolio comparators.

## Watchlists window

Create, rename, duplicate, and delete Watchlists, then maintain their ordered
Asset members. Asset order is observable: it is the final tie-break within a
System when Portfolio comparator keys are equal.

## Profiles window

Use this window to create reusable risk and capital-management policies. A
Profile can be named for a user-defined policy, such as **Conservative**,
**Moderate**, or **Aggressive**; these are not built-in presets. The editor
starts with risk-distance sizing and is where drawdown adjustment and
insufficient-cash settings are configured. Entry Filters are configured in the
Strategies window, not in Profiles.

## Strategies window

Combine one Model and one Profile, override declared Model inputs, and build an
optional Entry Filter for fresh initial Requested Orders. Duplicate a Strategy
before making a variant used by only some Systems.

## Systems window

Use this window to bind a Watchlist, a Main Strategy, optional Failsafe Strategy
bindings, and shared Model Performance settings.

The System editor shows:

- Watchlist
- Main Strategy
- Failsafe Strategy bindings
- Model
- Model Inputs
- Profile
- Entry Filter
- Shared Model Performance Calculation

Use the Strategies window to create, edit, duplicate, and inspect reusable
Strategy resources. Editing a Strategy updates every System that references it.
If you need a variant, duplicate the Strategy first.

## Markets and Brokers windows

Markets define minimum quantity and quantity step. Brokers define additive
fixed or percentage fee rules by position scope and trigger. These resources
are selected by Portfolios, not embedded in Strategies. See
[Profiles, Markets, and Brokers](risk-market-broker.md).

## Portfolios window

Use this window to configure shared capital, Market (the trading venue or
market context), Broker (the Market's broker or execution provider), capacity
limits, entry comparators, and ordered Systems.

Entry comparator caveat: Current Close sees completed current-bar OHLCV and
custom DATA. Next Open sees the previous completed bar; use Requested Order
Price when you need the executable current open price. See
[Portfolio Backtesting](portfolio-backtesting.md) for authoritative comparator
behavior.

## Chart, Trades, and Overview

These windows display the selected completed Portfolio run. Chart shows one
expanded System × Asset backtest plus Portfolio overlays. Trades can switch
between actual shared-capital executions and hypothetical Model activity.
Overview summarizes Portfolio results. See [Results and Accounting](results.md).

## Notes

- Model JSON import/export is available in Models; `.pludux` opens and saves
  the complete workspace. Other interchange formats are not claimed here.
- When a label is not explicitly verified above, follow the current GUI prompt.

## Related docs

- [Getting Started](getting-started.md)
- [CSV Data](data-csv.md)
- [Portfolio Backtesting](portfolio-backtesting.md)
- [Resources and Workflows](resources-and-workflows.md)
- [Workspace Persistence](persistence.md)
