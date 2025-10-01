module;

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <vector>

export module pludux:screener.change_method;

import :asset_snapshot;
import :screener.method_call_context;
import :screener.method_output;

import :screener.ohlcv_method;

export namespace pludux::screener {

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
                  MethodCallContext<ResultType> auto context) noexcept -> double
  {
    const auto current = self.source_(asset_data, context);
    const auto previous = self.source_(asset_data[1], context);

    return current - previous;
  }

  auto operator()(this const auto& self,
                  AssetSnapshot asset_data,
                  MethodOutput output,
                  MethodCallContext<ResultType> auto context) noexcept -> double
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

} // namespace pludux::screener