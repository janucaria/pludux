module;

#include <cstddef>

export module pludux.screener.stoch_rsi_method;

import pludux.asset_snapshot;
import pludux.series;
import pludux.screener.screener_method;

namespace pludux::screener {

export class StochRsiMethod {
public:
  StochRsiMethod(StochOutput output,
                 ScreenerMethod rsi_input,
                 std::size_t rsi_period,
                 std::size_t k_period,
                 std::size_t k_smooth,
                 std::size_t d_period,
                 std::size_t offset = 0)
  : rsi_input_{rsi_input}
  , output_{output}
  , rsi_period_{rsi_period}
  , k_period_{k_period}
  , k_smooth_{k_smooth}
  , d_period_{d_period}
  , offset_{offset}
  {
  }

  auto operator()(AssetSnapshot asset_data) const
   -> SubSeries<PolySeries<double>>
  {
    const auto stoch_rsi_series = StochRsiSeries{output_,
                                                 rsi_input_(asset_data),
                                                 rsi_period_,
                                                 k_period_,
                                                 k_smooth_,
                                                 d_period_};

    return SubSeries{PolySeries<double>{stoch_rsi_series},
                     static_cast<std::ptrdiff_t>(offset_)};
  }

  auto operator==(const StochRsiMethod& other) const noexcept -> bool = default;

  auto rsi_input() const noexcept -> ScreenerMethod
  {
    return rsi_input_;
  }

  auto rsi_period() const noexcept -> std::size_t
  {
    return rsi_period_;
  }

  auto offset() const noexcept -> std::size_t
  {
    return offset_;
  }

  void offset(std::size_t offset) noexcept
  {
    offset_ = offset;
  }

  auto output() const noexcept -> StochOutput
  {
    return output_;
  }

  void output(StochOutput output) noexcept
  {
    output_ = output;
  }

  auto k_period() const noexcept -> std::size_t
  {
    return k_period_;
  }

  void k_period(std::size_t k_period) noexcept
  {
    k_period_ = k_period;
  }

  auto k_smooth() const noexcept -> std::size_t
  {
    return k_smooth_;
  }

  void k_smooth(std::size_t k_smooth) noexcept
  {
    k_smooth_ = k_smooth;
  }

  auto d_period() const noexcept -> std::size_t
  {
    return d_period_;
  }

  void d_period(std::size_t d_period) noexcept
  {
    d_period_ = d_period;
  }

private:
  ScreenerMethod rsi_input_;
  StochOutput output_;
  std::size_t rsi_period_;
  std::size_t k_period_;
  std::size_t k_smooth_;
  std::size_t d_period_;
  std::size_t offset_;
};

} // namespace pludux::screener
