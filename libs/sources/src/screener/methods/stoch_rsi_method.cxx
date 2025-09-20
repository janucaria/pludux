module;

#include <cstddef>

export module pludux:screener.stoch_rsi_method;

import :asset_snapshot;
import :screener.screener_method;

export namespace pludux::screener {

class StochRsiMethod {
public:
  StochRsiMethod(ScreenerMethod rsi_input,
                 std::size_t rsi_period,
                 std::size_t k_period,
                 std::size_t k_smooth,
                 std::size_t d_period)
  : rsi_input_{rsi_input}
  , rsi_period_{rsi_period}
  , k_period_{k_period}
  , k_smooth_{k_smooth}
  , d_period_{d_period}
  {
  }

  auto operator==(const StochRsiMethod& other) const noexcept -> bool = default;

  auto operator()(this const auto& self, AssetSnapshot asset_data)
   -> PolySeries<double>
  {
    return StochRsiSeries{self.rsi_input_(asset_data),
                          self.rsi_period_,
                          self.k_period_,
                          self.k_smooth_,
                          self.d_period_};
  }

  auto rsi_input() const noexcept -> ScreenerMethod
  {
    return rsi_input_;
  }

  auto rsi_period() const noexcept -> std::size_t
  {
    return rsi_period_;
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
  std::size_t rsi_period_;
  std::size_t k_period_;
  std::size_t k_smooth_;
  std::size_t d_period_;
};

} // namespace pludux::screener