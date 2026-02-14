module;

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <ranges>
#include <utility>

export module pludux:series.stddev_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

import :series.ohlcv_method;

export namespace pludux {

template<typename TSourceMethod = CloseMethod>
class StddevMethod {
public:
  using ResultType = typename TSourceMethod::ResultType;

  StddevMethod()
  : StddevMethod{20}
  {
  }

  explicit StddevMethod(std::size_t period)
  : StddevMethod{TSourceMethod{}, period}
  {
  }

  explicit StddevMethod(TSourceMethod source, std::size_t period)
  : source_{std::move(source)}
  , period_{period}
  {
  }

  auto operator==(const StddevMethod& other) const noexcept -> bool = default;

  auto operator()(this const StddevMethod& self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) noexcept -> ResultType
  {
    const auto sum = std::ranges::fold_left(
     std::views::iota(0uz, self.period_), ResultType{0}, [&](auto acc, auto i) {
       return acc + self.source_(asset_snapshot[i], context);
     });
    const auto mean = sum / static_cast<ResultType>(self.period_);

    auto sum_squared_diff = ResultType{0};
    for(auto i = 0uz; i < self.period_; ++i) {
      const auto diff = self.source_(asset_snapshot[i], context) - mean;
      sum_squared_diff += diff * diff;
    }

    const auto variance =
     sum_squared_diff / static_cast<ResultType>(self.period_);
    const auto stddev = std::sqrt(variance);
    return stddev;
  }

  auto operator()(this const StddevMethod& self,
                  AssetSnapshot asset_snapshot,
                  SeriesOutput output,
                  MethodContextable auto context) noexcept -> ResultType
  {
    return std::numeric_limits<ResultType>::quiet_NaN();
  }

  auto source(this const StddevMethod& self) noexcept -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this StddevMethod& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const StddevMethod& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this StddevMethod& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

private:
  TSourceMethod source_;
  std::size_t period_;
};

} // namespace pludux
