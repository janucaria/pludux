module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:series.roc_method;

import :asset_snapshot;
import :series.method_contextable;
import :series.method_output;

import :series.ohlcv_method;

export namespace pludux::series {

template<typename TSourceMethod = CloseMethod>
class RocMethod {
public:
  using ResultType = typename TSourceMethod::ResultType;

  RocMethod()
  : RocMethod{14}
  {
  }

  RocMethod(std::size_t period)
  : RocMethod{TSourceMethod{}, period}
  {
  }

  RocMethod(TSourceMethod source, std::size_t period)
  : source_{std::move(source)}
  , period_{period}
  {
  }

  auto operator==(const RocMethod& other) const noexcept -> bool = default;

  auto operator()(this const auto& self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) noexcept
   -> ResultType
  {
    const auto source_size = asset_snapshot.size();
    if(source_size < self.period_) {
      return std::numeric_limits<ResultType>::quiet_NaN();
    }

    const auto current = self.source_(asset_snapshot, context);
    const auto end = self.source_(asset_snapshot[self.period_], context);

    return 100 * (current - end) / end;
  }

  auto operator()(this const auto& self,
                  AssetSnapshot asset_snapshot,
                  MethodOutput output,
                  MethodContextable auto context) noexcept
   -> ResultType
  {
    return std::numeric_limits<ResultType>::quiet_NaN();
  }

  auto source(this const auto& self) noexcept -> TSourceMethod
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
