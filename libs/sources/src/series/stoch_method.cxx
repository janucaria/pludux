module;

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>

export module pludux:series.stoch_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

import :series.ohlcv_method;
import :series.operators_method;
import :series.value_method;
import :series.sma_method;
import :series.highest_method;
import :series.lowest_method;

export namespace pludux {

class StochMethod {
public:
  using ResultType = std::common_type_t<typename HighMethod::ResultType,
                                        typename LowMethod::ResultType,
                                        typename CloseMethod::ResultType>;

  StochMethod(std::size_t k_period, std::size_t k_smooth, std::size_t d_period)
  : k_period_{k_period}
  , k_smooth_{k_smooth}
  , d_period_{d_period}
  {
  }

  auto operator==(const StochMethod& other) const noexcept -> bool = default;

  auto operator()(this const StochMethod& self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) noexcept -> ResultType
  {
    return self(asset_snapshot, SeriesOutput::KPercent, context);
  }

  auto operator()(this const StochMethod& self,
                  AssetSnapshot asset_snapshot,
                  SeriesOutput output,
                  MethodContextable auto context) noexcept -> ResultType
  {
    const auto close = CloseMethod{};
    const auto highest_high = HighestMethod{HighMethod{}, self.k_period_};
    const auto lowest_low = LowestMethod{LowMethod{}, self.k_period_};
    const auto stoch = DivideMethod{MultiplyMethod{ValueMethod{100},
                                                   SubtractMethod{
                                                    close,
                                                    lowest_low,
                                                   }},
                                    SubtractMethod{
                                     highest_high,
                                     lowest_low,
                                    }};

    const auto k_percent = SmaMethod{stoch, self.k_smooth_};

    switch(output) {
    case SeriesOutput::KPercent:
      return k_percent(asset_snapshot, context);
    case SeriesOutput::DPercent:
      return SmaMethod{k_percent, self.d_period_}(asset_snapshot, context);
    default:
      return std::numeric_limits<ResultType>::quiet_NaN();
    }
  }

  auto k_period(this const StochMethod& self) noexcept -> std::size_t
  {
    return self.k_period_;
  }

  void k_period(this StochMethod& self, std::size_t k_period) noexcept
  {
    self.k_period_ = k_period;
  }

  auto k_smooth(this const StochMethod& self) noexcept -> std::size_t
  {
    return self.k_smooth_;
  }

  void k_smooth(this StochMethod& self, std::size_t k_smooth) noexcept
  {
    self.k_smooth_ = k_smooth;
  }

  auto d_period(this const StochMethod& self) noexcept -> std::size_t
  {
    return self.d_period_;
  }

  void d_period(this StochMethod& self, std::size_t d_period) noexcept
  {
    self.d_period_ = d_period;
  }

private:
  std::size_t k_period_;
  std::size_t k_smooth_;
  std::size_t d_period_;
};

} // namespace pludux
