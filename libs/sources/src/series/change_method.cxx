module;

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <vector>

export module pludux:series.change_method;

import :asset_snapshot;
import :series.method_contextable;
import :series.method_output;

import :series.ohlcv_method;

export namespace pludux::series {

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
  : source_{source}
  {
  }

  auto operator==(const ChangeMethod& other) const noexcept -> bool = default;

  auto operator()(this const auto& self,
                  AssetSnapshot asset_data,
                  MethodContextable auto context) noexcept -> double
  {
    const auto current = self.source_(asset_data, context);
    const auto previous = self.source_(asset_data[1], context);

    return current - previous;
  }

  auto operator()(this const auto& self,
                  AssetSnapshot asset_data,
                  MethodOutput output,
                  MethodContextable auto context) noexcept -> double
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

private:
  TSourceMethod source_;
};

} // namespace pludux::series