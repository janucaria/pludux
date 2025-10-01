module;

#include <algorithm>
#include <iterator>
#include <vector>

export module pludux:screener.value_method;

import :asset_snapshot;
import :screener.method_call_context;
import :screener.method_output;

export namespace pludux::screener {

template<typename TResult = double>
  requires std::numeric_limits<TResult>::has_quiet_NaN
class ValueMethod {
public:
  using ResultType = TResult;

  explicit ValueMethod(TResult value)
  : value_{value}
  {
  }

  auto operator==(const ValueMethod& other) const noexcept -> bool = default;

  auto operator()(this auto self,
                  AssetSnapshot asset_data,
                  MethodCallContext<ResultType> auto context) noexcept
   -> ResultType
  {
    return self.value_;
  }

  auto operator()(this auto self,
                  AssetSnapshot asset_data,
                  MethodOutput output_name,
                  MethodCallContext<ResultType> auto context) noexcept -> double
  {
    return std::numeric_limits<ResultType>::quiet_NaN();
  }

  auto value(this auto self) noexcept -> ResultType
  {
    return self.value_;
  }

  void value(this auto& self, ResultType new_value) noexcept
  {
    self.value_ = new_value;
  }

private:
  ResultType value_;
};

} // namespace pludux::screener
