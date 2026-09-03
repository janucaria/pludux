# Profiles, Markets, and Brokers

These three resources turn a Model's candidate entry and one-R distance into a
quantity and cost. They remain explicit so the same Strategy can be tested
under different portfolio constraints.

## Position sizing

All configured sizing values must be finite and positive unless stated
otherwise. Fractions are ratios: `0.01` means one percent.

| Profile method | Default | Initial calculation | Hard constraint |
| --- | --- | --- | --- |
| Risk Distance | risk fraction `0.01` | `equity × fraction / riskDistance` | Loss at the adverse 1R boundary, including entry and estimated exit fees, must fit `equity × fraction` |
| Fixed Quantity | quantity `1` | Configured units | Nearest Market step; raised to Market minimum if necessary |
| Fixed Budget | budget `1000` | `budget / entryPrice` | Entry notional plus entry fees must fit the budget |
| Equity Fraction | fraction `0.01` | `equity × fraction / entryPrice` | Entry notional plus entry fees must fit the equity allocation |
| Bayesian Kelly | adverse quantiles, credible mass `0.80`, multiplier `0.50`, maximum fraction `1.0` | Posterior Kelly allocation × equity / entry price | Entry cost must fit the resulting allocation |

Bayesian Kelly uses the Strategy binding's theoretical Model Performance under
the System's shared calculation configuration, not actual Portfolio trade
results. Its full priors, posterior estimates, and
failure cases are in [Model Performance Bayesian Kelly](strategy-performance-bayesian-kelly.md).

## Drawdown adjustment

Drawdown adjustment is disabled by default. Its defaults are a drawdown step of
`0.10`, size reduction of `0.20`, and notional-equity reduction of `0`.
For current drawdown ratio `d` and step `s`:

```text
steps = floor(max(d, 0) / s)
size multiplier = max(1 - steps × sizeReduction, 0)
```

The size multiplier scales both requested quantity and its sizing constraint.
When notional-equity reduction is nonzero, sizing methods that use equity are
also re-evaluated from:

```text
effective peak equity × max(1 - steps × notionalEquityReduction, 0)
```

A multiplier of zero suppresses the request. This mechanism is distinct from
Portfolio drawdown reporting and from the `PORTFOLIO.DRAWDOWN` Model node.

## Market normalization

A Market defines minimum order quantity and quantity step; both default to
`1`. Fixed Quantity uses nearest-step rounding and then raises a smaller result
to the Market minimum. Budget-, risk-, cash-, and maximum-quantity constrained
paths search for a fitting quantity and floor it to the step. If the resulting
quantity is zero, below minimum, or violates the constraint, the order is
rejected rather than rounded upward.

## Broker fees

A Broker contains ordered fee rules, and all applicable rules are summed.

- **Percentage Notional** is `trade notional × value / 100`; **Fixed** adds the
  configured currency amount.
- Position scope is **Long**, **Short**, or **Long and Short**.
- **Entry** and **Exit** refer to lifecycle phase. **Buy** and **Sell** refer to
  transaction direction: a long entry and short exit are buys; a short entry
  and long exit are sells. **All** matches every action in scope.

Fees are included in candidate entry cost. Risk Distance sizing also estimates
the fee for an exit at the adverse one-R boundary. The actual exit fee is
calculated from the eventual action and price, so it can differ after gaps or a
different exit reason. Pludux does not model slippage.

## Insufficient cash and capacity

After ranking, the Portfolio checks maximum open trades, maximum combined
layers, and available shared cash. **Reject** discards an order whose entry cost
exceeds cash. **Cap to Available Cash** sizes it again under the cash budget and
applies Market normalization; it still rejects if no valid minimum-step order
fits. Pyramiding uses the initial executed unit quantity and frozen risk
distance, but remains subject to cash, Market, Broker, and Portfolio layer
capacity.

See [Node and Method Reference](nodes.md) for the diagnostics available through
`REQUESTED_ORDER.*`.
