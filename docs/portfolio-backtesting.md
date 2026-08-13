# Portfolio Backtesting

For a complete multi-market configuration example, see
[Turtle Trading Guidelines for Pludux](turtle-trading.md).

## Terminology

Pludux treats a **Backtest** as a reusable Watchlist plus ordered setup
configuration and a **Portfolio** as the account that executes one or more
Backtests.

- A Backtest selects a Watchlist and contains a Main setup plus ordered
  Failsafe setups. Each setup selects a Strategy and Profile and stores strategy
  input overrides. The Backtest shares one Strategy Performance calculation
  policy across its setups.
- A Portfolio owns initial capital, one Market, one Broker, aggregate capacity
  limits, and an ordered collection of Backtests. The Market is the trading
  venue or market context and provides venue-specific rules such as minimum
  order quantity and quantity step. The Broker represents the Market's broker
  or execution provider and provides its fee and execution-cost rules. Each
  setup Profile owns its
  drawdown adjustment and insufficient-cash response.
- Running a Portfolio simulates all of its Backtests. A single-asset simulation
  is a Portfolio containing one Backtest.

The same Backtest configuration may be referenced by different Portfolios.
Runtime positions, indicator results, and Strategy Performance evidence are
isolated within each Portfolio run.

## Ownership

A Backtest decides when it wants to trade and how large its Profile wants the
position to be. The Portfolio decides whether the shared account can execute
that request.

```text
Portfolio
|- Initial capital and account equity
|- Market rules
|- Broker fees
|- Maximum open trades
|- Maximum combined layers
|- Entry comparators
`- Ordered Backtests
   |- Watchlist
   `- Setups
      |- Strategy and inputs
      |- Entry filter
      `- Profile
         |- Position sizing
         |- Drawdown adjustment
         `- Insufficient-cash policy
```

Position sizing and Entry Filters read the current Portfolio equity and
drawdown. Each setup maintains independent theoretical Strategy Performance,
so Bayesian or Kelly sizing uses only that setup's history within the current
Portfolio run. Shared reservations do not change a Profile's sizing intent;
they constrain the later cash decision.

When an accepted initial order opens a real position, that position permanently
belongs to the requesting setup. Pyramids, partial exits, snapshots, the final
closed trade, and position-related execution events retain that owner even if a
different setup is active later. A rejected initial order is attributed to the
setup that requested it.

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
ranked by the Portfolio's ordered comparators. Portfolio, Backtest, and
Watchlist expansion order resolves complete ties.

For each risk-increasing request, Pludux performs these steps:

1. Capture the setup's theoretical Strategy Performance snapshot.
2. Evaluate its Profile sizing constraint and drawdown adjustment.
3. Use the shared Market and Broker to create an immutable Requested Order with
   normalized quantity, notional, fees, and risk values.
4. Evaluate the setup's Entry Filter against Strategy Performance, Requested
   Order values, and current account metrics. An entry-filtered setup may allow
   the next eligible Failsafe to produce its own fresh entry.
5. Rank every accepted Requested Order using the Portfolio comparators.
6. Enforce maximum-open-trade and maximum-combined-layer capacity.
7. Compare the fee-inclusive requested cost with available shared cash.
8. Reject the request or cap it to the largest affordable valid quantity using
   the owning Profile's insufficient-cash policy.

### Entry comparator data

Portfolio Entry Comparators can combine immutable Requested Order values with
the order's asset OHLCV fields and custom `DATA` fields. They support numeric
constants, basic scalar math, and lookback. They intentionally do not expose
Strategy named series or technical-indicator nodes: Strategy and Profile logic
has already produced the Requested Order before Portfolio ranking begins.

Asset data is phase-safe. A Current Close order sees the completed current bar.
A Next Open order sees the previous completed bar, because the current bar's
high, low, close, and volume do not yet exist. Use Requested Order Price when a
Next Open comparison needs the executable open price.

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
3. Apply each active backtest's configured intrabar path and price exits.
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

## Results and Charts

`PortfolioTimeline` records shared capital, available capital, equity, peak
equity, drawdown, realized and unrealized P&L, reserved notional, gross and net
exposure, open-position count, and backtest freshness on the union clock.

Each backtest retains a sparse `BacktestTimeline` and separate
series-evaluation results for every setup. The chart projects the selected
Setup × Asset row onto a compact axis:

- Every X index represents one real bar from the selected Backtest × Asset run,
  so its candles and volume remain consecutive.
- Portfolio equity and drawdown are sampled from the union timeline only at
  those backtest timestamps and plotted at the corresponding backtest index.
- Portfolio-only timestamps are omitted from this chart view. They remain in
  the stored `PortfolioTimeline` and reappear when a backtest containing those
  timestamps is selected.
- Axis labels and inspection still report each backtest bar's real UTC
  timestamp.
- No candles or portfolio values are synthesized to fill missing data.
- Strategy plots, named series, shadow returns, streaks, and strategy
  performance come from the selected setup.
- Entry, pyramid, rejection, and exit markers are shown only when attributed to
  the selected setup. Position and risk overlays are shown only for positions
  owned by that setup.
- The Trades view filters actual open and closed trades by their permanent setup
  owner. Its Hypothetical source reads only the selected setup's theoretical
  positions.
- Portfolio equity, drawdown, account metrics, execution status, and Overview
  remain aggregate because main and failsafe setups still execute together with
  shared Portfolio capital.

## Portfolio Hierarchy and Chart Selection

The Portfolios window is an expandable tree. Each top-level Portfolio contains
a flat Setup × Asset list. Rows are ordered by Portfolio Backtest order, then
Watchlist asset order, then Main setup followed by its ordered Failsafes. Each
row identifies the Backtest, Asset, setup role, Strategy, and Profile.

Selecting a Portfolio opens it and restores that Portfolio's last valid setup
selection. If the remembered setup was removed or is no longer valid, Pludux
selects the first valid row in display order. An empty or incomplete Portfolio
may have no active setup. Each Portfolio remembers its choice independently,
including when two Portfolios reuse the same Backtest.

Selecting a Setup × Asset child selects its parent Portfolio and filters the
Chart and Trades inspection to that setup. Candles provide the shared market
context, while Portfolio accounting, execution status, and Overview remain
aggregate across the Backtest × Asset run. Missing Backtest, Asset, Strategy,
or Profile references remain visible as disabled rows so broken configurations
can be identified without silently changing their order.

Portfolio search covers both levels. A Portfolio-name match shows all of its
children. Child matching includes Backtest, Asset, setup role, Strategy, and
Profile names while preserving the original display order.

The Backtests editor selection is separate: it chooses which reusable Backtest
configuration is being edited and never changes the Chart. The Portfolios tree
is the only Chart setup selector.

This first portfolio engine intentionally excludes margin, leverage models,
currency conversion, correlation limits, risk parity, and other advanced
allocation systems.
