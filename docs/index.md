# Documentation

Pludux is a desktop GUI backtesting app. Start with the workflow below, then
use the reference pages when you need exact semantics.

## Start here

- [Getting Started](getting-started.md) — first usable workflow
- [Core Concepts](concepts.md) — canonical object semantics
- [CSV Data](data-csv.md) — importing an Asset from CSV
- [GUI Guide](gui-guide.md) — where common tasks live in the desktop UI
- [Limitations](limitations.md) — current scope and known boundaries

## Reference pages

- [Portfolio Backtesting](portfolio-backtesting.md) — authoritative portfolio timeline and valuation behavior
- [Turtle Trading Guidelines for Pludux](turtle-trading.md) — advanced Turtle-style reference
- [Model Performance Bayesian Kelly](strategy-performance-bayesian-kelly.md) — advanced sizing reference

## Recommended first flow

1. Import one CSV file as an Asset.
2. Create a Model.
3. Create a Profile.
4. Create a reusable Strategy from that Model + Profile.
5. Create a System that references the Strategy.
6. Add the System to a Portfolio.
7. Set Market and Broker if needed.
8. Run the Portfolio and inspect Chart, Trades, and Overview.
