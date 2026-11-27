# Portfolio Backtesting

For a complete multi-market configuration example, see
[Turtle Trading Guidelines for Pludux](turtle-trading.md).

## Terminology

- A Strategy defines deterministic entry and exit rules, indicators, and
  input defaults. Systems may reference the same Strategy more than once.
- A System selects one Watchlist and one Profile. Its Main setup and ordered
  alternate setups each reference a Strategy and own their input overrides and
  Entry Filter. Each alternate also has an activation rule. The System shares one
  Strategy Performance calculation policy across its setups.
- A Portfolio owns initial capital, one Market, one Broker, the Intrabar Path,
  aggregate capacity limits, and an ordered collection of Systems. The Market
  is the trading venue or market context and provides venue-specific rules such
  as minimum order quantity and quantity step. The Broker represents the
  Market's broker or execution provider and provides its fee and execution-cost
  rules. Each System's Profile defines drawdown adjustment and the
  insufficient-cash response.
- Running a Portfolio simulates all of its Systems. A single-asset simulation
  is a Portfolio containing one System × Asset Backtest.

The same System configuration may be referenced by different Portfolios.
Runtime positions, indicator results, and Strategy Performance evidence are
isolated within each Portfolio run.

## Ownership

The Strategy defines when to request an entry. The System's Profile sizes it,
the setup's Entry Filter screens it, and the Portfolio arbitrates eligible
requests against shared cash and capacity.

```text
Portfolio
|- Initial capital and account equity
|- Market rules and Broker fees
|- Intrabar Path
|- Maximum open trades and maximum combined layers
|- Entry comparators
`- Ordered Systems
   |- Watchlist
   |- Profile (position sizing, drawdown adjustment, insufficient-cash policy)
   |- Strategy Performance calculation settings
   |- Main setup (Strategy, input overrides, Entry Filter)
   `- Ordered alternate setups (Strategy, input overrides, Entry Filter, activation)
```

Position sizing and Entry Filters read the current Portfolio equity and
drawdown. Each setup maintains its own theoretical Strategy Performance per Asset.
Bayesian or Kelly sizing reads that setup’s history for that Asset in the current
Portfolio run. Shared reservations do not change a Profile's sizing intent;
they constrain the later cash decision.

When an accepted initial order opens a real position, that position permanently
belongs to the requesting setup. Pyramids, partial exits, snapshots, the
final closed trade, and position-related execution events retain that owner even
if a different Strategy is active later. A rejected initial order is
attributed to the setup that requested it.

Sizing methods express different limits:

- Fixed Quantity requests explicit asset units and uses nearest-step Market
  normalization.
- Fixed Budget, Equity Fraction, and Bayesian Kelly cap entry notional plus
  applicable Broker entry fees.
- Risk Distance caps loss at the risk boundary plus estimated entry and
  boundary-exit fees.

## Shared-Capital Arbitration

At a market timestamp, risk-reducing work is processed before risk-increasing
work. Entry and pyramiding Requested Orders that occur in the same phase are
ranked by the Portfolio's ordered comparators. Complete ties resolve by the
Portfolio's ordered System list, then that System's Watchlist Asset order.
Separate Portfolios do not arbitrate with one another.

For a fresh initial entry, Pludux processes the request in this order.

1. Capture the setup's theoretical Strategy Performance snapshot.
2. Evaluate the System Profile’s sizing constraint and drawdown adjustment.
3. Use the shared Market and Broker to create an immutable Requested Order with
   normalized quantity, notional, fees, and risk values.
4. Evaluate the setup's Entry Filter against Strategy Performance, Requested
   Order values, and current account metrics. An entry-filtered Main Strategy
   may allow the next eligible Alternate Strategy to produce its own fresh entry.
5. Rank every accepted Requested Order using the Portfolio comparators.
6. Enforce maximum-open-trade and maximum-combined-layer capacity.
7. Compare the fee-inclusive requested cost with available shared cash.
8. Reject the request or cap it to the largest affordable valid quantity using
   the System Profile's insufficient-cash policy.

Pyramids use the frozen unit size of the actual position. They join the same
Portfolio ranking, capacity, and cash checks as initial orders.

Call entries use the open-price phase and join the same ranking and admission
as other open-price requests. **Entry Priority** applies to opposing entries
from the same setup at the same open. The Portfolio comparators then rank
requests using their regular rules. A Call's sizing and fill use the open
price, with Market quantity rules and Broker fees applied as usual.
[Call entries](execution.md#call-entries)

### Entry comparator data

Portfolio Entry Comparators can combine immutable Requested Order values with
the order's asset OHLCV fields, custom `DATA` fields, and
`STRATEGY_PERFORMANCE.VALUE`. They support numeric constants, basic scalar math,
and lookback. They intentionally do not expose Strategy named series or
technical-indicator nodes: Strategy and Profile logic has already produced the
Requested Order before Portfolio ranking begins.

For each initial or pyramiding request, `STRATEGY_PERFORMANCE.VALUE` reads the
immutable Strategy Performance snapshot captured from the requesting Strategy
binding during request discovery. The snapshot contains only evidence visible
in that execution phase and cannot change while accepted requests are ranked.
Each candidate therefore uses its own Strategy setup's current theoretical
history without introducing look-ahead or ranking-order effects.

Strategy Performance may already affect Bayesian or Kelly position sizing. Using
it again in a comparator is valid, but can intentionally or accidentally give
the same performance evidence additional influence over capital allocation.

Asset data is phase-safe. A Current Close order sees the completed current bar.
A Next Open or Call order sees the previous completed bar, because the current
bar's high, low, close, and volume do not yet exist. Use Requested Order Price
when a comparison needs the executable open price.

A direction-adjusted strength comparator can rank historical price movement in
units of the order's risk distance:

```text
Requested Order Direction
* (Requested Order Price - Lookback(Close, 63))
/ Requested Order Risk Distance
```

Set the comparator to Higher First. Positive values rank favorable movement for
both long and short orders in the same direction-aware list. If an asset lacks
the requested history or data, the score is non-finite and ranks after every
finite score.

Both reductions use the completed step count from current drawdown relative to
peak equity. Notional equity reduction changes only sizing methods that consume
equity; fixed quantity and fixed budget remain fixed unless Size Reduction is
also configured. A Turtle-style setup uses a 10% drawdown step, 0% Size
Reduction, and 20% Notional Equity Reduction.

An accepted order reserves its notional before the next backtest request is
considered. Pyramiding an existing trade does not consume another open-trade
slot, but every successful layer consumes combined-layer capacity. Closing a
trade releases its open-trade slot and all of its combined layers before later
entry phases and backtests are processed.

New Portfolios allow at most 10 open trades by default. The Portfolio editor
accepts values of 1 or greater. The underlying configuration also permits 0,
which rejects every initial entry and can be useful for programmatic scenarios.

## Synchronized Execution

For every timestamp, Pludux executes these phases:

1. Apply open-price exits for every backtest with a real bar.
2. Process open-price entries and pyramids in Portfolio order.
3. Apply the Portfolio's [Intrabar Path](execution.md#intrabar-path) and process
   price exits for each active backtest.
4. Apply close-price signal exits for every active backtest.
5. Process close-price entries and pyramids in Portfolio order.
6. Schedule next-open actions for each backtest's next real bar.
7. Mark all positions to market and append the Portfolio result.

Capital released by an exit is available to later entry phases at the same
timestamp. A pending next-open action waits for that backtest's next real bar,
not merely the next timestamp produced by another asset.

## Union Timeline and Missing Bars

The Portfolio clock is the sorted union of all backtest timestamps. Asset
timestamps must be finite and strictly increasing.

```text
Asset A:   09:00  09:05         09:15
Asset B:   09:00         09:10  09:15
Portfolio: 09:00  09:05  09:10  09:15
```

At `09:05`, Asset A processes its real bar. Asset B does not evaluate its
Strategy and cannot execute entries, exits, stops, or targets. If Asset B has a
position, the Portfolio values it using its last known close and marks that
backtest stale. Before its first price, a backtest is unavailable.

Pludux never manufactures candles to fill missing timestamps.

A Call is evaluated at its Asset's matching candle open. The Calls table adds
no timestamps to the Portfolio clock. A row without a matching candle leaves
the union timeline unchanged. [Calls resource and mapping rules](resources-and-workflows.md#calls)

## Results and Charts

`PortfolioTimeline` records shared capital, available capital, equity, peak
equity, drawdown, realized and unrealized P&L, reserved notional, gross and net
exposure, open-position count, and backtest freshness on the union clock.

Each System retains a sparse `BacktestTimeline` and separate
series-evaluation results for every setup. The chart projects the selected
Setup × Asset row onto a compact axis:

- Every X index represents one real bar from the selected System × Asset run,
  so its candles and volume remain consecutive.
- The Equity and Drawdown top plots sample Portfolio account values from the
  union timeline at the selected backtest's timestamps.
- Net P&L adds the selected setup's cumulative realized P&L, including partial
  exits, to the unrealized P&L of its remaining position. Both include applicable
  Broker fees. The chart divides that total by Portfolio initial capital and
  shows both the amount and percentage in the legend.
- P&L Drawdown subtracts the current Net P&L percentage from its running peak,
  starting with a peak of zero. If initial capital is zero, the percentage
  legends display “—”.
- Portfolio-only timestamps are omitted from this chart view. They remain in
  the stored `PortfolioTimeline` and reappear when a backtest containing those
  timestamps is selected.
- Axis labels and inspection report each backtest bar in the application's
  local/display timezone; labels are not guaranteed to be UTC.
- No candles or portfolio values are synthesized to fill missing data.
- Strategy plots, named series, shadow returns, streaks, and strategy
  performance come from the selected setup.
- Entry, pyramid, rejection, and exit markers are shown only when attributed to
  the selected setup. Position and risk overlays are shown only for positions
  owned by that setup.
- The Trades view filters actual open and closed trades by their permanent setup
  owner. Its Hypothetical source reads only the selected setup's theoretical
  positions.
- The Portfolio Chart shows account Equity and Drawdown on the union timeline.
  Hovering or pinning selects one union timestamp for both legends. Main and
  alternate setups execute together with shared Portfolio capital.

### Events

The **Events** window beside **Trades** lists actual fills and order
rejections for the selected System, Asset, and Strategy in the active Portfolio,
including rejected initial orders that never opened a trade. It follows the same
Portfolios tree selection as Trades. Events appear newest first.
Rows with the same timestamp follow execution order, so you can see which
order used shared cash before another order was rejected.

Each row shows the time, System, Strategy, Asset, event, quantity, and price.
Fills show the buy or sell side and the cause (entry, scale in, scale out,
signal exit, stop loss, or take profit). Rejections show the requested quantity
and price and identify the limit reached (cash, open trades, or combined
layers). A cash rejection also shows the required and available amounts.

The event-type filter defaults to all events within the selected setup. Use it
to inspect, for example, stop-loss fills. A partial exit with a recorded cause
matches both Scale out and its exit cause, with one row in the timeline. Hypothetical Strategy activity is available in the Trades window.

## Portfolio Hierarchy and Chart Selection

The Portfolios window is an expandable tree. Each top-level Portfolio contains
a flat Setup × Asset list. Rows are ordered by Portfolio System order, then
Watchlist asset order, then Main setup followed by its ordered alternate
setups. Each row identifies the Strategy, Asset, setup role, and System Profile.

Selecting a Portfolio opens it and restores that Portfolio's last valid setup
selection. If the remembered setup was removed or is no longer valid, Pludux
selects the first valid row in display order. An empty or incomplete Portfolio
may have no active setup. Each Portfolio remembers its choice independently,
including when two Portfolios reuse the same System.

Selecting a Setup × Asset child selects its parent Portfolio and filters the
Chart, Trades, Events, and Overview to that setup. Candles provide the shared
market context. Portfolio accounting and execution status cover the entire run.
Missing Strategy, Asset, or Profile references appear as disabled rows so you
can identify broken configurations in their original order.

Portfolio search covers both levels. A Portfolio-name match shows all of its
children. Child matching includes Strategy, Asset, setup role, and
Profile names while preserving the original display order.

The Systems editor selection is separate: it chooses which reusable System
configuration is being edited and never changes the Chart. The Portfolios tree
is the only Chart setup selector.

This first portfolio engine intentionally excludes margin, leverage models,
currency conversion, correlation limits, risk parity, and other advanced
allocation systems.
