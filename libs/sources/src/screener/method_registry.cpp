

#include <pludux/screener/method_registry.hpp>

namespace pludux::screener {

MethodRegistry::MethodRegistry() = default;

MethodRegistry::MethodRegistry(
 std::unordered_map<std::string, ScreenerMethod> methods)
: methods_{std::move(methods)}
{
}

auto MethodRegistry::methods() const noexcept
 -> const std::unordered_map<std::string, ScreenerMethod>&
{
  return methods_;
}

void MethodRegistry::methods(
 std::unordered_map<std::string, ScreenerMethod> new_methods) noexcept
{
  methods_ = std::move(new_methods);
}

void MethodRegistry::set(const std::string& name, ScreenerMethod method)
{
  if(methods_.contains(name)) {
    methods_.at(name) = std::move(method);
  } else {
    methods_.emplace(name, std::move(method));
  }
}

auto MethodRegistry::get(const std::string& name) const
 -> std::optional<ScreenerMethod>
{
  const auto it = methods_.find(name);
  if(it == methods_.end()) {
    return std::nullopt;
  }
  return it->second;
}

auto MethodRegistry::has(const std::string& name) const -> bool
{
  return methods_.contains(name);
}

} // namespace pludux::screener