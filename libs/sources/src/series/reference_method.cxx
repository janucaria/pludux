module;

#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <variant>

export module pludux:series.reference_method;

import :asset_snapshot;
import :series.method_call_context;
import :series.method_output;

export namespace pludux::series {

template<typename T = double>
  requires std::numeric_limits<T>::has_quiet_NaN
class ReferenceMethod {
public:
  using ResultType = T;

  ReferenceMethod(std::string name)
  : name_{std::move(name)}
  {
  }

  auto operator==(const ReferenceMethod& other) const noexcept
   -> bool = default;

  auto operator()(this const auto& self,
                  AssetSnapshot asset_data,
                  MethodCallContext<ResultType> auto context) noexcept
   -> ResultType
  {
    if constexpr(std::is_same_v<std::monostate, decltype(context)>) {
      return std::numeric_limits<ResultType>::quiet_NaN();
    } else {
      return context.dispatch_call(self.name(), asset_data);
    }
  }

  auto operator()(this const auto& self,
                  AssetSnapshot asset_data,
                  MethodOutput output_name,
                  MethodCallContext<ResultType> auto context) noexcept
   -> ResultType
  {
    if constexpr(std::is_same_v<std::monostate, decltype(context)>) {
      return std::numeric_limits<ResultType>::quiet_NaN();
    } else {
      return context.dispatch_call(self.name(), asset_data, output_name);
    }
  }

  auto name(this const auto& self) noexcept -> const std::string&
  {
    return self.name_;
  }

  void name(this auto& self, std::string new_name) noexcept
  {
    self.name_ = std::move(new_name);
  }

private:
  std::string name_{};
};

} // namespace pludux::series