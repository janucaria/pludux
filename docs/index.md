# Documentation

Pludux is a desktop GUI backtesting app. Start with the workflow below, then
use the reference pages when you need exact semantics.

## Start here

- [Getting Started](getting-started.md) — first usable workflow
- [Core Concepts](concepts.md) — canonical object semantics
- [Resources and Workflows](resources-and-workflows.md) — ownership and end-to-end setup
- [CSV Data](data-csv.md) — importing an Asset from CSV
- [GUI Guide](gui-guide.md) — where common tasks live in the desktop UI
- [Model JSON](model-json.md) — import/export format and method identifiers
- [Workspace Persistence](persistence.md) — `.pludux` save/open behavior
- [Limitations](limitations.md) — current scope and known boundaries

## Reference pages

- [Portfolio Backtesting](portfolio-backtesting.md) — authoritative portfolio timeline and valuation behavior
- [Node and Method Reference](nodes.md) — every Model, comparator, Entry Filter, and plot method
- [Execution and Position Lifecycle](execution.md) — timing, intrabar paths, exits, pyramiding, and Failsafes
- [Profiles, Markets, and Brokers](risk-market-broker.md) — sizing, normalization, fees, and cash policy
- [Results and Accounting](results.md) — Chart, Trades, Overview, and metric meanings
- [Turtle Trading Guidelines for Pludux](turtle-trading.md) — advanced Turtle-style reference
- [Model Performance Bayesian Kelly](strategy-performance-bayesian-kelly.md) — advanced sizing reference

## Developer reference

- [Building and Contributing](development.md) — presets, tests, architecture, and change discipline

## Recommended first flow

1. Import one CSV file as an Asset.
2. Create a Watchlist.
3. Create a Model.
4. Create a Profile.
5. Create a reusable Strategy from that Model + Profile.
6. Create a System that references the Strategy.
7. Set Market and Broker if needed.
8. Add the System to a Portfolio.
9. Run the Portfolio and inspect Chart, Trades, and Overview.
