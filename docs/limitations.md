# Limitations

## Product scope

- Pludux is focused on portfolio backtesting, not live trading.
- Model JSON import/export is the primary documented interchange format.

## Simulation boundaries

- Portfolio backtests use the union of asset timelines.
- Missing candles are not synthesized.
- Last-known prices are used only for valuation where the design permits it.
- Asset timestamps must be finite and strictly increasing.
- The first public portfolio engine intentionally excludes margin, leverage
  models, currency conversion, correlation limits, risk parity, slippage, and
  deeper intrabar simulation.

## Data and import boundaries

- CSV import accepts the timestamp grammar documented in [CSV Data](data-csv.md).
- Exact column names should be taken from the Asset editor.
- If a label or optional field is not verified in the docs, use the GUI prompt.

## Workflow boundaries

- The first recommended flow is Asset -> Watchlist; Model + Profile ->
  Strategy; Watchlist + Strategy -> System; System -> Portfolio -> Backtest.
- Failsafe Strategy bindings exist, but they are part of more advanced System
  design.
- Strategies are reusable stored resources; editing one affects every System
  that references it.
- Advanced Turtle and Bayesian Kelly docs are reference material, not required
  first-run setup.

## Reference links

- [Portfolio Backtesting](portfolio-backtesting.md)
- [Turtle Trading Guidelines for Pludux](turtle-trading.md)
- [Model Performance Bayesian Kelly](strategy-performance-bayesian-kelly.md)
