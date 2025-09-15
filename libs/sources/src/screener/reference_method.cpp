#include <utility>

#include <pludux/asset_snapshot.hpp>

#include <pludux/screener/reference_method.hpp>

namespace pludux::screener {

ReferenceMethod::ReferenceMethod(std::shared_ptr<const MethodRegistry> registry,
                                 std::string name,
                                 std::size_t offset)
: registry_{std::move(registry)}
, name_{std::move(name)}
, offset_{offset}
{
}

auto ReferenceMethod::operator()(AssetSnapshot asset_data) const
 -> PolySeries<double>
{
  auto series = PolySeries<double>{RepeatSeries<double>{
   std::numeric_limits<double>::quiet_NaN(), asset_data.size()}};

  if(registry_) {
    const auto method_opt = registry_->get(name_);
    if(method_opt.has_value()) {
      const auto& method = method_opt.value();
      series = method(asset_data);
    }
  }

  return LookbackSeries{series, static_cast<std::ptrdiff_t>(offset_)};
}

auto ReferenceMethod::operator==(const ReferenceMethod& other) const noexcept
 -> bool
{
  return registry_ == other.registry_ && name_ == other.name_ &&
         offset_ == other.offset_;
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

auto ReferenceMethod::offset() const noexcept -> std::size_t
{
  return offset_;
}

void ReferenceMethod::offset(std::size_t new_offset) noexcept
{
  offset_ = new_offset;
}

} // namespace pludux::screener