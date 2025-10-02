module;

#include <cmath>
#include <cstddef>
#include <limits>
#include <utility>

export module pludux:series.hma_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

import :series.value_method;
import :series.wma_method;
import :series.operators_method;

export namespace pludux {

template<typename TSourceMethod = CloseMethod>
class HmaMethod {
public:
  using ResultType = typename TSourceMethod::ResultType;

  HmaMethod()
  : HmaMethod{20}
  {
  }

  explicit HmaMethod(std::size_t period)
  : HmaMethod{TSourceMethod{}, period}
  {
  }

  explicit HmaMethod(TSourceMethod source, std::size_t period)
  : source_{std::move(source)}
  , period_{period}
  {
  }

  auto operator==(const HmaMethod& other) const noexcept -> bool = default;

  auto operator()(this const auto& self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) noexcept
   -> ResultType
  {
    const auto wam1 = WmaMethod{self.source_, self.period_ / 2};
    const auto scalar_2_method = ValueMethod{2.0};
    const auto times_2_wam1 = MultiplyMethod{scalar_2_method, wam1};

    const auto wam2 = WmaMethod{self.source_, self.period_};
    const auto diff = SubtractMethod{times_2_wam1, wam2};

    const auto hma =
     WmaMethod{diff, static_cast<std::size_t>(std::sqrt(self.period_))};

    return hma(asset_snapshot, context);
  }

  auto operator()(this const auto& self,
                  AssetSnapshot asset_snapshot,
                  SeriesOutput output,
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

} // namespace pludux
