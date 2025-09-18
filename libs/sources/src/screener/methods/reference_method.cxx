module;

#include <limits>
#include <memory>
#include <string>
#include <utility>

export module pludux.screener.reference_method;

import pludux.asset_snapshot;
import pludux.screener.method_registry;

export namespace pludux::screener {

class ReferenceMethod {
public:
  ReferenceMethod(std::shared_ptr<const MethodRegistry> registry,
                  std::string name)
  : registry_{std::move(registry)}
  , name_{std::move(name)}
  {
  }

  auto operator==(const ReferenceMethod& other) const noexcept
   -> bool = default;

  auto operator()(AssetSnapshot asset_data) const -> PolySeries<double>
  {
    if(registry_) {
      const auto method_opt = registry_->get(name_);
      if(method_opt.has_value()) {
        const auto& method = method_opt.value();
        return method(asset_data);
      }
    }

    return RepeatSeries<double>{std::numeric_limits<double>::quiet_NaN(),
                                asset_data.size()};
  }

  auto registry() const noexcept -> std::shared_ptr<const MethodRegistry>
  {
    return registry_;
  }

  void registry(std::shared_ptr<const MethodRegistry> new_registry) noexcept
  {
    registry_ = std::move(new_registry);
  }

  auto name() const noexcept -> const std::string&
  {
    return name_;
  }

  void name(std::string new_name) noexcept
  {
    name_ = std::move(new_name);
  }

private:
  std::shared_ptr<const MethodRegistry> registry_;
  std::string name_{};
};

} // namespace pludux::screener