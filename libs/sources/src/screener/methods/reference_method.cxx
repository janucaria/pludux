module;

#include <limits>
#include <memory>
#include <string>
#include <utility>

export module pludux:screener.reference_method;

import :asset_snapshot;
import :screener.method_registry;

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

  auto operator()(this const auto& self, AssetSnapshot asset_data)
   -> PolySeries<double>
  {
    if(self.registry_) {
      const auto method_opt = self.registry_->get(self.name_);
      if(method_opt.has_value()) {
        const auto& method = method_opt.value();
        return method(asset_data);
      }
    }

    return RepeatSeries<double>{std::numeric_limits<double>::quiet_NaN(),
                                asset_data.size()};
  }

  auto registry(this const auto& self) noexcept
   -> std::shared_ptr<const MethodRegistry>
  {
    return self.registry_;
  }

  void registry(this auto& self,
                std::shared_ptr<const MethodRegistry> new_registry) noexcept
  {
    self.registry_ = std::move(new_registry);
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
  std::shared_ptr<const MethodRegistry> registry_;
  std::string name_{};
};

} // namespace pludux::screener