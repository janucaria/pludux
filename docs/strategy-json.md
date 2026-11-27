# Strategy JSON

Strategy JSON exchanges the rules and input defaults of a single Strategy.
The **Strategies** editor imports and exports this file. To share a System's
Profile selection, input overrides, Calls mappings, external Asset bindings,
and Entry Filters, save the workspace.
The Strategy parser accepts JSON comments and rejects trailing commas.

## Root structure

Version 1 requires only `version`. `series`, `positions`, and `plots` are
optional.

```json
{
  "version": 1,
  "series": {},
  "positions": { "long": false, "short": false },
  "plots": []
}
```

Version 1 files with the legacy `execution.intrabarPath` field still import.
Backtests use the Portfolio's Intrabar Path, and current Strategy exports omit
the legacy field. [Intrabar Path](execution.md#intrabar-path)

An omitted field uses the value from a freshly constructed `Strategy` or nested
rule type. This applies to position sections and rule properties, collections,
plots, and method parameters that have domain defaults. For recognized fields,
the parser rejects `null`, wrong types, invalid enums or methods, invalid
ranges, lossy integral conversions, numeric strings, and overflow. Unknown
properties are ignored.

`series` is a lower-snake-case dictionary of reusable named expressions.
`positions` is an object whose `long` and `short` values are `false` or a side
configuration defining entry rules, risk distance, ordered signal exits,
pyramiding, stop losses, and take profits. `plots` is an array of plot groups;
each group carries a label, overlay choice, and ordered plot methods. Current
enum spellings include:

- entry source: `SIGNAL` (default), `CALL`
- price exits start: `CURRENT_BAR` (default), `NEXT_BAR`
- exit activation: `SIMULTANEOUS`, `AFTER_PREVIOUS`
- pyramid retrigger: `EVERY_EVALUATION`, `AFTER_FALSE`
- stop/target reference: `LATEST_ENTRY_PRICE`, `AVERAGE_PRICE`,
  `INITIAL_ENTRY_PRICE`

Reduction fractions must be greater than zero and no greater than one. Refer to
[Execution and Position Lifecycle](execution.md) before changing these fields.

Set `positions.long.entry.source` or `positions.short.entry.source` to `CALL`
to use the matching Calls table row at the candle open.
[Call entries](execution.md#call-entries)

## Signal order fields

Each position's `entry`, each rule in `exits.rules`, and `pyramiding` accept
`price` and `delay`. The default for each is `MARKET_DATA.OPEN` with delay 1.
`SIGNAL` entries also accept `priceExitsStart`. For example, these are the
defaults under `positions.long.entry`.

```json
{
  "price": { "method": "MARKET_DATA.OPEN" },
  "delay": 1,
  "priceExitsStart": "CURRENT_BAR"
}
```

- `price` accepts a numeric literal or a Strategy method. Use
  `MARKET_DATA.OPEN` or `MARKET_DATA.CLOSE` directly to fill at the
  corresponding candle event. Other expression results fill at their first
  touch on the candle's modeled path.
- `delay` is a nonnegative integer that looks back for the rule's signal in
  real candles on the same Asset. Delay 0 uses the current candle.
- `priceExitsStart` controls when stop losses, trailing stops, and take profits
  become active on a new position. `CURRENT_BAR` starts them after the fill.
  `NEXT_BAR` starts them at the Asset's next real open.

For example, this fragment selects the previous candle's signal, calculates
95% of the current candle's closing price, and starts price exits at the next
real open. The current candle must touch the computed price to fill.

```json
{
  "price": {
    "method": "OPERATOR.MULTIPLY",
    "params": {
      "multiplicand": "MARKET_DATA.CLOSE",
      "multiplier": 0.95
    }
  },
  "delay": 1,
  "priceExitsStart": "NEXT_BAR"
}
```

Existing `timing` values still import for entries, signal exits, and
pyramiding. `NEXT_OPEN` maps to `MARKET_DATA.OPEN` and delay 1.
`CURRENT_CLOSE` maps to `MARKET_DATA.CLOSE` and delay 0. A migrated entry uses
`CURRENT_BAR` for `NEXT_OPEN` or `NEXT_BAR` for `CURRENT_CLOSE`. When a legacy
entry omits both `delay` and `priceExitsStart`, its `timing` continues to
determine the fill even if the file contains an old `price` field. Exports
write the new fields. [Signal prices and delays](execution.md#signal-prices-and-delays)

## Method objects

Serialized `method` identifiers use uppercase, dot-qualified names.

```json
{
  "method": "INDICATOR.SMA",
  "params": {
    "period": 20,
    "source": "MARKET_DATA.CLOSE"
  }
}
```

String shorthand is accepted where the method has no required explicit
parameters. Other values retain their documented casing; for example an input
`representation` may be `UnsignedInteger`. The complete current inventory and
contexts are in [Node and Method Reference](nodes.md).

`EXTERN.FIELD` requires `reference` and `field` strings in `params`. For example,
`{"method":"EXTERN.FIELD","params":{"reference":"market","field":"CLOSE"}}`
reads the mapped close from the Asset the System binds to `market`.
[External Asset lookup rules](nodes.md#external-asset-fields)

`OTHER.CALL` uses a logical `field` and an optional `representation` in `params`.
Map the field to a column in the System's selected table. [Worked example](data-csv.md#backtest-calls-with-two-stop-alternatives)

Every method object that is present must include its canonical `method`
discriminator. Operands and payloads remain required when the concrete node has
no valid default. Export always writes the full canonical representation and
never emits legacy aliases.

## Above By and Below By comparisons

Use `COMPARISON.ABOVE_BY_PERCENT` or `COMPARISON.BELOW_BY_PERCENT` with
`target`, `reference`, and `percent`. A `percent` of `15` sets the minimum gap
to 15% of the reference. Use `COMPARISON.ABOVE_BY` or `COMPARISON.BELOW_BY`
with `target`, `reference`, and `gap` to supply the minimum gap directly. Each
parameter accepts a numeric literal or expression.

Both samples use `LOGIC.OR` and calculate the ATR-based gap with
`OPERATOR.MULTIPLY`.

- The long entry triggers when the close is at least 15% above the 20-period
  SMA or when the 20-period SMA is above the 50-period SMA by at least
  1.5 times the 14-period ATR. [Above By Strategy JSON](../data/backtest/samples/above_by.json)
- The short entry triggers when the close is at least 15% below the 20-period
  SMA or when the 20-period SMA is below the 50-period SMA by at least
  1.5 times the 14-period ATR. [Below By Strategy JSON](../data/backtest/samples/below_by.json)

## Multi-output series and plots

`INDICATOR.BB`, `INDICATOR.KC`, `INDICATOR.DC`, `INDICATOR.MACD`,
`INDICATOR.STOCH`, and `INDICATOR.STOCH_RSI` have named outputs. Select one with
`OPERATOR.SELECT_OUTPUT`; supported names are `default`, `macd-line`,
`signal-line`, `histogram`, `k-percent`, `d-percent`, `middle-band`,
`upper-band`, and `lower-band`, as appropriate to the source.

Plot groups use `PLOT.HLINE`, `PLOT.LINE`, `PLOT.HISTOGRAM`, or
`PLOT.MOMENTUM_HISTOGRAM`. A plot reads `PLOT_SOURCE.SERIES` or
`PLOT_SOURCE.CONSTANT`. Momentum histograms configure positive/negative and
rising/falling colors.

## Schema and samples

The [draft schema](../data/backtest/schemas/pludux.backtest.strategy.v1-draft.json)
and [samples](../data/backtest/samples) are authoring aids. The schema permits
unknown properties and mirrors runtime defaults while retaining required
method discriminators and non-defaultable operands. The running parser
and [Node Reference](nodes.md) are the current behavior source if a registered
method is not yet represented there. Import errors are reported in **Strategies**;
export names the JSON from the Strategy name.
