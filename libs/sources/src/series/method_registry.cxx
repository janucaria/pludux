module;

#include <optional>
#include <string>
#include <unordered_map>

export module pludux:series.method_registry;

import :series.any_method;

export namespace pludux {

class MethodRegistry {
public:
  MethodRegistry() = default;

  MethodRegistry(std::unordered_map<std::string, AnyMethod> methods)
  : methods_{std::move(methods)}
  {
  }

  auto methods(this const auto& self) noexcept
   -> const std::unordered_map<std::string, AnyMethod>&
  {
    return self.methods_;
  }

  void
  methods(this auto& self,
          std::unordered_map<std::string, AnyMethod> new_methods) noexcept
  {
    self.methods_ = std::move(new_methods);
  }

  void set(this auto& self, const std::string& name, AnyMethod method)
  {
    if(self.methods_.contains(name)) {
      self.methods_.at(name) = std::move(method);
    } else {
      self.methods_.emplace(name, std::move(method));
    }
  }

  auto get(this const auto& self, const std::string& name)
   -> std::optional<AnyMethod>
  {
    const auto it = self.methods_.find(name);
    if(it == self.methods_.end()) {
      return std::nullopt;
    }
    return it->second;
  }

  auto has(this const auto& self, const std::string& name) -> bool
  {
    return self.methods_.contains(name);
  }

private:
  std::unordered_map<std::string, AnyMethod> methods_;
};

} // namespace pludux