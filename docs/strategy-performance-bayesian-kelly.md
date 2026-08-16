# Model Performance Bayesian Kelly

## Purpose

# Model Performance Bayesian Kelly is a position-sizing policy for backtesting
research. It estimates the edge of the Model's theoretical positions and
converts that estimate into a requested fraction of current execution equity.

The word **Model** is important. The statistical observations come from the
positions produced by the Model as a black box. They do not come from the
positions that happened to be executed after filtering, sizing, market rules,
fees, or cash constraints.

This document specifies the model and its implementation. It is intended to be
reviewable by statisticians as well as developers. It is not financial advice
and does not claim that the resulting leverage is optimal in live markets.

## Separation of Responsibilities

The backtest has five relevant layers:

1. The Strategy emits entry, pyramid, and exit intents and maintains a
   theoretical position with unit-sized entries.
2. Model Performance observes only closed theoretical positions and updates
   its frequentist and Bayesian evidence.
3. Position Sizing reads the current Model Performance posterior at an entry
   intent and produces an equity-allocation constraint.
4. The Profile may reevaluate sizing with peak-based notional equity, applies
   any final-quantity drawdown reduction, and produces a Market-valid immutable
   Requested Order with fee and risk estimates.
5. The setup's Entry Filter decides whether that order may enter Portfolio
   ranking. Portfolio admission then applies capacity and the selected
   Profile's insufficient-cash policy.

Consequently, an entry-filtered or unaffordable entry still belongs to the
theoretical Strategy history. Conversely, its absent or reduced execution
result does not change the statistical payoff observation.

## Observations

For a closed theoretical position, let the normalized return be \(r\). It is a
ratio such as `0.02` for a 2% return, not account profit and loss.

| Return | Binary evidence | Winning magnitude | Losing magnitude |
| --- | ---: | ---: | ---: |
| \(r>0\) | win | \(r\) | none |
| \(r<0\) | loss | none | \(|r|\) |
| \(r=0\) | configured break-even treatment | none | none |

The break-even treatment is `SKIP`, `COUNT_AS_WIN`, or `COUNT_AS_LOSS`.
`COUNT_AS_LOSS` is the default. This setting changes only Beta-Bernoulli
evidence. Frequentist outcomes preserve wins, break-even results, and losses as
three separate categories. Zero returns always contribute to the lifetime count
and frequentist return statistics, and they never contribute a zero magnitude
to either positive-support payoff model.

For history-weighted positive, zero, and negative counts \(W\), \(E\), and
\(L\), respectively, the frequentist rates are:

\[
\text{win rate}=\frac{W}{W+E+L},\qquad
\text{break-even rate}=\frac{E}{W+E+L},\qquad
\text{loss rate}=\frac{L}{W+E+L}
\]

They sum to one whenever at least one observation has effective weight.

Winning and losing magnitudes are modeled separately because their distributions
need not have the same scale or shape. Losses use absolute magnitude so both
payoff models have positive support.

## History Weighting

Priors are never removed or decayed. Only observational evidence is weighted.

- **All history:** every closed theoretical position retains weight 1.
- **Rolling window:** only the latest \(N\) total closed positions remain. A
  zero return occupies a window slot regardless of its break-even treatment.
- **Exponential decay:** before each new closed position is added, all existing
  observational sufficient statistics are multiplied by the configured decay
  factor. The new observation then receives weight 1 where applicable. A zero
  return therefore ages earlier evidence before its configured binary
  classification is applied.

Effective counts may be fractional under exponential decay.

## Winning and Losing Streaks

Model Performance also records chronological streaks from theoretical
position returns:

- A positive return increments the current winning streak, resets the current
  losing streak, and may increase the maximum winning streak.
- A negative return increments the current losing streak, resets the current
  winning streak, and may increase the maximum losing streak.
- An exactly zero return resets both current streaks. Its Bayesian break-even
  treatment does not change this frequentist behavior.

Current and maximum streaks always use the complete lifetime sequence. They are
integer counts and are not rolled out or decayed by the statistical history
policy. Consequently, a rolling window or exponential decay changes the
frequentist evidence summaries and Bayesian posteriors but not the streaks.
Streaks use theoretical Strategy positions only and are independent of realized
execution.

## Win Probability Model

Let \(p\) be the probability of a binary win under the configured classification.
Positive returns are wins, negative returns are losses, and zero returns are
skipped or classified as wins or losses according to the break-even treatment.
The likelihood is Bernoulli and the prior is Beta:

\[
p \sim \operatorname{Beta}(\alpha_0,\beta_0)
\]

The configuration uses a prior probability (p_0) and prior strength (n_0):

\[
\alpha_0=p_0n_0,\qquad
\beta_0=(1-p_0)n_0
\]

For weighted win evidence (W) and loss evidence (L):

\[
p\mid D\sim\operatorname{Beta}(\alpha_0+W,\beta_0+L)
\]

The posterior mean is:

\[
E[p\mid D]=\frac{\alpha_0+W}{\alpha_0+\beta_0+W+L}
\]

The default prior is (p_0=0.5) and (n_0=2), equivalent to
\(\operatorname{Beta}(1,1)\).

## Winning and Losing Payoff Models

Each positive payoff magnitude (X) is modeled with a Gamma likelihood whose
shape is fixed by the configured coefficient of variation:

\[
k=\frac{1}{\mathrm{CV}^2},\qquad
X\mid\theta\sim\operatorname{Gamma}
\left(k,\frac{\theta}{k}\right)
\]

Here the second Gamma argument is the scale, so
\(E[X\mid\theta]=\theta\). The unknown mean magnitude \(\theta\) has an
inverse-gamma posterior. For prior mean magnitude (m_0), prior strength
(n_0), effective magnitude count (N), and weighted magnitude sum
\(S=\sum X_i\):

\[
\alpha=1+k(n_0+N),\qquad
\beta=k(n_0m_0+S)
\]

\[
\theta\mid D\sim\operatorname{InverseGamma}(\alpha,\beta)
\]

The implementation uses inverse-gamma shape \(\alpha\) and scale \(\beta\).
Its posterior mean is:

\[
E[\theta\mid D]=\frac{\beta}{\alpha-1}
\]

Winning and losing models have independent configurations and sufficient
statistics. Both default to:

- prior mean magnitude (m_0=0.01);
- prior strength (n_0=0.01);
- coefficient of variation \(\mathrm{CV}=1.0\).

## Binary Kelly Fraction

Suppose investing fraction (f) of equity has two possible normalized returns:

- probability (p) of gaining winning magnitude (b>0);
- probability (1-p) of losing magnitude (a>0).

Expected logarithmic growth is:

\[
g(f)=p\log(1+fb)+(1-p)\log(1-fa)
\]

Setting its derivative to zero gives the unconstrained binary Kelly fraction:

\[
f_{\mathrm{raw}}=\frac{p}{a}-\frac{1-p}{b}
\]

This is a binary approximation: it replaces the full return distributions with
one win probability and one conditional mean magnitude for each outcome.

The requested allocation fraction is:

\[
f_{\mathrm{scaled}}=m\max(f_{\mathrm{raw}},0)
\]

\[
f_{\mathrm{entry}}=\min(f_{\mathrm{scaled}},f_{\max})
\]

where (m\in[0,1]) is the Kelly multiplier and (f_{\max}>0) is the
per-entry maximum equity fraction. The requested unsigned asset quantity is:

\[
Q=\frac{f_{\mathrm{entry}}\times\text{current equity}}
        {\text{entry price}}
\]

The defaults are half Kelly, (m=0.5), and a maximum equity fraction of 1.0.
The corresponding allocation is an entry-cost budget, so the submitted
quantity may be smaller than (Q) when Broker entry fees apply or Market rules
require discrete quantity steps. The maximum may be configured above 1.0,
although cash policy can still reduce or reject an unaffordable order.

## Posterior Estimate Modes

### Posterior Mean

Posterior Mean uses:

\[
p=E[p\mid D],\qquad b=E[\theta_{win}\mid D],\qquad
a=E[\theta_{loss}\mid D]
\]

This summarizes each marginal posterior independently. It is not the posterior
mean of the nonlinear Kelly fraction itself.

### Adverse Quantiles

Let \(C\in(0,1)\) be the **central credible mass** used by Adverse Quantiles.
Each tail has probability \(q=(1-C)/2\). Adverse Quantiles mode selects inputs
adverse to the estimated edge:

\[
p=Q_{\operatorname{Beta}}(q)
\]

\[
b=Q_{\operatorname{InverseGamma,win}}(q)
\]

\[
a=Q_{\operatorname{InverseGamma,loss}}(1-q)
\]

With the default (C=0.80), these are the 10th percentile of win probability,
the 10th percentile of winning magnitude, and the 90th percentile of losing
magnitude. These marginal quantiles form a deliberately conservative scenario;
they are not a joint 80% credible region.

## Timing and Absence of Look-Ahead

At an entry intent, sizing reads the Model Performance snapshot available at
that instant. A theoretical position contributes evidence only after it closes.
Therefore, neither its future outcome nor any later bar can influence its entry
size. The initial executed entry freezes its final quantity as the position's
Unit and freezes its risk distance. Pyramid layers request that Unit and do not
reevaluate equity, drawdown adjustment, Bayesian Kelly, or the risk-distance
expression. A new position starts with a fresh posterior and sizing evaluation.

No warm-up count is imposed. Under untouched symmetric defaults, Posterior Mean
has \(p=0.5\), \(a=b=0.01\), and therefore \(f_{raw}=0\). Adverse Quantiles
mode is more adverse and also requests no position.

## Worked Examples

### Untouched priors

With (p=0.5), (a=b=0.01):

\[
f_{raw}=0.5/0.01-0.5/0.01=0
\]

The theoretical Strategy intent remains recorded, but execution is skipped with
the `NoPositiveSize` outcome.

### Posterior-mean positive edge and allocation cap

Assume all-history evidence contains 8 wins totaling 0.16 and 4 losses totaling
0.04. With the default priors and \(\mathrm{CV}=1\):

\[
p=9/(9+5)=0.642857
\]

\[
b=0.1601/8.01\approx0.0199875,\qquad
a=0.0401/4.01=0.01
\]

Thus (f_{raw}\approx46.42). Half Kelly is still far above the default
maximum, so (f_{entry}=1.0). At equity 10,000 and entry price 100, the primary
request is 100 units. This example also shows why a separate allocation cap is
important when payoff magnitudes are small normalized returns.

### No edge

For equal magnitudes \(a=b\), the raw fraction is zero at \(p=0.5\) and
negative below it. Negative raw fractions are converted to zero; the
implementation never uses this sizing method to open the opposite direction.

### Adverse Quantiles mode

For a central credible mass of 0.80, evaluate the Beta 10th percentile, winning
inverse-gamma 10th percentile, and losing inverse-gamma 90th percentile using
the stored posterior parameters. Insert those three values into the same Kelly
formula. The timeline records the exact selected values so the calculation can
be reproduced without inferring them from displayed 95% intervals.

### Cash capping

If the Kelly allocation is 10,000 at price 100, allocation sizing first finds
the largest Market-valid quantity whose notional and entry fees do not exceed
10,000. The pipeline then compares that fee-inclusive entry cost with shared
cash after reservations. If shared cash supports only 97 units, the
`CapToAvailableCash` policy searches for the largest affordable quantity without
exceeding the Kelly allocation. This execution reduction does not alter
Model Performance evidence.

## Configuration and Diagnostic Mapping

| Configuration or diagnostic | Symbol or meaning |
| --- | --- |
| `breakEvenTreatment` | Zero-return binary classification; defaults to `COUNT_AS_LOSS` |
| `winRate` | History-weighted positive-return rate, independent of break-even treatment |
| `breakEvenRate` | History-weighted zero-return rate, independent of break-even treatment |
| `lossRate` | History-weighted negative-return rate, independent of break-even treatment |
| `estimate` | Posterior Mean or Adverse Quantiles input selection |
| `centralCredibleMass` | Central credible mass \(C\), used only for Adverse Quantiles |
| `kellyMultiplier` | (m\in[0,1]) |
| `maxEquityFraction` | (f_{max}>0) |
| `winProbability` | selected (p) |
| `winningPayoff` | selected (b) |
| `losingPayoff` | selected (a) |
| `rawKellyFraction` | (f_{raw}) |
| `scaledKellyFraction` | (m\max(f_{raw},0)) |
| `allocationFraction` | (f_{entry}) |
| `requestedQuantity` | (Q) before execution overlays |
| `requestedLimit` | equity-allocation entry-cost budget |
| `drawdownAdjustedQuantity` | quantity after drawdown adjustment |
| `drawdownAdjustedLimit` | allocation after drawdown adjustment |
| `sizingNormalizedQuantity` | Market-valid, fee-aware allocation quantity |
| `entryCost` | submitted notional plus entry fees |
| `cashRequired` | entry cost compared with shared cash |
| `cashAvailable` | shared cash after existing reservations |
| `finalQuantity` | quantity submitted after cash policy |
| `finalEntryCost` | submitted notional plus fees after cash policy |

## Validation and Reproduction

- Existing fixed/risk/equity sizing parameters must be finite and positive.
- `centralCredibleMass` must be finite and strictly between 0 and 1.
- `kellyMultiplier` must be finite and in the closed interval `[0, 1]`.
- `maxEquityFraction` must be finite and positive; it has no configured upper
  bound.
- Equity, entry price, required risk distance, posterior magnitudes, and
  distribution parameters must be valid at evaluation time.
- A finite non-positive Kelly edge is a valid no-size result, not a model error.

To reproduce a decision, use the timeline's selected (p,b,a), multiplier, and
cap. Recompute the raw, scaled, and allocation fractions with the equations
above, then multiply allocation by that decision's equity and divide by its entry
price. Later pipeline quantities may differ for the documented execution-layer
reasons.

## Assumptions and Limitations

- The Beta and two payoff posteriors are combined as independent marginal
  summaries. Dependence between win rate and payoff magnitudes is not modeled.
- Gamma likelihoods impose positive, light-tailed magnitude models with fixed
  coefficients of variation. Real strategy payoffs may be heavier-tailed or
  multimodal.
- Priors can materially affect small samples. No warm-up rule protects against
  an inappropriate prior.
- Rolling and decay histories mitigate but do not solve non-stationarity.
- Serial dependence, clustered losses, liquidity changes, market impact, and
  estimation of ruin probability are outside this model.
- Adverse marginal quantiles are a heuristic stress scenario, not full
  Bayesian decision integration and not a joint posterior draw.
- Binary Kelly is distinct from continuous Kelly. It compresses all wins to one
  magnitude and all losses to another rather than optimizing over the complete
  return distribution.
- The formula assumes the estimated binary process is relevant to the next
  theoretical position. Backtest evidence alone cannot establish that premise.
- Counting break-even positions as wins or losses changes event probability but
  not either conditional payoff distribution. This is an intentional
  classification policy rather than a claim that a zero payoff has positive or
  negative magnitude.
