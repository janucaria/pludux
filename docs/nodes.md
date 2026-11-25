# Node and Method Reference

This is the complete method inventory exposed by the current editors and
serializers. Method identifiers are uppercase and dot-qualified. **Strategy**
means named series, signals, prices, risk, and plots. **Comparator** means
Portfolio entry ranking. **Entry Filter** uses its own scalar/condition parser.
Both Comparator and Entry Filter expose `STRATEGY_PERFORMANCE.VALUE`.

Unless stated otherwise, numeric sources accept another series node and
condition inputs accept another condition node. Defaults shown by imported JSON
are defined by the parser; the GUI may seed a more useful initial expression.

## Evaluation contract

- A numeric node returns one `double` per evaluation. Price nodes use Asset
  price units, quantity nodes use units, and fee/cost/equity nodes use the
  Portfolio currency. Percentage nodes use percentage points.
- Nodes see only state visible in the current execution phase. For **Next
  Open**, historical market-data and named-series expressions remain on the
  preceding completed candle; `REQUESTED_ORDER.PRICE` is the current executable
  open in a comparator.
- Indicators and lookbacks need history. Before sufficient history exists they
  can produce a non-finite value. Arithmetic generally propagates non-finite
  inputs; division by zero and square root of a negative value must not be used
  as trading signals.
- `SERIES.REFERENCE` must name a top-level series. Recursive or missing
  references are invalid. `INPUT.NUMERIC` values are declared by a Strategy and
  may be overridden by each Main or Failsafe setup in a System.

## Context matrix

| Family | Strategy | Comparator | Entry Filter |
| --- | --- | --- | --- |
| Market data, lookback, scalar arithmetic | Yes | Yes | No |
| Call row values | Yes | No | No |
| Constant | Yes | Yes | Yes |
| Indicators, named series, position state | Yes | No | No |
| Portfolio equity/drawdown state | Yes | No | Yes |
| Conditions | All below | No | Six basic numeric comparisons and AND/OR/XOR/NOT/ALWAYS/NEVER |
| Requested Order values | No | Yes | Yes, for the candidate order |
| Strategy Performance | No | Yes | Yes |
| Risk, stop, target, and plot methods | Owning Strategy fields only | No | No |

The comparator deliberately does not resolve named Strategy series or indicators.
It has OHLCV/custom data, constants, lookback, scalar arithmetic, Requested
Order values, and Strategy Performance snapshots. These restrictions are part of
the execution contract.

## Market, account, and reusable values

| ID | Output and parameters | Context |
| --- | --- | --- |
| `MARKET_DATA.OPEN` | Current visible open | Strategy, comparator |
| `MARKET_DATA.HIGH` | Current visible high | Strategy, comparator |
| `MARKET_DATA.LOW` | Current visible low | Strategy, comparator |
| `MARKET_DATA.CLOSE` | Current visible close | Strategy, comparator |
| `MARKET_DATA.VOLUME` | Current visible volume | Strategy, comparator |
| `MARKET_DATA.FIELD` | Named imported numeric field; required `field` string | Strategy, comparator |
| `PORTFOLIO.EQUITY` | Current Portfolio equity | Strategy, Entry Filter |
| `PORTFOLIO.EQUITY_PERCENT` | Equity relative to initial capital, percent | Strategy, Entry Filter |
| `PORTFOLIO.DRAWDOWN` | Current drawdown | Strategy, Entry Filter |
| `VALUE.CONSTANT` | Constant; required `value` | Strategy, comparator, Entry Filter |
| `SERIES.REFERENCE` | Named top-level series; `name` | Strategy |
| `INPUT.NUMERIC` | Declared input; `label`, `representation`, `value` | Strategy |

Comparator market fields obey Portfolio comparator visibility rules. In
particular, Next Open ranking sees the previous completed bar; use
`REQUESTED_ORDER.PRICE` for the executable current open. See
[Portfolio Backtesting](portfolio-backtesting.md).

`INPUT.NUMERIC.representation` is `Decimal` (default), `SignedInteger`, or
`UnsignedInteger`; defaults are an empty label and value `0`. A numeric JSON
literal is shorthand for `VALUE.CONSTANT`. Parameterless methods may use their
exact method-ID string as shorthand.

## Call row values

| ID | Output and parameters | Context |
| --- | --- | --- |
| `OTHER.CALL` | Value for the required logical `field`. Optional `representation` is `Decimal` (default), `SignedInteger`, or `UnsignedInteger`. | Strategy |

The System setup maps the logical field to a column in the selected Calls
table. As with `INPUT.NUMERIC`, `Decimal` preserves the numeric value,
`SignedInteger` truncates toward zero, and `UnsignedInteger` also clamps
negative values to zero.

During entry sizing, `OTHER.CALL` reads the candidate row. After entry, it reads the
row of the position being evaluated, whether actual or hypothetical, until
that position closes. A later Call row leaves the existing position's values
intact. The node gives a missing value (`NaN`) when the cell is blank or no
Call row is active, including when the Strategy is flat with no Call candidate
or is evaluating a Signal-owned position. [Worked example](data-csv.md#backtest-calls-with-two-stop-alternatives)

## Indicators

| ID | Output and principal parameters | Multi-output |
| --- | --- | --- |
| `INDICATOR.SMA` | Simple moving average; `source`=close, `period`=20 | No |
| `INDICATOR.EMA` | Exponential moving average; `source`=close, `period`=20 | No |
| `INDICATOR.WMA` | Weighted moving average; `source`=close, `period`=20 | No |
| `INDICATOR.HMA` | Hull moving average; `source`=close, `period`=20 | No |
| `INDICATOR.RMA` | Running/Wilder moving average; `source`=close, `period`=20 | No |
| `INDICATOR.RSI` | Relative Strength Index; `source`=close, `period`=14 | No |
| `INDICATOR.ROC` | Rate of change; `source`=close, `period`=14 | No |
| `INDICATOR.RVOL` | Relative volume; `period`=14 | No |
| `INDICATOR.HIGHEST` | Highest source value; `source`=close, `period`=14 | No |
| `INDICATOR.LOWEST` | Lowest source value; `source`=close, `period`=14 | No |
| `INDICATOR.TR` | True range from OHLC | No |
| `INDICATOR.ATR` | Smoothed true range; `period`=14, `maSmoothingType`=`RMA` | No |
| `INDICATOR.STDDEV` | Standard deviation; `source`=close, `period`=20 | No |
| `INDICATOR.BB` | `maSource`=close, `period`=20, `stddev`=2, `maType`=`SMA` | middle/upper/lower bands |
| `INDICATOR.KC` | `maSource`=close, `period`=20, `multiplier`=1.5, `bandAtrPeriod`=14, `bandMethodType`=`ATR`, `maMethodType`=`EMA` | middle/upper/lower bands |
| `INDICATOR.DC` | Donchian high/low channel; `period`=20 | upper/lower bands |
| `INDICATOR.MACD` | `source`=close, `fastPeriod`=12, `slowPeriod`=26, `signalPeriod`=9 | MACD line/signal line/histogram |
| `INDICATOR.STOCH` | `kPeriod`=5, `kSmooth`=3, `dPeriod`=3 | K percent/D percent |
| `INDICATOR.STOCH_RSI` | `rsiSource`=close, `rsiPeriod`=14, `kPeriod`=5, `kSmooth`=3, `dPeriod`=3 | K percent/D percent |

Use `OPERATOR.SELECT_OUTPUT` to consume one output of a multi-output indicator.
Period-like parameters are numeric expressions, commonly unsigned
`INPUT.NUMERIC` nodes. Moving-average enum values are `SMA`, `EMA`, `RMA`,
`WMA`, and `HMA` where the field supports them.

## Numeric operators

| ID | Output and parameters | Context |
| --- | --- | --- |
| `OPERATOR.CHANGE` | Current minus previous source; `source` defaults to close | Strategy |
| `OPERATOR.LOOKBACK` | Source at integer `period` bars back; source defaults to close | Strategy, comparator |
| `OPERATOR.ADD` | `augend + addend` | Strategy, comparator |
| `OPERATOR.SUBTRACT` | `minuend - subtrahend` | Strategy, comparator |
| `OPERATOR.MULTIPLY` | `multiplicand * multiplier` | Strategy, comparator |
| `OPERATOR.DIVIDE` | `dividend / divisor` | Strategy, comparator |
| `OPERATOR.NEGATE` | `-operand` | Strategy, comparator |
| `OPERATOR.ABS` | `abs(operand)` | Strategy, comparator |
| `OPERATOR.ABS_DIFF` | Absolute difference; Strategy keys `minuend`/`subtrahend`, comparator keys `left`/`right` | Strategy, comparator |
| `OPERATOR.SQRT` | `sqrt(operand)` | Strategy, comparator |
| `OPERATOR.MAX` | Maximum of `left` and `right` | Strategy, comparator |
| `OPERATOR.MIN` | Minimum of `left` and `right` | Strategy, comparator |
| `OPERATOR.POSITIVE_PART` | `max(operand, 0)` | Strategy, comparator |
| `OPERATOR.NEGATIVE_PART` | `max(-operand, 0)` | Strategy, comparator |
| `OPERATOR.PERCENTAGE` | `base * percent / 100`; defaults close and 100 | Strategy |
| `OPERATOR.SELECT_OUTPUT` | Select named `output` from `source`; source defaults to close | Strategy |

`OPERATOR.SELECT_OUTPUT.output` accepts `default`, `macd-line`, `signal-line`,
`histogram`, `k-percent`, `d-percent`, `middle-band`, `upper-band`, and
`lower-band`. Choose an output supported by its source.

## Conditions

| ID | Result and parameters | Notes |
| --- | --- | --- |
| `COMPARISON.LESS_THAN` | `target < threshold` | Numeric pair |
| `COMPARISON.GREATER_THAN` | `target > threshold` | Numeric pair |
| `COMPARISON.LESS_EQUAL` | `target <= threshold` | Numeric pair |
| `COMPARISON.GREATER_EQUAL` | `target >= threshold` | Numeric pair |
| `COMPARISON.EQUAL` | `target == threshold` | Numeric pair |
| `COMPARISON.NOT_EQUAL` | `target != threshold` | Numeric pair |
| `COMPARISON.ABOVE_BY` | `target >= reference + gap` | `gap` uses the same units as `target` and `reference` |
| `COMPARISON.ABOVE_BY_PERCENT` | `target >= reference + reference * percent / 100` | `percent` uses percentage points |
| `COMPARISON.CROSSOVER` | Value crosses from at/below to above reference | Requires previous values |
| `COMPARISON.CROSSUNDER` | Value crosses from at/above to below reference | Requires previous values |
| `LOGIC.ALL_OF` | All child conditions | Variable-length list |
| `LOGIC.ANY_OF` | Any child condition | Variable-length list |
| `LOGIC.NOT` | Logical negation | One condition |
| `LOGIC.AND` | Both conditions | Two conditions |
| `LOGIC.OR` | Either condition | Two conditions |
| `LOGIC.XOR` | Exactly one condition | Two conditions |
| `LOGIC.ALWAYS` | Always true | No parameters |
| `LOGIC.NEVER` | Always false | No parameters |

Both Above By comparisons accept numeric expressions for each operand and
include equality at the threshold. A positive gap requires the target to exceed
the reference. Each returns `false` if an operand or the threshold is non-finite,
including during indicator warm-up.

All conditions are available to Strategy signals. Entry Filters accept the six
basic numeric comparisons listed first and `LOGIC.AND`, `LOGIC.OR`, `LOGIC.XOR`,
`LOGIC.NOT`, `LOGIC.ALWAYS`, and `LOGIC.NEVER`. Entry Filter scalar leaves are
constants, Portfolio state, Requested Order values, or
`STRATEGY_PERFORMANCE.VALUE`.

## Risk, stops, targets, and position state

| ID | Output and principal parameters |
| --- | --- |
| `RISK_DISTANCE.AMOUNT` | Fixed distance; `amount` default 1 |
| `RISK_DISTANCE.PERCENT` | Entry-price distance; `percentage` default 1 |
| `RISK_DISTANCE.ATR` | ATR distance; `period`=14, `multiplier`=2, smoothing=`RMA` |
| `STOP_LOSS.AMOUNT` | Stop an `amount` from reference; default 0 |
| `STOP_LOSS.PERCENT` | Stop a `percent` from reference; default 0 |
| `STOP_LOSS.ATR` | ATR stop; `period`=14, `multiplier`=2, smoothing=`RMA` |
| `STOP_LOSS.ONE_R` | Stop at one risk distance |
| `STOP_LOSS.R_MULTIPLE` | Stop at `multiple` R; default 1 |
| `TAKE_PROFIT.AMOUNT` | Target an `amount` from reference; default 0 |
| `TAKE_PROFIT.PERCENT` | Target a `percent` from reference; default 0 |
| `TAKE_PROFIT.ATR` | ATR target; `period`=14, `multiplier`=2, smoothing=`RMA` |
| `TAKE_PROFIT.R_MULTIPLE` | Target at `multiple` R; default 2 |
| `POSITION.INITIAL_ENTRY_PRICE` | First fill price |
| `POSITION.LATEST_ENTRY_PRICE` | Most recent layer fill price |
| `POSITION.AVERAGE_PRICE` | Quantity-weighted average entry price |
| `POSITION.STOP_TARGET_REF_PRICE` | Current reference selected for stop/target evaluation |
| `POSITION.DIRECTION` | `1` for long and `-1` for short |
| `POSITION.PYRAMIDING_LAYER` | Accepted layer count; `0` while flat |
| `POSITION.R_MULTIPLE` | Directional movement of `source` (default close) from the risk reference, divided by frozen one-R distance |

These are Strategy-context nodes. Values requiring a live position are meaningful
only while the relevant theoretical/actual position state exists.

## Portfolio Requested Order values

These scalar nodes exist in Portfolio comparator expressions and Strategy Entry
Filters. Entry Filters evaluate them for the candidate initial order, so
`REQUESTED_ORDER.IS_PYRAMIDING` is false there and frozen unit quantity is not
yet available.

| ID | Output |
| --- | --- |
| `REQUESTED_ORDER.PRICE` | Requested executable entry price |
| `REQUESTED_ORDER.DIRECTION` | Long/short direction |
| `REQUESTED_ORDER.IS_PYRAMIDING` | 1 for a pyramid request, otherwise 0 |
| `REQUESTED_ORDER.RAW_QUANTITY` | Quantity before later constraints |
| `REQUESTED_ORDER.RAW_QUANTITY_LIMIT` | Pre-adjustment sizing limit |
| `REQUESTED_ORDER.DRAWDOWN_ADJUSTED_QUANTITY` | Quantity after drawdown adjustment |
| `REQUESTED_ORDER.DRAWDOWN_ADJUSTED_QUANTITY_LIMIT` | Limit after drawdown adjustment |
| `REQUESTED_ORDER.QUANTITY` | Final normalized requested quantity |
| `REQUESTED_ORDER.NOTIONAL` | Price × quantity |
| `REQUESTED_ORDER.COST` | Cash cost including estimated entry fee |
| `REQUESTED_ORDER.ESTIMATED_ENTRY_FEE` | Estimated fee to enter |
| `REQUESTED_ORDER.ESTIMATED_1R_EXIT_FEE` | Estimated fee at the 1R risk exit |
| `REQUESTED_ORDER.RISK_DISTANCE` | Strategy-defined one-R price distance |
| `REQUESTED_ORDER.PRICE_RISK` | Quantity × risk distance |
| `REQUESTED_ORDER.RISK_WITH_FEES` | Price risk plus relevant fee estimates |
| `REQUESTED_ORDER.FROZEN_UNIT_QUANTITY` | Unit quantity frozen for Strategy pyramiding |

`COST = NOTIONAL + ESTIMATED_ENTRY_FEE`. `PRICE_RISK = QUANTITY ×
RISK_DISTANCE`. `RISK_WITH_FEES = PRICE_RISK + ESTIMATED_ENTRY_FEE +
ESTIMATED_1R_EXIT_FEE`. Values that do not apply to a candidate request are
non-finite; expressions must not assume every optional limit or frozen quantity
exists.

## Entry Filter and Comparator Strategy Performance

`STRATEGY_PERFORMANCE.VALUE` accepts a `metric` and returns the current value from
the Strategy setup's theoretical Strategy Performance tracker. A System shares
the calculation/prior configuration, not one mutable result history. Entry
Filters evaluate it for candidate initial orders. Comparators evaluate it for
initial and pyramiding requests from the immutable snapshot captured for each
requesting Strategy setup during request discovery, before ranking. That
snapshot contains only evidence visible in the current execution phase; it
cannot observe later ranking or execution outcomes. The 20 metric values are:

| Counts and rates | Streaks | Returns and Bayesian estimates |
| --- | --- | --- |
| `LIFETIME_COUNT` | `CURRENT_WINNING_STREAK` | `MEAN_RETURN` |
| `EFFECTIVE_COUNT` | `CURRENT_LOSING_STREAK` | `RETURN_STANDARD_DEVIATION` |
| `WIN_RATE` | `MAXIMUM_WINNING_STREAK` | `BAYESIAN_WIN_PROBABILITY` |
| `BREAK_EVEN_RATE` | `MAXIMUM_LOSING_STREAK` | `BAYESIAN_WIN_PROBABILITY_LOWER_95` |
| `LOSS_RATE` |  | `BAYESIAN_WIN_PROBABILITY_UPPER_95` |
|  |  | `BAYESIAN_WINNING_PAYOFF` |
|  |  | `BAYESIAN_WINNING_PAYOFF_LOWER_95` |
|  |  | `BAYESIAN_WINNING_PAYOFF_UPPER_95` |
|  |  | `BAYESIAN_LOSING_PAYOFF` |
|  |  | `BAYESIAN_LOSING_PAYOFF_LOWER_95` |
|  |  | `BAYESIAN_LOSING_PAYOFF_UPPER_95` |

## Plot methods

Plots are methods but are not numeric editor nodes:

| ID | Parameters and output |
| --- | --- |
| `PLOT.HLINE` | `level`=0 and `color`=white |
| `PLOT.LINE` | `source` and `color`=white |
| `PLOT.HISTOGRAM` | `source` and `color`=white |
| `PLOT.MOMENTUM_HISTOGRAM` | `source` plus required `positiveRisingColor`, `positiveFallingColor`, `negativeFallingColor`, and `negativeRisingColor` |
| `PLOT_SOURCE.SERIES` | Reads named series `name`; default empty |
| `PLOT_SOURCE.CONSTANT` | Constant plot `value`; default 0 |

Colors accept a supported color string or an unsigned packed color value.

See [Strategy JSON](strategy-json.md) for document structure and exact serialized
spelling.

## Completeness and stability

This page inventories the methods currently registered by the
Strategy, comparator, Entry Filter, and plot serializers. Validate hand-authored
documents with the bundled schema and the Strategies import preview before relying
on them in an experiment.
