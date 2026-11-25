# CSV Data

Pludux imports market data into **Assets** and dated trade instructions into
**Calls** from separate CSV files.

Reimporting a CSV replaces the history of the first Asset with the same
case-sensitive name in the Assets list. Its watchlist references and OHLCV
mappings are preserved. Desktop and browser imports use the full filename,
including its extension. To refresh an older Asset named `BTC` in place,
rename it to `BTC.csv` before importing that file.

## Verified behavior

- The first column is read as the date/time field.
- Remaining columns are loaded as numeric series.
- The date column may be a numeric timestamp or one of these exact text forms:
  `YYYY-MM-DD`, `YYYY-MM-DDThh:mm`, or `YYYY-MM-DD hh:mm`, optionally followed
  by `:ss`, a fractional-second suffix, and `Z` or `+hh:mm`/`-hh:mm`.
- If the loaded data is in reverse chronological order, the series is reversed
  before being stored.
- Pludux does not repair arbitrary out-of-order timestamps.
- Portfolio timelines require finite, strictly increasing timestamps.
- The Asset editor lets you map the OHLCV field names used by the importer.

## Asset editor mapping

The Asset editor shows explicit fields for:

- Name
- Open price
- High price
- Low price
- Close price
- Volume

The editor text says to enter the exact column headers and notes that header
matching is case-sensitive.

## Practical guidance

If your CSV uses different column names, adjust the field mapping in the GUI to
match the file exactly. Where the file format is not verified here, follow the
current importer/editor prompts rather than guessing labels.

Numeric timestamps are stored as supplied. Parsed text timestamps pass through
the platform C time conversion, and chart labels use the application's local
display timezone. Therefore, use one explicit convention throughout a dataset,
verify an imported candle's displayed time, and do not infer that a `Z` suffix
makes every GUI label display in UTC. The importer reverses the whole dataset
only when the final timestamp is earlier than the first; it does not sort or
deduplicate mixed-order rows.

## Calls CSV

In **Calls**, select **Import CSV** to create a trade-call table from a CSV
file with a header row. Use the same timestamp convention as the Asset data
when preparing the CSV. Map the imported table in **Systems**. [Calls resource and mapping rules](resources-and-workflows.md#calls)

### Backtest calls with two stop alternatives

Suppose the Watchlist contains daily OHLCV data for `ABC.csv` and `XYZ.csv`.
The `SL1 Distance`, `SL2 Distance`, and `TP Distance` columns in this example
are distances from the fill price, in Asset price units.

```csv
Date,Symbol,Side,Entry Low,Entry High,SL1 Distance,SL2 Distance,TP Distance
2025-01-06,ABC,Long,100,103,4,6,8
2025-01-07,XYZ,Long,170,172,5,7,10
```

1. Import the CSV in **Calls**. In **Systems**, select the table and map `Date` to
   Datetime, `Symbol` to Asset, and `Side` to Direction. Map both entry bounds.
2. Create a long Strategy with **Call** as its entry source. Set a one-R
   stop. Use Call nodes with field `risk_distance` for the risk amount and
   field `target_distance` for the take-profit amount. The JSON below defines
   these rules.
3. Select the Strategy in the System's Main setup. Map `risk_distance` to
   `SL1 Distance` and `target_distance` to `TP Distance`. Run the Portfolio.
   To compare the alternative stop, map `risk_distance` to `SL2 Distance`
   and rerun with the same Portfolio settings.

```json
{
  "version": 1,
  "positions": {
    "long": {
      "entry": { "source": "CALL" },
      "riskDistance": {
        "method": "RISK_DISTANCE.AMOUNT",
        "params": {
          "amount": { "method": "OTHER.CALL", "params": { "field": "risk_distance" } }
        }
      },
      "stopLosses": {
        "rules": [{ "enabled": true, "stopPrice": { "method": "STOP_LOSS.ONE_R" } }]
      },
      "takeProfits": {
        "rules": [{
          "enabled": true,
          "targetPrice": {
            "method": "TAKE_PROFIT.AMOUNT",
            "params": {
              "amount": { "method": "OTHER.CALL", "params": { "field": "target_distance" } }
            }
          }
        }]
      }
    },
    "short": false
  }
}
```

If `ABC.csv` opens at 101 on January 6 and the Portfolio admits the order,
entry is at 101, with a stop at 97 and a target at 109. If `XYZ.csv` opens at
174 on January 7, its Call is skipped. Trading through the range later in the
same candle does not create an entry. [Call entries](execution.md#call-entries)

## Related docs

- [Getting Started](getting-started.md)
- [GUI Guide](gui-guide.md)
- [Core Concepts](concepts.md)
