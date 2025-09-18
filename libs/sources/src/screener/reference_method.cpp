#include <utility>


#include <pludux/screener/reference_method.hpp>

namespace pludux::screener {

ReferenceMethod::ReferenceMethod(std::shared_ptr<const MethodRegistry> registry,
                                 std::string name)
: registry_{std::move(registry)}
, name_{std::move(name)}
{
}

auto ReferenceMethod::operator==(const ReferenceMethod& other) const noexcept
 -> bool = default;

auto ReferenceMethod::operator()(AssetSnapshot asset_data) const
 -> PolySeries<double>
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

auto ReferenceMethod::registry() const noexcept
 -> std::shared_ptr<const MethodRegistry>
{
  return registry_;
}

void ReferenceMethod::registry(
 std::shared_ptr<const MethodRegistry> new_registry) noexcept
{
  registry_ = std::move(new_registry);
}

auto ReferenceMethod::name() const noexcept -> const std::string&
{
  return name_;
}

void ReferenceMethod::name(std::string new_name) noexcept
{
  name_ = std::move(new_name);
}

} // namespace pludux::screener