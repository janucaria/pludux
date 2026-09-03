# Results and Accounting

Pludux exposes results through **Chart**, **Trades**, and **Overview** after a
Portfolio run. Results are invalidated when setup changes and are not persisted
as reusable resources.

## Chart

Select a Portfolio setup and Asset to inspect candles, Model plots, position
state, and execution markers for that expanded Backtest. Chart timestamp labels
use the application's local/display timezone; they are not guaranteed to be UTC
labels. Hover details show the values visible at that bar.

## Trades

**Actual** trades are executions admitted by shared Portfolio capital and
capacity. **Hypothetical** trades are the per-Backtest theoretical executions
used to understand Model behavior even when the Portfolio rejected or ranked
away a request. Filters narrow the displayed Portfolio/System/Asset or side.
Do not combine actual and hypothetical rows as one accounting ledger.

A closed actual trade records entry and exit value, quantities, fees, direction,
strategy index, risk reference/distance, and realized profit or loss. Partial
reductions accumulate into the eventual closed-trade record.

## Overview and timeline terms

- **Cash** is uncommitted account cash after executions and fees.
- **Market value** values open positions from prices allowed by the portfolio
  valuation rules.
- **Equity** is cash plus open-position market value.
- **Peak equity** is the greatest equity observed so far.
- **Drawdown** is the decline from peak equity at that timeline row.
- **Maximum drawdown** is the greatest drawdown observed so far.
- **Realized P&L** comes from closed actual trades after their entry and exit
  fees; unrealized movement enters equity through valuation.

Summary counts, win/loss rates, returns, streaks, and fee totals are computed
from actual closed Portfolio trades. Model Performance metrics are a separate
theoretical stream and can differ when entry filtering, ranking, cash, or
capacity prevents execution.

For exact union-clock rows, missing-candle valuation, entry comparator
visibility, and event ordering, use the authoritative
[Portfolio Backtesting](portfolio-backtesting.md).
