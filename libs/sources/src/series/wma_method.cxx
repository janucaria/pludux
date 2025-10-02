module;

#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

export module pludux:series.wma_method;

import :asset_snapshot;
import :series.method_contextable;
import :series.method_output;

import :series.ohlcv_method;

export namespace pludux::series {

template<typename TSourceMethod = CloseMethod>
class WmaMethod {
public:
  using ResultType = typename TSourceMethod::ResultType;

  WmaMethod()
  : WmaMethod{20}
  {
  }

  explicit WmaMethod(std::size_t period)
  : WmaMethod{TSourceMethod{}, period}
  {
  }

  explicit WmaMethod(TSourceMethod source, std::size_t period)
  : source_{std::move(source)}
  , period_{period}
  {
  }

  auto operator==(const WmaMethod& other) const noexcept -> bool = default;

  auto operator()(this const auto& self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) noexcept
   -> ResultType
  {
    const auto asset_size = asset_snapshot.size();
    if(asset_size < self.period_) {
      return std::numeric_limits<ResultType>::quiet_NaN();
    }

    auto norm = ResultType{0};
    auto sum = ResultType{0};
    for(auto i = 0uz; i < self.period_; ++i) {
      const auto weight = (self.period_ - i) * self.period_;
      sum += self.source_(asset_snapshot[i], context) * weight;
      norm += weight;
    }

    return sum / norm;
  }

  auto operator()(this const auto& self,
                  AssetSnapshot asset_snapshot,
                  MethodOutput output,
                  MethodContextable auto context) noexcept
   -> ResultType
  {
    return std::numeric_limits<ResultType>::quiet_NaN();
  }

  auto source(this const auto& self) noexcept -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this auto& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const auto& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this auto& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

private:
  TSourceMethod source_;
  std::size_t period_;
};

} // namespace pludux::series
