# Execution and Position Lifecycle

This page describes the lifecycle configured by a Strategy, System, and
Portfolio. [Portfolio Backtesting](portfolio-backtesting.md) remains
authoritative for portfolio clock and valuation behavior.

## Intrabar Path

Set **Intrabar Path** in the Portfolio editor. The Portfolio applies the same
rule to each Asset candle across its Systems and Strategy setups, including
hypothetical trades. The rule supplies a deterministic price sequence from
each candle's OHLC prices for custom entries, signal exits, pyramiding, stop
losses, trailing stops, and take profits.

- **Low First** follows open → low → high → close.
- **High First** follows open → high → low → close.
- **Candle Direction** follows Low First on an up candle and High First on a
  down candle. A doji visits the extreme nearest the open first, with ties
  going to the low.

**Candle Direction** is the default for new Portfolios and older workspaces
that have no Portfolio Intrabar Path setting.

If the open gaps through a pending stop or target, the first reachable price on
the path can be the open rather than the configured level. Pludux has no
separate slippage model.

## Signal prices and delays

Signal entries, signal exits, and pyramiding each use **Price** and **Delay**.
Delay looks back for the signal in real candles on the same Asset. Delay 0 uses
the current candle's signal and Delay 1 uses the previous real candle's signal.
A timestamp where only another Asset has a candle does not count toward the
delay.

**Price** uses the standard Strategy method editor. The **Open** and **Close**
methods used directly fill at the corresponding event of the execution candle.
A price from another method must fall within that candle's low–high range,
including the endpoints. When the rule is eligible, it can fill at the first
touch along the Portfolio's Intrabar Path. If the candle misses the price, the
attempt ends with that candle.

The expression can use completed OHLC values for the Asset's current candle.
If `Close × 0.95` returns 95 and the candle touched 95 earlier, a qualifying
rule can fill at that earlier touch. With Delay 0, the current candle's signal
can qualify the touch. Sizing, filters, ranking, and protective orders still
use data visible in the execution phase. New signal rules default to Open with
Delay 1, which uses the previous candle's signal at the current open.

## Initial entries

The active setup evaluates its Strategy signal, entry price, and risk distance.
The System's Profile sizes the Requested Order and the setup's Entry Filter
decides whether to submit it. The Portfolio ranks fresh requests with its
comparators and admits them while capital and capacity permit. Complete ties
use System order and then Watchlist Asset order.

### Signal entries

**Price Exits Start** sets when the position's stop losses, trailing stops,
and take profits become active. **Current Bar** starts them after the fill,
so they can act on the remaining path of the entry candle. **Next Bar** starts
them at the Asset's next real open. Both settings use the same entry rule and
the same stop, target, and trailing calculations.

New signal entries use Open, Delay 1, and Current Bar. Strategies saved with
**Next Open** use the same settings. Strategies saved with **Current Close**
use Close, Delay 0, and Next Bar.

### Call entries

Each Strategy side can use **Signal** (the default) or **Call** as its entry
source. **Call** uses the table row's direction and tests the open of the real
candle with the exact Asset and datetime. The attempt happens once at that
open. When an entry bound is set, the open must satisfy it inclusively. A row
whose open is outside a bound is skipped.

At a Call open, Strategy expressions read market data from the previous
completed candle. The executable open price and the candidate Call row are
available for entry and sizing, including on the first candle. A Call order
goes through regular setup activation, sizing, Entry Filter, and Portfolio
admission. If the same setup offers opposing Call and Signal entries at the
same open, the System's **Entry Priority** decides which to consider first.
The default is **Call first**.

The setup that opens a position manages its exits and pyramids. The position's
`OTHER.CALL` fields come from its entry row through closure, including after
later calls for the same Asset. [Call row values](nodes.md#call-row-values)

## Exits and reductions

Signal exits, stop losses, and take profits are evaluated in configured order.
An exit may be **Simultaneous** or **After Previous**; the latter is not eligible
until the preceding configured exit has completed. A reduction is in `(0, 1]`.
A partial fill reduces position quantity; a full reduction closes it.

Each signal exit rule has its own **Price** and **Delay**. Its delayed signal
can only reduce the position that generated it. If that position closes before
the target candle or touch, the signal cannot carry over to a new position.
An **After Previous** signal exit can fill on a later touch in the same candle
after its preceding exit completes.
For a position opened during the candle, custom signal exits and pyramids can
use the path from the fill onward. A position first opened at the close can use
those actions starting on the next candle.
[Signal prices and delays](#signal-prices-and-delays)

Along a price segment, the closest reachable level fills first. Equal-price
ties prefer a stop over a target, then the lower configured rule index. At an
opening gap, all already-marketable price exits fill at the actual open with
the same stop-before-target/index priority. Partial reductions can expose the
next eligible rule, so processing continues until no immediate trigger remains
or the position closes.

Stops and targets can reference **Initial Entry Price**, **Latest Entry Price**,
or **Average Price**. This choice changes levels after adding layers. A trailing
stop moves only in the favorable direction and never loosens once established.

## Pyramiding

Pyramiding adds to an existing position up to the Strategy's maximum layers.
It uses its own signal, **Price**, and **Delay**. A delayed signal only applies
to the position that generated it. **Every Evaluation** may trigger again after
cooldown. **After False** also requires the signal to become false before
rearming. The cooldown and the limit of one addition per bar still apply with
a custom price or a longer delay. [Signal prices and delays](#signal-prices-and-delays)

The frozen unit quantity is established by the initial executed entry for
Strategy-driven layer sizing. Fresh-entry Profile sizing and Entry Filter are not
rerun for pyramid additions, although the resulting pyramid request still joins
the applicable Portfolio ranking phase and faces cash/capacity checks.

After an added layer, stop/target levels are reevaluated. Favorable and
unfavorable additions have separate reference choices: latest fill,
quantity-weighted average, or initial entry. Existing trailing stops ratchet in
the favorable direction and never loosen during this update.

## Alternate Strategies

At each entry phase, a System considers its Main Strategy first, followed by
its Alternate Strategies in setup order. Each alternate has two activation
options.

- **Always** lets the alternate act on its own fresh entry signal, even when
  the preceding Strategy has no signal.
- **Failsafe** also requires the preceding Strategy's Entry Filter to have
  rejected the initial entry for a theoretical position that is still open.

Both options require a fresh signal from the alternate when it is eligible
to enter. Theoretical positions run independently of Portfolio admission.

Each Strategy setup keeps its own hypothetical session and Strategy Performance
history, but an expanded System × Asset backtest has only one real execution
position. An active owner blocks another Strategy from opening a competing real
position. Once admitted, the actual trade remains attributed to the setup
that opened it until closure, even if the alternate's eligibility later changes.

## Portfolio event order

The Portfolio processes the union of all real Asset timestamps. At each open,
modeled touch, or close, it settles existing exits before ranking competing
entries at that point. Last-known prices are used only for valuation where the
authoritative portfolio document permits them. [Synchronized Execution](portfolio-backtesting.md#synchronized-execution)

Strategy Performance is theoretical Strategy state and is distinct from actual
Portfolio trade results. That distinction is important for Entry Filters,
Alternate Strategies, and Bayesian Kelly sizing.
