# Getting Started

This page gives you a practical first backtest path in the Pludux GUI.

## Install and use Pludux Web offline

Open the hosted release online in a browser with WebGPU support. In Chrome or
Edge, use the browser's install option to add Pludux as an app. The installed
app opens in its own window.

Wait for **Offline ready** before going offline. You can then launch Pludux from
its app icon or reopen the same URL in the same browser profile. You can open
local `.pludux` workspaces, import local CSVs, run backtests, and save workspaces
while offline. Use **Save** before closing and **Open** to resume from the saved
file.

If browser site data is cleared, revisit the hosted app online and wait for
**Offline ready** again. When a new release is available, close all open Pludux
tabs and installed app windows to let the update activate.

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

### 3) Create a Strategy

Open **Strategies** and create a new Strategy, or import one from JSON. Strategy JSON
import/export is the documented exchange format for Strategies. See
[Strategy JSON](strategy-json.md) for the identifier convention and schema reference.

For a first test, keep the logic simple. A Strategy can be built from the node
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

### 5) Create a System

Open **Systems**, create a System, and select its Watchlist and Profile.
In the Main setup, choose the Strategy you created. The setup uses its
declared input defaults unless you override them here. Configure the Entry
Filter if you want to screen fresh initial orders.

For a first run, the Main setup is sufficient. To add a Failsafe, select its
Strategy, input overrides, Entry Filter, and activation rule. Every setup in the
System uses the same Profile and Strategy Performance calculation settings.

### 6) Set Market and Broker

Open **Markets** and **Brokers** if you need venue-specific rules or execution
provider fees for the Portfolio.

### 7) Add the System to a Portfolio

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

### 8) Run and inspect

Run the Portfolio, then choose the scope in **Portfolios**. Select a
Single-Asset child to inspect one Strategy setup within a System–Asset run.
Its parent Portfolio is selected automatically. Expanding a Portfolio row
activates Single-Asset scope, restoring its last selected child or choosing the
first valid child when no valid selection is remembered. Collapsing the row
activates Portfolio scope and remembers the child for reopening. Clicking a
Portfolio row uses its expanded or collapsed state to choose the context.
Selecting a child keeps Single-Asset scope active. A Portfolio without a valid
child uses Portfolio scope even while expanded.

- **Chart** shows Equity and Drawdown in Portfolio scope. In Single-Asset scope,
  the top plot offers Portfolio Equity and Drawdown alongside Net P&L and P&L
  Drawdown for the selected setup.
- **Trades** shows every child's trades in Portfolio scope, with open trades
  first and closed trades ordered by close time, newest first. Single-Asset
  scope shows the selected child's trades with their existing ordering.
- **Events** shows all Portfolio execution events or the selected child's
  events, newest first. Events sharing a timestamp retain their recorded
  execution order.
- **Overview** shows the Portfolio summary or performance attributed to the
  selected Single-Asset.

Single-Asset Overview includes realized net P&L, open unrealized P&L, and trade
statistics. Realized net P&L includes partial exits from positions that remain
open. Its two return measures use completed trades, including their partial
exits and Broker fees.

- **Return on deployed capital** divides completed-trade net P&L by those
  trades' cumulative entry investment, including pyramid entries and entry fees.
- **Contribution to Portfolio return** divides completed-trade net P&L by
  Portfolio initial capital.

Ratios with zero denominators display “—”. Account equity and drawdown appear
in Portfolio Overview.

## Practical notes

- If a label or field is not listed here, use the current GUI prompt.
- Strategy inputs come from the selected Strategy and can be overridden in each
  System setup.
- Portfolio backtests share capital and process the union of asset timelines.

## See also

- [Core Concepts](concepts.md)
- [CSV Data](data-csv.md)
- [GUI Guide](gui-guide.md)
- [Portfolio Backtesting](portfolio-backtesting.md)
- [Resources and Workflows](resources-and-workflows.md)
- [Node and Method Reference](nodes.md)
