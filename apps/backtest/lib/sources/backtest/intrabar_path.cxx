module;

#include <array>
#include <cmath>

export module pludux.backtest:intrabar_path;

import :execution_model;

export namespace pludux::backtest {

using IntrabarPrices = std::array<double, 4>;

auto make_intrabar_prices(IntrabarPath path,
                          double open,
                          double high,
                          double low,
                          double close) noexcept -> IntrabarPrices
{
  const auto low_first = [&] {
    switch(path) {
    case IntrabarPath::LowFirst:
      return true;
    case IntrabarPath::HighFirst:
      return false;
    case IntrabarPath::CandleDirection:
      if(close > open) {
        return true;
      }
      if(close < open) {
        return false;
      }
      return std::abs(open - low) <= std::abs(high - open);
    }
    return true;
  }();

  return low_first ? IntrabarPrices{open, low, high, close}
                   : IntrabarPrices{open, high, low, close};
}

} // namespace pludux::backtest
