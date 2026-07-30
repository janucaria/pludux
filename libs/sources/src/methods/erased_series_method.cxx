module;

#include <any>
#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

export module pludux:methods.erased_series_method;

import :asset_snapshot;
import :method_contextable;

import :methods.ohlcv_method;
import :methods.select_output_method;

import :evaluate_series_method;
import :hash_series_method;

export namespace pludux {

template<MethodContextable TContext>
class ErasedSeriesMethod {
public:
  ErasedSeriesMethod()
  : ErasedSeriesMethod{CloseMethod{}}
  {
  }

  template<typename UMethod>
    requires(!std::same_as<std::remove_cvref_t<UMethod>, ErasedSeriesMethod>) &&
             (!std::same_as<std::remove_cvref_t<UMethod>,
                            std::vector<ErasedSeriesMethod>>) &&
             std::equality_comparable<UMethod>
  ErasedSeriesMethod(UMethod impl)
  : impl_{std::make_any<UMethod>(std::move(impl))}
  , evaluate_{[](const std::any& impl,
                 AssetSnapshot asset_snapshot,
                 TContext context) static -> double {
    const auto& method = *std::any_cast<UMethod>(&impl);
    return evaluate_series_method(method, asset_snapshot, context);
  }}
  , evaluate_with_output_{[](const std::any& impl,
                             MethodOutput output,
                             AssetSnapshot asset_snapshot,
                             TContext context) static -> double {
    const auto& method = *std::any_cast<UMethod>(&impl);
    return evaluate_series_method(output, method, asset_snapshot, context);
  }}
  , hash_series_method_{[](const std::any& impl) static -> std::size_t {
    const auto& method = *std::any_cast<UMethod>(&impl);
    return hash_series_method(method);
  }}
  , equals_{[](const std::any& impl,
               const ErasedSeriesMethod& other) static -> bool {
    if(auto other_method = std::any_cast<UMethod>(&other.impl_)) {
      const auto& method = *std::any_cast<UMethod>(&impl);
      return method == *other_method;
    }
    return false;
  }}
  , not_equals_{
     [](const std::any& impl, const ErasedSeriesMethod& other) static -> bool {
       if(auto other_method = std::any_cast<UMethod>(&other.impl_)) {
         const auto& method = *std::any_cast<UMethod>(&impl);
         return method != *other_method;
       }
       return true;
     }}
  {
  }

  friend auto pludux_tag_invoke(EvaluateSeriesMethod,
                                const ErasedSeriesMethod& method,
                                AssetSnapshot asset_snapshot,
                                TContext context) -> double
  {
    return method.evaluate_(method.impl_, asset_snapshot, context);
  }

  friend auto pludux_tag_invoke(EvaluateSeriesMethod,
                                MethodOutput output,
                                const ErasedSeriesMethod& method,
                                AssetSnapshot asset_snapshot,
                                TContext context) -> double
  {
    return method.evaluate_with_output_(
     method.impl_, output, asset_snapshot, context);
  }

  friend auto hash_series_method(const ErasedSeriesMethod& method) -> size_t
  {
    return method.hash_series_method_(method.impl_);
  }

  auto operator==(this const ErasedSeriesMethod& self,
                  const ErasedSeriesMethod& other) noexcept -> bool
  {
    return self.equals_(self.impl_, other);
  }

  auto operator!=(this const ErasedSeriesMethod& self,
                  const ErasedSeriesMethod& other) noexcept -> bool
  {
    return self.not_equals_(self.impl_, other);
  }

  template<typename UMethod>
  friend auto series_method_cast(const ErasedSeriesMethod& method) noexcept
   -> const UMethod*
  {
    return std::any_cast<const UMethod>(&method.impl_);
  }

  template<typename UMethod>
  friend auto series_method_cast(ErasedSeriesMethod& method) noexcept
   -> UMethod*
  {
    return std::any_cast<UMethod>(&method.impl_);
  }

private:
  std::any impl_;

  std::function<auto(const std::any&, AssetSnapshot, TContext)->double>
   evaluate_;

  std::function<
   auto(const std::any&, MethodOutput, AssetSnapshot, TContext)->double>
   evaluate_with_output_;

  std::function<auto(const std::any&)->std::size_t> hash_series_method_;

  std::function<auto(const std::any&, const ErasedSeriesMethod&)->bool> equals_;

  std::function<auto(const std::any&, const ErasedSeriesMethod&)->bool>
   not_equals_;
};

} // namespace pludux
