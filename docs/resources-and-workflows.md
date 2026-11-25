# Resources and Workflows

Pludux stores nine reusable resources. A Backtest records a System × Asset run.

| Resource | Owns | Used by |
| --- | --- | --- |
| Asset | Name, OHLCV mapping, timestamped numeric history | Watchlist |
| Calls table | Name, column headers, and data rows | System |
| Watchlist | Ordered, duplicate-free Assets | System |
| Strategy | Inputs and defaults, series, long/short rules, execution path, plots | Main and Failsafe setups |
| Profile | Sizing, drawdown adjustment, insufficient-cash policy | System |
| System | Watchlist, Profile, Main setup, ordered Failsafes, performance settings, optional Calls table and mappings | Portfolio |
| Market | Minimum quantity and quantity step | Portfolio |
| Broker | Fixed/percentage fee rules | Portfolio |
| Portfolio | Capital, constraints, comparators, ordered Systems | Runner/results |

Editing a Strategy changes its rules across all uses. Changing its inputs or
Entry Filter in a System affects only that setup. Deleting a referenced
resource leaves its owner incomplete until the reference is repaired.

## Recommended workflow

1. Import Assets from CSV and verify OHLCV mapping.
2. Put Assets into a Watchlist in deterministic order.
3. Create a Strategy in **Strategies**, or import Strategy JSON.
4. Create a Profile and select one of its five sizing methods.
5. Create a System and select its Watchlist and Profile. Choose a Main Strategy,
   set its input overrides and Entry Filter, and add ordered Failsafes as needed.
6. Define Market quantity rules and Broker fees.
7. Create a Portfolio, add ordered Systems, configure capacity and comparators,
   then run it.
8. Inspect Chart, Trades, and Overview.

## Strategies and their setups

The Strategy defines named series for its long/short entry, exit, risk,
pyramid, and plot expressions. Each Main or Failsafe setup references a
Strategy and can override its declared numeric inputs. For example, a System
can reference a breakout Strategy twice with different channel lengths.

The setup's Entry Filter evaluates the theoretical fresh initial Requested
Order and decides whether it enters Portfolio ranking. The filter can combine
standard conditions with `STRATEGY_PERFORMANCE.VALUE`. [Node Reference](nodes.md)

A System expands its Watchlist into per-Asset simulations. All its setups use
the same Profile and Strategy Performance calculation settings. Each setup
maintains its own theoretical trades and performance history for each Asset.
[Execution and Position Lifecycle](execution.md)

## Calls

The Calls library holds named trade-call tables with headers and data rows.
A System can select one table for its Strategy setups. The Strategy side's
entry source determines whether matching rows can request an entry.

Map **Asset** and **Datetime** in the System. Datetime defaults to the first
column. Pludux matches the Asset by exact Watchlist name, then by a unique
`.csv` filename match, such as `ABC` to `ABC.csv`. Rows outside the Watchlist
remain unmatched. A Call can request entry at a real candle with the exact
timestamp. Duplicate Asset and datetime keys report both conflicting rows.

The optional **Direction** column accepts the values below. An unmapped
column or blank cell defaults to Long. Pludux ignores case and surrounding
whitespace and reports the row and column of any unrecognized value.

| Direction | Accepted values |
| --- | --- |
| Long | `long`, `true`, `yes`, `1` |
| Short | `short`, `false`, `no`, `0` |

**Entry Low** and **Entry High** are optional inclusive bounds on the open
price. Equal bounds specify an exact price. Map any extra numeric columns to
logical `OTHER.CALL` fields separately for each Main or Failsafe setup. A
required field with no mapping leaves the setup incomplete. Blank extra cells
give missing values. Text and non-finite numbers in mapped numeric cells are
invalid.

The table preview, diagnostics, and results count the header as row 1, so
the first data row is row 2. [Worked CSV import example](data-csv.md#backtest-calls-with-two-stop-alternatives)

## Profiles and sizing

Profiles offer five sizing policies:

- **Risk Distance** fits quantity to an equity-risk allowance divided by the
  Strategy's one-R price distance, accounting for estimated entry and 1R exit fees.
- **Fixed Quantity** starts from a configured unit count.
- **Fixed Budget** converts a configured cash budget to quantity.
- **Equity Fraction** uses a configured fraction of sizing equity.
- **Bayesian Kelly** derives a fraction from the requesting setup’s Strategy Performance; see
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

Keep input CSV and exported Strategy JSON alongside experiment notes. Preserve
Watchlist and System ordering because both are deterministic tie-break inputs.
Save the `.pludux` workspace for convenient restoration, but do not treat result
views as persisted experiment output; see [Persistence](persistence.md).
