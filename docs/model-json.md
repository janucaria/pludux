# Model JSON

Model JSON is Pludux's exchange format for importing and exporting Models. The
root document uses `"version": 1`.

## Method identifiers

Every serialized `method` identifier is uppercase and dot-qualified. Dots
separate semantic namespaces, and underscores separate words within a name:

- `INDICATOR.SMA`
- `INDICATOR.STOCH_RSI`
- `OPERATOR.POSITIVE_PART`
- `MARKET_DATA.CLOSE`
- `REQUESTED_ORDER.PRICE`
- `PLOT.MOMENTUM_HISTOGRAM`
- `PLOT_SOURCE.SERIES`

The same identifiers are used when a method supports string shorthand. For
example, use `"MARKET_DATA.CLOSE"` as a direct market-data source, or use an
object when parameters are needed:

```json
{
  "method": "INDICATOR.SMA",
  "params": {
    "period": 20,
    "source": "MARKET_DATA.CLOSE"
  }
}
```

This naming rule applies only to method identifiers. Other schema values keep
their defined spelling and case. For example, `maSmoothingType` may be `"RMA"`
and an input `representation` may be `"UnsignedInteger"`.

See the
[version 1 draft schema](../data/backtest/schemas/pludux.backtest.model.v1-draft.json)
for the complete document structure and accepted identifiers.
