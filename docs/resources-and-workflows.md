# Resources and Workflows

Pludux stores nine reusable setup resources. A Backtest is the result of running
one System against one Asset; it is not an editable stored resource or GUI
window.

```text
Asset <- Watchlist <- System <- Portfolio
                    /       \
Model + Profile <- Strategy  Market + Broker
```

| Resource | Owns | Used by |
| --- | --- | --- |
| Asset | Name, OHLCV mapping, timestamped numeric history | Watchlist |
| Watchlist | Ordered, duplicate-free Assets | System |
| Model | Inputs, series, long/short rules, execution path, plots | Strategy |
| Profile | Sizing, drawdown adjustment, insufficient-cash policy | Strategy |
| Strategy | Model, Profile, input overrides, Entry Filter | System |
| System | Watchlist, Main Strategy, ordered Failsafes, shared performance config | Portfolio |
| Market | Minimum quantity and quantity step | Portfolio |
| Broker | Fixed/percentage fee rules | Portfolio |
| Portfolio | Capital, constraints, comparators, ordered Systems | Runner/results |

References are shared. Editing a Strategy affects every System that references
it; duplicate first when making a variant. Deleting a referenced resource makes
its owner incomplete until the reference is repaired.

## Recommended workflow

1. Import Assets from CSV and verify OHLCV mapping.
2. Put Assets into a Watchlist in deterministic order.
3. Create a Model in **Models**, or import Model JSON.
4. Create a Profile and select one of its five sizing methods.
5. Create a Strategy that combines Model + Profile, overrides Model inputs, and
   optionally filters fresh entries.
6. Create a System with the Watchlist, Main Strategy, and optional ordered
   Failsafes.
7. Define Market quantity rules and Broker fees.
8. Create a Portfolio, add ordered Systems, configure capacity and comparators,
   then run it.
9. Inspect Chart, Trades, and Overview.

## Model and Strategy

A Model defines deterministic trading rules, not necessarily machine learning.
Its named series can be reused by long/short entry, exit, risk, pyramid, and
plot expressions. Each Strategy may override the Model's declared numeric
inputs without modifying the Model.

The Strategy Entry Filter evaluates a theoretical fresh initial Requested
Order. It decides whether that request enters Portfolio ranking; it does not
create signals or reevaluate pyramid layers. It can combine standard conditions
with `MODEL_PERFORMANCE.VALUE`. See [Node Reference](nodes.md).

## System and Failsafes

A System expands its Watchlist into per-Asset simulations. It owns one Main
Strategy, optional ordered Failsafe Strategy bindings, and one Model Performance
configuration shared across those Strategies. Failsafes are described in
[Execution and Position Lifecycle](execution.md).

## Profiles and sizing

Profiles offer five sizing policies:

- **Risk Distance** fits quantity to an equity-risk allowance divided by the
  Model's one-R price distance, accounting for estimated entry and 1R exit fees.
- **Fixed Quantity** starts from a configured unit count.
- **Fixed Budget** converts a configured cash budget to quantity.
- **Equity Fraction** uses a configured fraction of sizing equity.
- **Bayesian Kelly** derives a fraction from shared Model Performance; see
  [Bayesian Kelly](strategy-performance-bayesian-kelly.md).

Drawdown adjustment can reduce quantity and/or notional sizing equity in steps.
The Market then applies minimum quantity and rounds down to quantity step.
Broker fees are estimated during sizing. If entry cost exceeds cash, the Profile
either **Rejects** the request or **Caps to Available Cash**, then normalizes
again. A zero or invalid normalized quantity is rejected.

## Markets and Brokers

A Market represents venue quantity rules. A Broker has an ordered set of fee
rules. Each rule is fixed or percentage based; is scoped to **Long**, **Short**,
or **Both**; and triggers on **Entry**, **Exit**, **Buy**, **Sell**, or **All**.
All matching rules are added—the first match does not suppress later matches.

## Portfolio

A Portfolio owns initial capital, Market, Broker, maximum open trades, maximum
combined layers, entry comparator expressions, and ordered Systems. Complete
ranking ties resolve by Portfolio System order and then Watchlist Asset order.
The authoritative union-clock, comparator-visibility, accounting, and valuation
rules are in [Portfolio Backtesting](portfolio-backtesting.md).

## Reproducibility

Keep input CSV and exported Model JSON alongside experiment notes. Preserve
Watchlist and System ordering because both are deterministic tie-break inputs.
Save the `.pludux` workspace for convenient restoration, but do not treat result
views as persisted experiment output; see [Persistence](persistence.md).
