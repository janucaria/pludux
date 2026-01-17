module;

#include <cmath>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

export module pludux:series.rsi_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

import :series.change_method;
import :series.cached_results_rma_method;
import :series.operators_method;
import :series.ohlcv_method;
import :series.value_method;

export namespace pludux {

template<typename TSourceMethod = CloseMethod>
class RsiMethod {
public:
  using ResultType = typename TSourceMethod::ResultType;

  RsiMethod()
  : RsiMethod{14}
  {
  }

  explicit RsiMethod(std::size_t period)
  : RsiMethod{TSourceMethod{}, period}
  {
  }

  explicit RsiMethod(TSourceMethod source, std::size_t period)
  : source_{std::move(source)}
  , period_{period}
  , rs_method_{DivideMethod{
     CachedResultsRmaMethod{PositivePartMethod{ChangeMethod{source_}}, period_},
     CachedResultsRmaMethod{NegativePartMethod{ChangeMethod{source_}},
                            period_}}}
  {
  }

  auto operator==(const RsiMethod& other) const noexcept -> bool = default;

  auto operator()(this const RsiMethod& self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) noexcept -> ResultType
  {
    const auto rs = self.rs_method_(asset_snapshot, context);
    const auto rsi = 100 - (100 / (1 + rs));

    return rsi;
  }

  auto operator()(this const RsiMethod& self,
                  AssetSnapshot asset_snapshot,
                  SeriesOutput output,
                  MethodContextable auto context) noexcept -> ResultType
  {
    return std::numeric_limits<ResultType>::quiet_NaN();
  }

  auto source(this const RsiMethod& self) noexcept -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this RsiMethod& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
    self.recompute_rs_method();
  }

  auto period(this const RsiMethod& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this RsiMethod& self, std::size_t period) noexcept
  {
    self.period_ = period;
    self.recompute_rs_method();
  }

private:
  TSourceMethod source_;
  std::size_t period_;

  DivideMethod<
   CachedResultsRmaMethod<PositivePartMethod<ChangeMethod<TSourceMethod>>>,
   CachedResultsRmaMethod<NegativePartMethod<ChangeMethod<TSourceMethod>>>>
   rs_method_;

  void recompute_rs_method(this RsiMethod& self) noexcept
  {
    self.rs_method_ = DivideMethod{
     CachedResultsRmaMethod{PositivePartMethod{ChangeMethod{self.source_}},
                            self.period_},
     CachedResultsRmaMethod{NegativePartMethod{ChangeMethod{self.source_}},
                            self.period_}};
  }
};

RsiMethod() -> RsiMethod<CloseMethod>;
RsiMethod(std::size_t) -> RsiMethod<CloseMethod>;

} // namespace pludux
