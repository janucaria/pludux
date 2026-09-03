# Model JSON

Model JSON is the versioned exchange format for one Model. The GUI imports and
verifies **Models**, not Strategies. The parser accepts JSON comments but not
trailing commas.

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

An omitted field uses the value from a freshly constructed `Model` or nested
model type. This applies to execution, position sections and rule properties,
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
- exit activation: `SIMULTANEOUS`, `AFTER_PREVIOUS`
- pyramid retrigger: `EVERY_EVALUATION`, `AFTER_FALSE`
- stop/target reference: `LATEST_ENTRY_PRICE`, `AVERAGE_PRICE`,
  `INITIAL_ENTRY_PRICE`

Reduction fractions must be greater than zero and no greater than one. Refer to
[Execution and Position Lifecycle](execution.md) before changing these fields.

## Method objects

Every serialized `method` is uppercase and dot-qualified:

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

Every method object that is present must include its canonical `method`
discriminator. Operands and payloads remain required when the concrete node has
no valid default. Export always writes the full canonical representation and
never emits legacy aliases.

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

The [draft schema](../data/backtest/schemas/pludux.backtest.model.v1-draft.json)
and [samples](../data/backtest/samples) are authoring aids. The schema permits
unknown properties and mirrors runtime defaults while retaining required
method discriminators and non-defaultable operands. The running parser
and [Node Reference](nodes.md) are the current behavior source if a registered
method is not yet represented there. Import errors are reported in **Models**;
export names the JSON from the Model name.
