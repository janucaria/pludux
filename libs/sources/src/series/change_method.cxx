module;

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <vector>

export module pludux:series.change_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

import :series.ohlcv_method;

export namespace pludux {

template<typename TSourceMethod = CloseMethod>
  requires requires { typename TSourceMethod::ResultType; }
class ChangeMethod {
public:
  using ResultType = TSourceMethod::ResultType;

  ChangeMethod()
  : ChangeMethod{TSourceMethod{}}
  {
  }

  explicit ChangeMethod(TSourceMethod source)
  : source_{std::move(source)}
  {
  }

  auto operator==(const ChangeMethod& other) const noexcept -> bool = default;

  auto operator()(this const ChangeMethod& self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) noexcept -> double
  {
    const auto current = self.source_(asset_snapshot, context);
    const auto previous = self.source_(asset_snapshot[1], context);

    return current - previous;
  }

  auto operator()(this const ChangeMethod& self,
                  AssetSnapshot asset_snapshot,
                  SeriesOutput output,
                  MethodContextable auto context) noexcept -> double
  {
    return std::numeric_limits<ResultType>::quiet_NaN();
  }

  auto source(this const ChangeMethod& self) noexcept -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this ChangeMethod& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

private:
  TSourceMethod source_;
};

} // namespace pludux