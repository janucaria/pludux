# Portfolio Backtesting

## Terminology

Pludux treats a **Backtest** as a reusable Asset + Strategy + Profile
configuration and a **Portfolio** as the account that executes one or more
Backtests.

- A Backtest selects an Asset, Strategy, and Profile. It also stores strategy
  input overrides and the Strategy Performance model configuration.
- A Portfolio owns initial capital, one Market, one Broker, drawdown adjustment,
  insufficient-cash handling, and an ordered collection of Backtests.
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
|- Drawdown adjustment
|- Insufficient-cash policy
`- Ordered Backtests
   |- Asset + Strategy + Profile
   |- Asset + Strategy + Profile
   `- Asset + Strategy + Profile
```

Position sizing and execution filters read the current Portfolio equity and
drawdown. Strategy Performance remains backtest-specific, so Bayesian or Kelly
sizing uses only the theoretical history of that Backtest within the current
Portfolio run.

## Shared-Capital Arbitration

At a market timestamp, risk-reducing work is processed before risk-increasing
work. Entry and pyramiding requests that occur in the same phase are processed
in the Portfolio's displayed Backtest order. That order is therefore the
deterministic capital priority.

For each risk-increasing request, Pludux performs these steps:

1. Evaluate the backtest Profile's execution filter and requested size.
2. Apply the Portfolio drawdown adjustment.
3. Normalize quantity using the shared Market rules.
4. Calculate fees using the shared Broker.
5. Compare required capital with remaining shared capital.
6. Reject the request or cap it to the largest affordable valid quantity,
   according to the Portfolio policy.

An accepted order reserves its notional before the next backtest request is
considered.

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

Each backtest retains a sparse `BacktestTimeline` and its own series-evaluation
results. The chart projects the selected Backtest onto a compact axis:

- Every X index represents one real bar from the selected Backtest, so its
  candles, volume, indicators, signals, and trades are consecutive.
- Portfolio equity and drawdown are sampled from the union timeline only at
  those backtest timestamps and plotted at the corresponding backtest index.
- Portfolio-only timestamps are omitted from this chart view. They remain in
  the stored `PortfolioTimeline` and reappear when a backtest containing those
  timestamps is selected.
- Axis labels and inspection still report each backtest bar's real UTC
  timestamp.
- No candles or portfolio values are synthesized to fill missing data.
- The Trades view shows only trades from the selected Backtest.

## Portfolio Hierarchy and Chart Selection

The Portfolios window is an expandable tree. Each top-level Portfolio contains
its referenced Backtests in execution-priority order. Selecting a Portfolio
opens it and restores that Portfolio's last valid backtest selection. If the
remembered Backtest was removed or is no longer referenced, Pludux selects the
first available Backtest in portfolio order. An empty Portfolio has no active
backtest.

Selecting a Backtest child selects both its parent Portfolio and that backtest
for the Chart. Each Portfolio remembers this choice independently, including
when two Portfolios reuse the same Backtest. Missing Backtest references remain
visible as disabled `Missing Backtest` children so broken configurations can be
identified without silently changing their order.

Portfolio search covers both levels. A Portfolio-name match shows all of its
children; a Backtest-name match shows its parent and only matching children,
while preserving their original priority order.

The Backtests editor selection is separate: it chooses which reusable Backtest
configuration is being edited and never changes the Chart. The Portfolios tree
is the only Chart backtest selector.

This first portfolio engine intentionally excludes margin, leverage models,
currency conversion, correlation limits, risk parity, and other advanced
allocation systems.
