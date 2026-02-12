module;

#include <cassert>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:series.atr_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

import :series.tr_method;
import :series.adaptive_ma_method;

export namespace pludux {

class AtrMethod {
public:
  using ResultType = typename TrMethod::ResultType;

  AtrMethod()
  : AtrMethod{14}
  {
  }

  explicit AtrMethod(std::size_t period)
  : AtrMethod{MaMethodType::Rma, period}
  {
  }

  explicit AtrMethod(MaMethodType ma_smoothing_type, std::size_t period)
  : ma_smoothing_method_{ma_smoothing_type, TrMethod{}, period}
  {
  }

  auto operator==(const AtrMethod& other) const noexcept -> bool = default;

  auto operator()(this auto self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) noexcept -> ResultType
  {
    const auto atr = self.ma_smoothing_method_(asset_snapshot, context);

    return atr;
  }

  auto operator()(this AtrMethod self,
                  AssetSnapshot asset_snapshot,
                  SeriesOutput output,
                  MethodContextable auto context) noexcept -> ResultType
  {
    return std::numeric_limits<ResultType>::quiet_NaN();
  }

  auto period(this AtrMethod self) noexcept -> std::size_t
  {
    return self.ma_smoothing_method_.period();
  }

  void period(this AtrMethod& self, std::size_t new_period) noexcept
  {
    self.ma_smoothing_method_.period(new_period);
  }

  auto ma_smoothing_type(this const AtrMethod& self) noexcept -> MaMethodType
  {
    return self.ma_smoothing_method_.ma_type();
  }

  void ma_smoothing_type(this AtrMethod& self, MaMethodType new_type) noexcept
  {
    self.ma_smoothing_method_.ma_type(new_type);
  }

private:
  AdaptiveMaMethod<TrMethod> ma_smoothing_method_;
};

} // namespace pludux
