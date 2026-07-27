module;

#include <array>
#include <span>
#include <string_view>

export module pludux.apps.backtest:built_in_strategies;

export namespace pludux::apps {

struct BuiltInStrategy {
  std::string_view name;
  std::string_view description;
  std::string_view json;
};

auto built_in_strategies() noexcept -> std::span<const BuiltInStrategy>
{
  static constexpr auto strategies = std::array{
   BuiltInStrategy{"MA Cross",
                   "Trades when price crosses a configurable moving average.",
                   PLUDUX_BUILT_IN_STRATEGY_moving_avg_cross},
   BuiltInStrategy{
    "MA 2 Lines Cross",
    "Trades crossovers between configurable fast and slow moving averages.",
    PLUDUX_BUILT_IN_STRATEGY_moving_avg_2_line_cross},
   BuiltInStrategy{
    "Bollinger Bands Long",
    "A long-only Bollinger Bands strategy for lower-band entries.",
    PLUDUX_BUILT_IN_STRATEGY_bollinger_bands_L},
   BuiltInStrategy{
    "Bollinger Bands Short",
    "A short-only Bollinger Bands strategy for upper-band entries.",
    PLUDUX_BUILT_IN_STRATEGY_bollinger_bands_S},
   BuiltInStrategy{
    "Bollinger Bands Long & Short",
    "A two-sided Bollinger Bands strategy with long and short rules.",
    PLUDUX_BUILT_IN_STRATEGY_bollinger_bands_LS},
   BuiltInStrategy{"RSI",
                   "Trades configurable relative strength index thresholds.",
                   PLUDUX_BUILT_IN_STRATEGY_rsi},
   BuiltInStrategy{
    "MACD",
    "Trades moving average convergence divergence signal crossovers.",
    PLUDUX_BUILT_IN_STRATEGY_macd},
   BuiltInStrategy{
    "Keltner Channels",
    "Trades price movement through configurable Keltner Channels.",
    PLUDUX_BUILT_IN_STRATEGY_keltner_channels},
   BuiltInStrategy{"Donchian Channels",
                   "Trades breakouts from recent configurable price extremes.",
                   PLUDUX_BUILT_IN_STRATEGY_donchian_channels},
   BuiltInStrategy{
    "Stochastic",
    "Trades configurable stochastic oscillator signal crossovers.",
    PLUDUX_BUILT_IN_STRATEGY_stochastic},
   BuiltInStrategy{"Stochastic RSI",
                   "Trades configurable stochastic RSI signal crossovers.",
                   PLUDUX_BUILT_IN_STRATEGY_stochastic_rsi},
  };

  return strategies;
}

} // namespace pludux::apps
