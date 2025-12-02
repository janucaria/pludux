module;

#include <algorithm>
#include <cstddef>
#include <limits>
#include <numeric>
#include <utility>

export module pludux:series.stoch_rsi_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

import :series.rsi_method;
import :series.sma_method;
import :series.value_method;
import :series.ohlcv_method;
import :series.highest_method;
import :series.lowest_method;

export namespace pludux {

template<typename TRsiSourceMethod = CloseMethod>
class StochRsiMethod {
public:
  using ResultType = RsiMethod<TRsiSourceMethod>::ResultType;

  StochRsiMethod()
  : StochRsiMethod{5, 3, 3}
  {
  }

  explicit StochRsiMethod(std::size_t k_period,
                          std::size_t k_smooth,
                          std::size_t d_period)
  : StochRsiMethod{TRsiSourceMethod{}, 14, k_period, k_smooth, d_period}
  {
  }

  StochRsiMethod(TRsiSourceMethod rsi_source,
                 std::size_t rsi_period,
                 std::size_t k_period,
                 std::size_t k_smooth,
                 std::size_t d_period)
  : rsi_{rsi_source, rsi_period}
  , k_period_{k_period}
  , k_smooth_{k_smooth}
  , d_period_{d_period}
  {
  }

  auto operator==(const StochRsiMethod& other) const noexcept -> bool = default;

  auto operator()(this const StochRsiMethod& self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) noexcept -> ResultType
  {
    return self(asset_snapshot, SeriesOutput::KPercent, context);
  }

  auto operator()(this const StochRsiMethod& self,
                  AssetSnapshot asset_snapshot,
                  SeriesOutput output,
                  MethodContextable auto context) noexcept -> ResultType
  {
    const auto highest_rsi = HighestMethod{self.rsi_, self.k_period_};
    const auto lowest_rsi = LowestMethod{self.rsi_, self.k_period_};
    const auto stoch = DivideMethod{MultiplyMethod{ValueMethod{100},
                                                   SubtractMethod{
                                                    self.rsi_,
                                                    lowest_rsi,
                                                   }},
                                    SubtractMethod{
                                     highest_rsi,
                                     lowest_rsi,
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

  auto rsi_source(this const StochRsiMethod& self) noexcept
   -> const TRsiSourceMethod&
  {
    return self.rsi_.source();
  }

  void rsi_source(this StochRsiMethod& self,
                  TRsiSourceMethod rsi_source) noexcept
  {
    self.rsi_.source(rsi_source);
  }

  auto rsi_period(this const StochRsiMethod& self) noexcept -> std::size_t
  {
    return self.rsi_.period();
  }

  void rsi_period(this StochRsiMethod& self, std::size_t period) noexcept
  {
    self.rsi_.period(period);
  }

  auto k_period(this const StochRsiMethod& self) noexcept -> std::size_t
  {
    return self.k_period_;
  }

  void k_period(this StochRsiMethod& self, std::size_t period) noexcept
  {
    self.k_period_ = period;
  }

  auto k_smooth(this const StochRsiMethod& self) noexcept -> std::size_t
  {
    return self.k_smooth_;
  }

  void k_smooth(this StochRsiMethod& self, std::size_t smooth) noexcept
  {
    self.k_smooth_ = smooth;
  }

  auto d_period(this const StochRsiMethod& self) noexcept -> std::size_t
  {
    return self.d_period_;
  }

  void d_period(this StochRsiMethod& self, std::size_t period) noexcept
  {
    self.d_period_ = period;
  }

private:
  RsiMethod<TRsiSourceMethod> rsi_;
  std::size_t k_period_;
  std::size_t k_smooth_;
  std::size_t d_period_;
};

} // namespace pludux
