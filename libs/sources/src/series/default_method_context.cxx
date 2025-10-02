module;

#include <limits>
#include <optional>
#include <string>
#include <unordered_map>

export module pludux:series.default_method_context;

import :series.method_registry;

export namespace pludux {

class DefaultMethodContext {
public:
  using DispatchResultType = double;

  explicit DefaultMethodContext(const MethodRegistry& methods) noexcept
  : methods_{methods}
  {
  }

  auto dispatch_call(const std::string& name,
                     AssetSnapshot asset_data) const noexcept
   -> DispatchResultType
  {
    if(const auto method_opt = methods_.get(name); method_opt.has_value()) {
      const auto& method = method_opt.value();
      return method(asset_data, *this);
    }
    return std::numeric_limits<DispatchResultType>::quiet_NaN();
  }

  auto dispatch_call(const std::string& name,
                     AssetSnapshot asset_data,
                     MethodOutput output) const noexcept -> DispatchResultType
  {
    if(const auto method_opt = methods_.get(name); method_opt.has_value()) {
      const auto& method = method_opt.value();
      return method(asset_data, output, *this);
    }
    return std::numeric_limits<DispatchResultType>::quiet_NaN();
  }

private:
  const MethodRegistry& methods_{};
};

} // namespace pludux