# Turtle Trading Guidelines for Pludux

## Scope

This guide defines the closest practical Turtle Trading configuration in
Pludux. It assumes familiarity with Turtle terminology and concentrates on
configuration, runtime behavior, and fidelity boundaries.

The result is a Turtle-style daily-bar backtest, not a claim that Pludux
reproduces a historical execution record. Pludux closely represents the
mechanical strategy and risk relationships, while some original futures,
intraday, portfolio, and discretionary execution rules remain approximations or
are unsupported.

The most faithful practical use today is either:

- a stock or spot-market adaptation, where one asset unit has a dollar value
  equal to its quoted price; or
- research with externally prepared futures data whose contract economics are
  deliberately transformed for Pludux.

Pludux does not currently provide a native futures contract multiplier, margin,
currency conversion, expiry, or rollover model.

## Configuration Map

| Turtle component | Pludux representation | Fidelity |
| --- | --- | --- |
| Liquid market universe | System Watchlist selected in advance | Manual |
| `N`: 20-day Wilder average of True Range | 1R ATR Distance, period 20, multiplier 1, RMA | Close |
| One Unit has 1N dollar volatility equal to 1% of equity | Profile Risk Distance sizing at 1% equity risk | Close |
| Turtle System 1: 20-day breakout | Main Strategy using Donchian 20 | Close on daily bars |
| Skip Turtle System 1 after a winning theoretical breakout | Main Strategy Entry Filter using theoretical Model Performance | Close |
| Turtle System 2 failsafe: 55-day breakout | Failsafe Strategy binding with persistent filtered-position activation | Close |
| Add one Unit every 0.5N | Position R-Multiple threshold of 0.5 | Close, one layer per bar |
| Maximum four Units in one market | Model maximum pyramiding layers of 4 | Direct |
| Freeze Unit and N while building a position | Frozen initial executed quantity and risk distance | Direct |
| Normal stop: 2N from the latest Unit | Stop Loss R-Multiple 2 using Latest Entry Price | Close |
| System 1 exit: adverse 10-day breakout | Donchian 10 full exit | Close on daily bars |
| System 2 exit: adverse 20-day breakout | Donchian 20 full exit | Close on daily bars |
| Correlation and directional Unit limits | Maximum combined layers and Portfolio separation | Approximate |
| Reduce notional account after drawdown | Profile Notional Equity Reduction | Turtle-inspired |
| Buy strength and sell weakness | Direction-adjusted Portfolio strength comparator | Close on daily bars |
| Limit-order and fast-market tactics | Broker fees and bar execution assumptions | Not equivalent |

`N`, `1R`, and the campaign risk distance refer to the same value in this
configuration. The initial executed layer establishes both the frozen Unit
quantity and frozen `R` for that open position.

## Configuration Overview in One Pludux System

Use four configuration levels:

```text
Portfolio
|- Shared capital, Market, and Broker
|- Portfolio capacity and entry ordering
   `- Pludux System
    |- Watchlist
    |- Shared Model Performance calculation
   |- Main Strategy: Turtle System 1
   |  |- Turtle System 1 Model
    |  |- Turtle Profile
    |  `- Previous-breakout Entry Filter
   `- Failsafe Strategy binding: Turtle System 2
        |- Previous setup has an entry-filtered theoretical position
       |- Turtle System 2 Model
       |- Turtle Profile
       `- Always-allow Entry Filter
```

The Main and Failsafe Strategy bindings keep independent theoretical positions
and Model Performance histories. They share the System's calculation policy,
but not accumulated runtime state. The Portfolio executes at most one real
position for each expanded Pludux System and Watchlist asset, and records which
Strategy owns it.

## 1. Market Data and Watchlist

Use daily OHLCV data with enough warm-up history for the 55-day channel and
20-day RMA ATR. Donchian periods count available bars, not calendar days.

Select a stable Watchlist before running the backtest. The original Turtles
traded a broad set of sufficiently liquid futures and were instructed not to
trade a chosen market inconsistently. Pludux does not automatically screen
liquidity or roll futures contracts, so data preparation and universe selection
remain the user's responsibility.

For a stock adaptation, configure the Market with:

- Minimum order quantity: `1`
- Quantity step: `1`

This makes the Risk Distance constraint round down to whole shares while
staying inside the risk budget. Configure Broker fees to reflect the intended
venue. Pludux does not model slippage. Broker fees can make the final Unit
slightly smaller than the textbook formula.

## 2. Turtle Profile

Create a reusable Profile with the following settings.

### Position Sizing

```text
Method:       Risk Distance
Equity Risk:  1%
```

For a stock with equity `E`, entry price `P`, and `N = ATR(20, RMA)`, the
textbook Unit is:

\[
Q_{raw}=\frac{0.01E}{N}
\]

For futures, the original formula was:

\[
Q_{raw}=\frac{0.01E}{N\times\text{dollars per point}}
\]

Pludux's stock formula implicitly has one dollar per point per share. Native
futures dollars-per-point conversion is not currently modeled.

Pludux applies a stricter fee-aware constraint. The final requested quantity is
the largest Market-valid quantity for which the adverse 1R price loss plus the
estimated entry and 1R exit fees fit inside the 1% risk budget. Therefore, with
fees, the result can be lower than simply truncating `Q_raw`.

The original 2N protective stop is configured separately. Consequently, one
full initial Unit stopped at 2R has approximately 2% price risk, plus any
difference between estimated and realized fees or opening gaps. The
Requested Order's `Requested Risk With Fees` diagnostic is a 1R diagnostic; it
does not describe the configured 2R protective stop.

### Drawdown Adjustment

Use the following Turtle-inspired settings:

```text
Enabled:                    true
Drawdown step:              10%
Size reduction:              0%
Notional equity reduction:  20%
```

At each initial entry, Pludux calculates:

\[
steps=\left\lfloor\frac{currentDrawdownFromPeak}{10\%}\right\rfloor
\]

\[
notionalEquity=peakEquity\times\max(1-steps\times20\%,0)
\]

Risk Distance sizing uses this notional equity. Size Reduction remains zero, so
the resulting quantity is not reduced a second time.

This is deliberately not the original Turtle drawdown schedule. The original
rules used a yearly reference and reduced the current notional account by 20%
after losses equal to 10% of that current notional amount. A USD 1,000,000
notional account therefore moved to USD 800,000 at a USD 100,000 loss and then
to USD 640,000 after another USD 80,000 loss. The reduced notional was retained
until recovery to the yearly starting equity.

Pludux instead uses current drawdown from peak. Its levels are 80% of peak at a
10% drawdown and 60% at a 20% drawdown. The level recovers immediately when
current drawdown improves enough to cross a step boundary, and a new equity peak
becomes the next reference. This behavior is systematic and peak-aware, but it
is not historically identical to the original persistent, compounding schedule.

### Insufficient Cash

```text
Policy: Reject
```

Reject is the closest Turtle interpretation because it does not turn a Unit
into an arbitrary partial Unit. `Cap To Available Cash` is useful for cash-only
stock portfolios, but a cash-capped partial layer still counts as one executed
layer in Pludux and does not redefine the frozen Unit for later additions.

## 3. Shared Strategy Building Blocks

Create these named series or their equivalent node trees in both Strategies:

```text
N             = ATR(period: 20, smoothing: RMA)
Entry channel = Donchian Channel(entry period)
Exit channel  = Donchian Channel(exit period)
```

Configure both long and short position risk distance as:

```text
1R Distance: ATR
Period:      20
Multiplier:  1
Smoothing:   RMA
```

The RMA implementation starts with a 20-bar simple average and then uses an
alpha of `1 / 20`, matching the Turtle `N` recurrence.
Unlike the weekly Unit sheets described in the original rules, Pludux evaluates
the current `N` whenever a new initial Requested Order is created. After a real
initial fill, that `N` and the final executed Unit quantity remain frozen for
the position.

## 4. Turtle System 1 Strategy

### Entry

Use a 20-bar Donchian channel.

```text
Long:  current upper band > previous bar's upper band
Short: current lower band < previous bar's lower band
```

Because the current Donchian band includes the current bar, comparing it with
its one-bar lookback detects a new 20-bar extreme. It is the daily-bar analogue
of exceeding the preceding 20-day extreme by one tick.

Choose one timing policy and use it consistently:

- `Next Open` is the recommended auditable backtest setting. It observes the
  completed breakout bar and trades the next available open.
- `Current Close` trades on the breakout bar's close. It is closer in calendar
  time, but the close is not the original intraday breakout price.

Neither option exactly reproduces the original rule, which traded when the
level was crossed intraday and entered at the open when the market gapped
through the breakout.

### Previous-breakout Entry Filter

The System 1 setup must continue tracking every theoretical breakout even when
real execution is filtered. Configure its Entry Filter as the equivalent of:

```text
Model Performance Lifetime Count == 0
OR
Model Performance Current Losing Streak >= 1
```

This allows the first theoretical breakout and allows later breakouts only when
the most recently completed theoretical System 1 position lost. A winning most
recent position resets Current Losing Streak to zero, so the next System 1
Requested Order is entry-filtered.

Use `All History` for the Pludux System Model Performance history policy. Current
winning and losing streaks are lifetime chronological state in Pludux, so they
do not decay or roll out under other history modes; `All History` nevertheless
makes the intent of this configuration clearest.

The original rules classified the last breakout by whether a 2N adverse move
occurred before a profitable 10-day exit, regardless of whether that breakout
was actually traded. Pludux's independent theoretical System 1 position and
performance history reproduce that essential shadow-trade behavior. Exact
break-even edge cases and differences caused by bar execution or the modeled
multi-layer theoretical position can still classify a marginal trade
differently.

### Exit

Configure a full exit:

```text
Long:  current 10-bar lower band < previous 10-bar lower band
Short: current 10-bar upper band > previous 10-bar upper band
Reduce: 100%
```

Disable profit targets. Turtle trend exits deliberately allowed substantial
open profit to be given back in exchange for remaining in large trends.

## 5. Turtle System 2 Strategy and Failsafe

Create Turtle System 2 with the same risk, stop, and pyramiding rules as Turtle System 1, but
use:

```text
Entry channel: 55 bars
Exit channel:  20 bars
Entry Filter:  Always
```

Add it as `Failsafe 1` in the same System and select:

```text
Activation: Previous setup has an entry-filtered theoretical position
```

This activation is persistent. When Turtle System 1 produces a fresh breakout that is
rejected by its Entry Filter, Turtle System 1 still opens its theoretical position and
the marker remains active until that theoretical position fully closes. Turtle System
2 may execute during that interval, but only when Turtle System 2 produces its own
fresh 55-day entry signal. An old Turtle System 2 shadow position is never converted
into a real position merely because activation changes.

This is the closest Pludux representation of the Turtle System 1 failsafe rule. A
Failsafe is internal to the Pludux System; it does not create another Portfolio row
or another simultaneous real position for that Pludux System and asset.

The original Turtles could also allocate capital to Turtle System 2 as an independent
model rather than only as a Turtle System 1 failsafe. To study that behavior, add a
separate Pludux System whose Main Strategy uses the Turtle System 2 Model. If Turtle System 1 and
Turtle System 2 are intended to split one risk allocation, reduce their Profile risk
fractions accordingly; otherwise each setup can independently request a full
1% Unit from the same Portfolio equity.

## 6. Pyramiding

Configure both long and short positions as follows:

```text
Signal:       Position R-Multiple >= 0.5
Retrigger:    Every Evaluation
Cooldown:     0
Max layers:   4
```

Use High as the Position R-Multiple source for long signals and Low for short
signals if the goal is to detect that the bar touched the 0.5R threshold. Use
Close instead for a close-confirmed variant. The selected entry timing still
determines the actual fill price.

Set both favorable and unfavorable stop/target references to:

```text
Latest Entry Price
```

The first successful real execution captures:

- the final executed, Market-normalized quantity as the frozen Unit; and
- the evaluated 1R distance as the frozen `N`.

Every later layer requests that same absolute Unit quantity and uses the same
`N`. It does not reevaluate equity, drawdown adjustment, Profile sizing, or the
risk-distance expression. A rejected attempt does not add a layer. Entry Filters
apply only to fresh initial entries and are not reevaluated for pyramids. A
cash-capped execution adds a layer but does not replace the original frozen
Unit. Partial exits do not release layers or change the next requested Unit;
full closure resets the layer count, Unit, and `N` for the next position.

Position R-Multiple is measured directionally from the position's current risk
reference using frozen `N`. With Latest Entry Price selected, each successful
addition resets that reference to its actual fill. The next layer therefore
requires another favorable 0.5N move from the preceding fill, so an opening
gap and its actual fill affect later thresholds.

Pludux executes no more than one pyramiding layer for an expanded
Pludux System-asset run in one bar. The original Turtles could add all remaining
Units during a single fast-moving day. This can materially change exposure,
average price, and stop behavior in fast trends.

## 7. Protective Stops

Configure one full-position stop for each direction:

```text
Enabled:     true
Stop price:  Stop Loss R-Multiple
R-Multiple:  2
Trailing:    false
Reduce:      100%
Reference:   Latest Entry Price
```

For a long position:

\[
stop=latestEntryPrice-2N
\]

For a short position:

\[
stop=latestEntryPrice+2N
\]

Do not enable continuous trailing. The normal Turtle stop changed when a Unit
was added; it did not trail every favorable bar. Pludux recalculates the stop
after a successful addition and prevents it from moving in an adverse
direction. Rejected additions do not move it.

With regular 0.5N spacing, this reproduces the normal Turtle ratchet: every new
Unit advances the full-position stop by 0.5N. For example, with a long entry at
100 and `N = 2.50`:

| Executed layer | Fill | Full-position stop |
| ---: | ---: | ---: |
| 1 | 100.00 | 95.00 |
| 2 | 101.25 | 96.25 |
| 3 | 102.50 | 97.50 |
| 4 | 103.75 | 98.75 |

The original rules allowed different stops for individual Units after a large
gap. Earlier Unit stops could remain at the prior common
level while the newly gapped Unit received its own 2N stop. Pludux maintains a
shared execution position and shared stop level, so Latest Entry Price applies
the new stop to the entire position. That can be more aggressive than the
original gap-specific handling.

The alternate Turtle `Whipsaw` method is not represented precisely. It used
independent 0.5N stops for each Unit and re-entered a stopped Unit at its
original entry price. Pludux does not currently maintain independently stopped
sub-positions and re-entry prices for every layer.

## 8. Portfolio Limits

The original limits were:

| Scope | Original maximum |
| --- | ---: |
| Single market | 4 Units |
| Closely correlated markets, one direction | 6 Units |
| Loosely correlated markets, one direction | 10 Units |
| All markets in one direction | 12 Units |

Use Pludux System maximum layers of 4 for the single-market rule.

Use Portfolio `Maximum combined layers` as an explicit approximation for the
desired aggregate scope. It counts every successful initial entry and pyramid
across open positions. Long and short layers share one pool. Partial exits
release no layers; full execution closure releases all layers held by that
position.

There is no exact value that reproduces the original directional limits because
the Turtles could hold 12 long Units and 12 short Units simultaneously. A
Pludux limit of 12 is a conservative shared cap; a limit of 24 can permit the
same gross total but can also permit 24 Units in one direction.

For correlation-group research, separate related assets into separate
Portfolios and choose a combined-layer limit such as 6 or 10. This follows the
user-defined grouping model, but each Portfolio is an independent account.
Pludux does not enforce overlapping close-correlation, loose-correlation, and
directional limits against one shared account at the same time.

`Maximum open trades` has no direct Turtle equivalent. Set it high enough that
the intended Unit limit, rather than the position count, is the binding rule.
When both limits block an initial entry, Maximum Open Trades has precedence.

## 9. Simultaneous Signals and Ordering

The original Turtles bought the strongest markets and sold short the weakest
when simultaneous signals competed for limited capacity. Some used movement in
`N` since breakout or a three-month price change divided by current `N`.

Pludux ranks immutable Requested Orders at each Next Open or Current Close
phase. Portfolio comparators can combine Requested Order values with the
order's asset OHLCV fields, custom `DATA` fields, lookbacks, constants, and
basic scalar math. They cannot inspect technical-indicator nodes or named
Model series.

For a practical three-month strength ranking, add this Portfolio Entry
Comparator and select **Higher First**:

```text
Requested Order Direction
* (Requested Order Price - Lookback(Close, 63))
/ Requested Order Risk Distance
```

The 63-bar lookback approximates three trading months on daily data. For a long
order, rising prices produce a positive score. For a short order, direction is
`-1`, so falling prices also produce a positive score. Normalizing by Requested
Order Risk Distance expresses the movement in current `N` units and makes
different markets more comparable.

Use sufficient warm-up history for the lookback. If a score is unavailable or
non-finite, every finite score ranks ahead of it. Equal scores continue to the
next configured comparator, and complete ties use Portfolio, Pludux System, and
Watchlist order.

Asset visibility follows the execution phase:

- At Current Close, OHLCV and `DATA` refer to the completed current bar.
- At Next Open, OHLCV and `DATA` refer to the previous completed bar. Requested
  Order Price is the current executable open price.

The alternative strength interpretation based on movement from the exact
breakout price is not represented precisely. Pludux does not detect and fill a
Donchian entry at its intraday crossing price. Requested Order Price is the
configured Current Close or Next Open execution price, not necessarily the
historical breakout threshold or first traded breakout price.

Every Requested Order in a phase is built from the same pre-entry account
snapshot. Ranking values remain immutable. Cash and capacity are then refreshed
and enforced sequentially in ranked order, so a rejected order does not consume
capacity and the next order may still execute.

## 10. Worked Stock Example

Assume:

```text
Portfolio equity:  USD 100,000
Entry price:       USD 100.00
ATR(20, RMA):      USD 2.50
Equity risk:       1%
Quantity step:     1 share
Fees:              ignored for this example
```

Then:

\[
Unit=\frac{100,000\times0.01}{2.50}=400\text{ shares}
\]

The initial 2N stop is USD 95.00. Favorable additions are requested every USD
1.25 from the actual preceding fill, up to four layers. With exact fills, the
sequence is:

```text
Layer 1: 400 shares at 100.00; stop 95.00
Layer 2: 400 shares at 101.25; stop 96.25
Layer 3: 400 shares at 102.50; stop 97.50
Layer 4: 400 shares at 103.75; stop 98.75
```

In a real Pludux run, Broker fees may reduce the initial requested quantity.
Whatever quantity is actually executed for the initial layer becomes the
frozen Unit requested by all later layers.

## 11. Execution Differences That Matter

### Intraday breakout fills

The Turtles entered and exited when the market traded through a channel level,
including gap-open handling. Pludux Model signals are evaluated at Current
Close or scheduled for Next Open. OHLC price stops can be processed inside a
bar using the configured intrabar path, but Donchian entry and exit signals are
not continuously evaluated and filled at their exact intraday threshold.

### One layer per bar

Pludux adds at most one Unit per bar. The original rules allowed several 0.5N
thresholds and all four Units to execute in one fast day.

### Fill tactics and market impact

The original Turtles used judgment around near-market limit orders, fast
markets, liquidity, and large-order market impact. Pludux uses deterministic
Market normalization, Broker fees, and bar prices. It does not reproduce the
original order-working tactics.

### Futures economics and rollover

The original system traded liquid futures, selected contract months, and
rolled positions before liquidity deteriorated. Pludux has no native contract
multiplier, margin, open-interest, expiry, or rollover policy. Continuous
futures data prepared outside Pludux can introduce artificial roll gaps that
affect Donchian channels and `N` unless adjusted carefully.

### Portfolio risk structure

Maximum combined layers is a shared long-and-short count. It does not model the
original overlapping market, correlation, and direction-specific Unit limits.
Separate Portfolios do not share capital or capacity.

### Drawdown schedule

Pludux's peak-based adjustment is dynamic and non-persistent. The original
notional account reductions were persistent until recovery to the yearly
starting level and used compounding thresholds.

### Weekly Unit sheets

The original Turtles received Unit sizes on weekly sheets. Pludux calculates
`N` and initial Unit size at each new initial order, then freezes both for that
position. It does not provide a weekly cross-market Unit snapshot.

### Per-Unit gap stops and Whipsaw

Pludux uses one real position and shared stop state. It does not preserve
different stop levels or re-entry state for individual Units.

### Historical outcome sensitivity

Turtle System 1 filtering uses Pludux's completed theoretical position return. Daily
execution timing, pyramiding, shared stop behavior, and exact break-even cases
can change whether a marginal theoretical position is classified as a winner
or loser compared with an original tick-by-tick reconstruction.

## 12. Validation Checklist

Before treating a run as a Turtle-style result, verify:

- Daily data has sufficient warm-up and no unintended roll or split gaps.
- Both Strategies use ATR 20, multiplier 1, and RMA for 1R distance.
- The Profile uses Risk Distance sizing at 1%.
- Drawdown adjustment is either disabled for a pure sizing comparison or set to
  10% step, 0% Size Reduction, and 20% Notional Equity Reduction.
- Insufficient Cash is Reject unless partial cash-only Units are intentional.
- Turtle System 1 uses 20-day entry and 10-day exit channels.
- Turtle System 1 Entry Filter allows the first trade or a current losing streak.
- Turtle System 2 uses 55-day entry and 20-day exit channels.
- Failsafe 1 activation is Previous Setup Entry-Filtered Position.
- Pyramiding uses 0.5 Position R-Multiple, Every Evaluation, no cooldown, and
  four maximum layers.
- Stop Loss R-Multiple is 2, full reduction, non-trailing, and references Latest
  Entry Price.
- Take profits are disabled.
- Portfolio Maximum Open Trades is not unintentionally binding.
- Maximum Combined Layers and Portfolio grouping are documented as the chosen
  approximation, not mistaken for the original four simultaneous limits.
- The Portfolio strength comparator uses Higher First and direction-adjusted
  three-month price movement divided by Requested Order Risk Distance.
- The strength lookback has sufficient warm-up history, and any non-finite
  ranking scores are reviewed.
- Entry timing and High/Low versus Close signal sources are recorded with the
  results.
- Sensitivity runs compare Current Close and Next Open when execution timing is
  material.

## Interpretation

The setup above preserves the most important Turtle relationships:

- volatility-normalized initial risk;
- a frozen Unit and `N` while adding to a position;
- additions at favorable 0.5N intervals;
- a four-Unit single-market maximum;
- 2N protective stops ratcheted from actual fills;
- independent theoretical Turtle System 1 tracking;
- persistent Turtle System 2 failsafe eligibility after a filtered Turtle System 1 entry;
- direction-adjusted strength ordering for simultaneous Requested Orders;
- long-duration Donchian exits; and
- shared Portfolio admission after orders are sized and filtered.

The resulting backtest should be described as a **Pludux Turtle
representation** or **Turtle-style daily-bar implementation**, not as an exact
reproduction of the historical implementation. The unsupported details are most
consequential during gaps, fast intraday trends, simultaneous correlated
signals, futures roll periods, and drawdown regime transitions.
