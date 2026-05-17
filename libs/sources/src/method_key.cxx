module;

#include <any>
#include <concepts>
#include <functional>
#include <type_traits>

export module pludux:method_key;

export namespace pludux {

class MethodKey {
  friend std::hash<MethodKey>;

public:
  template<typename UMethod>
  MethodKey(UMethod method)
    requires(!std::same_as<std::remove_cvref_t<UMethod>, MethodKey>) &&
             requires {
               { method == method } -> std::convertible_to<bool>;
               { method != method } -> std::convertible_to<bool>;
             }
  : impl_{std::make_any<UMethod>(std::move(method))}
  , hash_method_{[](const std::any& impl) static -> std::size_t {
    const auto& method = *std::any_cast<UMethod>(&impl);
    return hash_series_method(method);
  }}
  , equals_{[](const std::any& impl, const MethodKey& other) static -> bool {
    if(auto other_method = std::any_cast<UMethod>(&other.impl_)) {
      const auto& method = *std::any_cast<UMethod>(&impl);
      return method == *other_method;
    }
    return false;
  }}
  , not_equals_{
     [](const std::any& impl, const MethodKey& other) static -> bool {
       if(auto other_method = std::any_cast<UMethod>(&other.impl_)) {
         const auto& method = *std::any_cast<UMethod>(&impl);
         return method != *other_method;
       }
       return true;
     }}
  {
  }

  auto operator==(this const MethodKey& self, const MethodKey& other) noexcept
   -> bool
  {
    return self.equals_(self.impl_, other);
  }

  auto operator!=(this const MethodKey& self, const MethodKey& other) noexcept
   -> bool
  {
    return self.not_equals_(self.impl_, other);
  }

private:
  std::any impl_;

  std::function<auto(const std::any&)->std::size_t> hash_method_;

  std::function<auto(const std::any&, const MethodKey&)->bool> equals_;

  std::function<auto(const std::any&, const MethodKey&)->bool> not_equals_;
};

} // namespace pludux

namespace std {
template<>
struct hash<pludux::MethodKey> {
  auto operator()(const pludux::MethodKey& method) const noexcept -> size_t
  {
    return method.hash_method_(method.impl_);
  }
};
} // namespace std