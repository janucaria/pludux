# GUI Guide

Pludux uses a docked desktop layout.

## Main windows

The dockspace opens these windows:

- Assets
- Calls
- Watchlists
- Strategies
- Profiles
- Systems
- Markets
- Brokers
- Portfolios
- Chart
- Trades
- Events
- Overview

## Assets window

Use this window to add, edit, import, or delete Assets. It supports CSV import
from file paths and text input. The editor includes:

- Asset Details
- CSV Column Mapping
- Price History

The importer expects exact OHLCV column headers.

## Calls window

Use **Calls** to preview, rename, or delete tables in the trade-call library.
CSV import can add a table or replace a table's contents. The workspace embeds
each table. Select a table in **Systems** to map its columns and use it in a
Portfolio. [Calls resource and mapping rules](resources-and-workflows.md#calls)

## Strategies window

Use this window to create Strategies, edit them, duplicate them, and import or
export JSON. The UI also exposes built-in Strategies and a node editor for
numeric expressions, conditions, named series, and portfolio comparators.

For each enabled side, set **Entry Source** to **Signal** or **Call**.
Use the **Call** node in Strategy expressions to read a logical field from the active
call row. For example, the same Strategy can use different columns for risk
distance in different System setups. [Call row values](nodes.md#call-row-values)

## Watchlists window

Create, rename, duplicate, and delete Watchlists, then maintain their ordered
Asset members. Asset order is observable: it is the final tie-break within a
System when Portfolio comparator keys are equal.

## Profiles window

Use this window to create reusable risk and capital-management policies. A
Profile can be named for a user-defined policy, such as **Conservative**,
**Moderate**, or **Aggressive**; these are not built-in presets. The editor
starts with risk-distance sizing and is where drawdown adjustment and
insufficient-cash settings are configured. Select one Profile for each System.

## Systems window

Select the Watchlist and Profile once for the entire System. In the Main setup,
select a Strategy and configure its Strategy Inputs and Entry Filter. Add any
Failsafes in evaluation order, each with its own Strategy, input overrides,
Entry Filter, and activation rule.

The System's Strategy Performance calculation settings apply to all its setups.
Each setup tracks its own theoretical history for each Asset. To edit the
underlying rules shared by its users, open the selected Strategy in **Strategies**.

To use Calls, select a table and map its **Datetime** and **Asset** columns.
Map **Direction** to specify Long or Short per row. Unmapped or blank
directions default to Long. **Entry Low** and **Entry High** are optional bounds.
Map any logical fields used by Call nodes separately for each Main or Failsafe
setup. **Entry Priority** selects **Call first** or **Signal first** when one
setup offers opposing entries at the same open. [Worked example](data-csv.md#backtest-calls-with-two-stop-alternatives)

## Markets and Brokers windows

Markets define minimum quantity and quantity step. Brokers define additive
fixed or percentage fee rules by position scope and trigger. These resources
are selected by Portfolios, not embedded in Strategies. See
[Profiles, Markets, and Brokers](risk-market-broker.md).

## Portfolios window

Use this window to configure shared capital, Market (the trading venue or
market context), Broker (the Market's broker or execution provider), capacity
limits, entry comparators, and ordered Systems.

Entry comparators may use Requested Order values, phase-visible market data,
and Strategy Performance from each requesting Strategy setup's immutable
request-discovery snapshot. This applies to initial and pyramiding requests;
named series and indicators are not available. Current Close sees completed
current-bar OHLCV and custom DATA. Next Open sees the previous completed bar;
use Requested Order Price when you need the executable current open price.
Performance may already influence Bayesian or Kelly sizing, so using it for
ranking can give the same evidence additional weight. See
[Portfolio Backtesting](portfolio-backtesting.md) for authoritative comparator
behavior.

## Chart, Trades, Events, and Overview

These windows share the scope selected in **Portfolios**. Selecting a
Single-Asset child selects its parent Portfolio and that Strategy setup within
a System–Asset run. Expanded Portfolio rows use Single-Asset scope, restoring
the last selected child or choosing the first valid child when necessary.
Collapsed rows use Portfolio scope and retain their child selection for
reopening. Clicking a row selects that Portfolio with
the context determined by its expansion state. If no valid child exists, the
Portfolio uses Portfolio scope.

Portfolio scope shows equity and drawdown in Chart, aggregate Trades and Events,
and the Portfolio Overview. Portfolio Trades places open trades first, followed
by closed trades in descending close-time order. Events uses newest-first time
order and retains execution order within equal timestamps.

The Single-Asset Chart's top plot selector offers Equity and Drawdown for the
Portfolio, and Net P&L and P&L Drawdown for the selected setup. The Net P&L legend
shows the amount and the percentage of Portfolio initial capital. P&L Drawdown
shows the drop from the running peak of that percentage.
Overview shows that child's actual trade performance, including net P&L,
trade statistics, return on deployed capital, and contribution to Portfolio
return. Trades can switch between actual shared-capital executions and
hypothetical Strategy activity in both scopes. See
[Getting Started](getting-started.md#8-run-and-inspect) for return definitions
and [Results and Accounting](results.md) for accounting details.

## Notes

- Strategy JSON import/export is available in Strategies; `.pludux` opens and saves
  the complete workspace. Other interchange formats are not claimed here.
- When a label is not explicitly verified above, follow the current GUI prompt.

## Related docs

- [Getting Started](getting-started.md)
- [CSV Data](data-csv.md)
- [Portfolio Backtesting](portfolio-backtesting.md)
- [Resources and Workflows](resources-and-workflows.md)
- [Workspace Persistence](persistence.md)
