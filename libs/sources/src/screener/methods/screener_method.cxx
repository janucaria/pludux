module;

#include <any>
#include <functional>
#include <memory>
#include <type_traits>
#include <variant>
#include <vector>

export module pludux:screener.screener_method;

import :asset_snapshot;
import :screener.method_call_context;
import :screener.method_output;

import :screener.any_method_context;

export namespace pludux::screener {

class ScreenerMethod {
public:
  using ResultType = double;

  ScreenerMethod() = default;

  template<typename UMethod>
  ScreenerMethod(UMethod impl)
  : impl_{std::make_any<UMethod>(std::move(impl))}
  , invoke_{[](
             const std::any& impl,
             AssetSnapshot asset_snapshot,
             AnyMethodContext context) static -> typename UMethod::ResultType {
    auto* method = std::any_cast<UMethod>(&impl);
    return method
            ? (*method)(asset_snapshot, context)
            : std::numeric_limits<typename UMethod::ResultType>::quiet_NaN();
  }}
  , invoke_with_output_{[](const std::any& impl,
                           AssetSnapshot asset_snapshot,
                           MethodOutput output,
                           AnyMethodContext context) static
                         -> typename UMethod::ResultType {
    auto* method = std::any_cast<UMethod>(&impl);
    return method
            ? (*method)(asset_snapshot, output, context)
            : std::numeric_limits<typename UMethod::ResultType>::quiet_NaN();
  }}
  , equals_{[](const std::any& impl,
               const ScreenerMethod& other) static -> bool {
    if(auto other_method = std::any_cast<UMethod>(&other.impl_)) {
      const auto& method = std::any_cast<UMethod>(impl);
      return method == *other_method;
    }
    return false;
  }}
  , not_equals_{
     [](const std::any& impl, const ScreenerMethod& other) static -> bool {
       if(auto other_method = std::any_cast<UMethod>(&other.impl_)) {
         const auto& method = std::any_cast<UMethod>(impl);
         return method != *other_method;
       }
       return true;
     }}
  {
  }

  auto operator()(this const auto& self,
                  AssetSnapshot asset_snapshot,
                  AnyMethodContext context) -> ResultType
  {
    return self.invoke_(self.impl_, asset_snapshot, context);
  }

  auto operator()(this const auto& self,
                  AssetSnapshot asset_snapshot,
                  MethodOutput output,
                  AnyMethodContext context) -> ResultType
  {
    return self.invoke_with_output_(
     self.impl_, asset_snapshot, output, context);
  }

  auto operator==(this const auto& self, const ScreenerMethod& other) noexcept
   -> bool
  {
    return self.equals_(self.impl_, other);
  }

  auto operator!=(this const auto& self, const ScreenerMethod& other) noexcept
   -> bool
  {
    return self.not_equals_(self.impl_, other);
  }

  template<typename UMethod>
  friend auto screener_method_cast(const ScreenerMethod& method) noexcept
   -> const UMethod*
  {
    return std::any_cast<const UMethod>(&method.impl_);
  }

  template<typename UMethod>
  friend auto screener_method_cast(ScreenerMethod& method) noexcept -> UMethod*
  {
    return std::any_cast<UMethod>(&method.impl_);
  }

private:
  std::any impl_;

  std::function<
   auto(const std::any&, AssetSnapshot, AnyMethodContext)->ResultType>
   invoke_;

  std::function<auto(
                 const std::any&, AssetSnapshot, MethodOutput, AnyMethodContext)
                 ->ResultType>
   invoke_with_output_;

  std::function<auto(const std::any&, const ScreenerMethod&)->bool> equals_;

  std::function<auto(const std::any&, const ScreenerMethod&)->bool> not_equals_;
};

} // namespace pludux::screener