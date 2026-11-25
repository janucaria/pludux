# Strategy JSON

Strategy JSON exchanges the rules and input defaults of a single Strategy.
The **Strategies** editor imports and exports this file. To share a System's
Profile selection, input overrides, Calls mappings, and Entry Filters, save the
workspace.
The Strategy parser accepts JSON comments and rejects trailing commas.

## Root structure

Version 1 requires only `version`. `execution`, `series`, `positions`, and
`plots` are optional:

```json
{
  "version": 1,
  "execution": { "intrabarPath": "CANDLE_DIRECTION" },
  "series": {},
  "positions": { "long": false, "short": false },
  "plots": []
}
```

`intrabarPath` is `LOW_FIRST`, `HIGH_FIRST`, or `CANDLE_DIRECTION`.

An omitted field uses the value from a freshly constructed `Strategy` or nested
rule type. This applies to execution, position sections and rule properties,
collections, plots, and method parameters that have domain defaults. Explicit
values are still validated: `null`, wrong types, invalid enums or methods,
invalid ranges, lossy integral conversions, numeric strings, and overflow are
rejected. Unknown properties are ignored. Former names are not aliases, so an
unknown legacy property never overrides a canonical default.

`series` is a lower-snake-case dictionary of reusable named expressions.
`positions` is an object whose `long` and `short` values are `false` or a side
configuration defining entry rules, risk distance, ordered signal exits,
pyramiding, stop losses, and take profits. `plots` is an array of plot groups;
each group carries a label, overlay choice, and ordered plot methods. Current
enum spellings include:

- entry timing: `CURRENT_CLOSE`, `NEXT_OPEN`
- entry source: `SIGNAL` (default), `CALL`
- exit activation: `SIMULTANEOUS`, `AFTER_PREVIOUS`
- pyramid retrigger: `EVERY_EVALUATION`, `AFTER_FALSE`
- stop/target reference: `LATEST_ENTRY_PRICE`, `AVERAGE_PRICE`,
  `INITIAL_ENTRY_PRICE`

Reduction fractions must be greater than zero and no greater than one. Refer to
[Execution and Position Lifecycle](execution.md) before changing these fields.

Set `positions.long.entry.source` or `positions.short.entry.source` to `CALL`
to use the matching Calls table row at the candle open. Signal and timing
apply to entries whose source is `SIGNAL`. [Call entries](execution.md#call-entries)

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
