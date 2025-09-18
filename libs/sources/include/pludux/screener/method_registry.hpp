#ifndef PLUDUX_PLUDUX_SCREENER_METHOD_REGISTRY_HPP
#define PLUDUX_PLUDUX_SCREENER_METHOD_REGISTRY_HPP

#include <optional>
#include <string>
#include <unordered_map>

#include <pludux/screener/screener_method.hpp>


namespace pludux::screener {

class MethodRegistry {
public:
  MethodRegistry();

  MethodRegistry(std::unordered_map<std::string, ScreenerMethod> methods);

  auto methods() const noexcept
   -> const std::unordered_map<std::string, ScreenerMethod>&;

  void methods(std::unordered_map<std::string, ScreenerMethod> new_methods) noexcept;

  void set(const std::string& name, ScreenerMethod method);

  auto get(const std::string& name) const -> std::optional<ScreenerMethod>;

  auto has(const std::string& name) const -> bool;

private:
  std::unordered_map<std::string, ScreenerMethod> methods_;
};

} // namespace pludux::screener

#endif
