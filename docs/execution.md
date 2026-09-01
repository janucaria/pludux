# Execution and Position Lifecycle

This page describes the lifecycle configured by a Model, Strategy, System, and
Portfolio. [Portfolio Backtesting](portfolio-backtesting.md) remains
authoritative for portfolio clock and valuation behavior.

## Bar visibility and entry timing

**Current Close** evaluates with the completed current candle and fills at that
close. **Next Open** evaluates from the previous completed candle and fills at
the next available open. No future candle value is exposed to the signal.

The Model chooses **Low First**, **High First**, or **Candle Direction** as its
deterministic intrabar path. Candle Direction uses open→low→high→close for an up
candle and open→high→low→close for a down candle. A doji visits the extreme
nearest the open first; equal distances choose low first. It resolves bar-level
ambiguity and does not reconstruct ticks.

If the open gaps through a pending stop or target, the first reachable price on
the path can be the open rather than the configured level. Pludux has no
separate slippage model.

## Initial entries

The active Strategy evaluates its signal and entry price, computes risk distance
and Profile sizing, applies Entry Filter, and produces a Requested Order. The
Portfolio gathers fresh requests at a union timestamp, ranks them with its
comparators, and admits requests while capital and capacity permit. Complete
ties use System order and then Watchlist Asset order.

## Exits and reductions

Signal exits, stop losses, and take profits are evaluated in configured order.
An exit may be **Simultaneous** or **After Previous**; the latter is not eligible
until the preceding configured exit has completed. A reduction is in `(0, 1]`.
A partial fill reduces position quantity; a full reduction closes it.

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

Pyramiding adds to an existing position up to the Model's maximum layers. It
has its own signal, delay, price, cooldown, and retrigger rule. **Every
Evaluation** may trigger again after cooldown. **After False** additionally
requires the signal to become false before rearming. At most one layer is
processed per bar; Portfolio layer capacity still applies.

The frozen unit quantity is established by the initial executed entry for
Model-driven layer sizing. Fresh-entry Profile sizing and Entry Filter are not
rerun for pyramid additions, although the resulting pyramid request still joins
the applicable Portfolio ranking phase and faces cash/capacity checks.

After an added layer, stop/target levels are reevaluated. Favorable and
unfavorable additions have separate reference choices: latest fill,
quantity-weighted average, or initial entry. Existing trailing stops ratchet in
the favorable direction and never loosen during this update.

## Failsafe Strategies

A System begins with its Main Strategy. A Failsafe is either **Always** active
or activated by the preceding Strategy's entry-filtered theoretical position.
That theoretical lifecycle continues independently of whether Portfolio capital
allowed an actual entry. Activation state is persistent according to that
simulation; toggling activation does not materialize a theoretical position as
an actual trade. Ordered Failsafes can therefore express fall-through setups
without looking ahead.

Each Strategy binding keeps its own hypothetical session and Model Performance
history, but an expanded System × Asset backtest has only one real execution
position. An active owner blocks another Strategy from opening a competing real
position. Once admitted, the actual trade remains attributed to the Strategy
that opened it until closure, even if Failsafe activation later changes.

## Portfolio event order

The Portfolio processes the union of all Asset timestamps. Missing candles are
not synthesized. Existing-position lifecycle work is processed before competing
fresh entries at the same union timestamp; eligible entries are then ranked and
admitted. Last-known prices are used only for valuation where the authoritative
portfolio document permits them.

Model Performance is theoretical Model state and is distinct from actual
Portfolio trade results. That distinction is important for Entry Filters,
Failsafes, and Bayesian Kelly sizing.
