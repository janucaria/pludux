# Documentation

Pludux is a desktop GUI backtesting app. Start with the workflow below, then
use the reference pages when you need exact semantics.

## Start here

- [Getting Started](getting-started.md) — first usable workflow
- [Core Concepts](concepts.md) — what each Pludux object does
- [CSV Data](data-csv.md) — importing an Asset from CSV
- [GUI Guide](gui-guide.md) — where common tasks live in the desktop UI
- [Limitations](limitations.md) — current scope and known boundaries

## Reference pages

- [Portfolio Backtesting](portfolio-backtesting.md) — authoritative portfolio timeline and valuation behavior
- [Turtle Trading Guidelines for Pludux](turtle-trading.md) — advanced Turtle-style reference
- [Strategy Performance Bayesian Kelly](strategy-performance-bayesian-kelly.md) — advanced sizing reference

## Recommended first flow

1. Import one CSV file as an Asset.
2. Create a Strategy.
3. Create a Profile.
4. Create a Backtest from that Asset + Strategy + Profile.
5. Add the Backtest to a Portfolio.
6. Set Market and Broker if needed.
7. Run the Portfolio and inspect Chart, Trades, and Overview.
