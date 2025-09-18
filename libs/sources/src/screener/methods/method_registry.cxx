module;

#include <optional>
#include <string>
#include <unordered_map>

export module pludux:screener.method_registry;

import :screener.screener_method;

export namespace pludux::screener {

class MethodRegistry {
public:
  MethodRegistry() = default;

  MethodRegistry(std::unordered_map<std::string, ScreenerMethod> methods)
  : methods_{std::move(methods)}
  {
  }

  auto methods() const noexcept
   -> const std::unordered_map<std::string, ScreenerMethod>&
  {
    return methods_;
  }

  void
  methods(std::unordered_map<std::string, ScreenerMethod> new_methods) noexcept
  {
    methods_ = std::move(new_methods);
  }

  void set(const std::string& name, ScreenerMethod method)
  {
    if(methods_.contains(name)) {
      methods_.at(name) = std::move(method);
    } else {
      methods_.emplace(name, std::move(method));
    }
  }

  auto get(const std::string& name) const -> std::optional<ScreenerMethod>
  {
    const auto it = methods_.find(name);
    if(it == methods_.end()) {
      return std::nullopt;
    }
    return it->second;
  }

  auto has(const std::string& name) const -> bool
  {
    return methods_.contains(name);
  }

private:
  std::unordered_map<std::string, ScreenerMethod> methods_;
};

} // namespace pludux::screener