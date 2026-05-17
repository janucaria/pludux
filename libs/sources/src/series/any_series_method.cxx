module;

#include <any>
#include <concepts>
#include <functional>
#include <memory>
#include <type_traits>
#include <variant>
#include <vector>

export module pludux:series.any_series_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;
import :any_method_context;

import :series.ohlcv_method;

export namespace pludux {

class AnySeriesMethod {
public:
  template<typename UMethod = CloseMethod>
    requires(!std::same_as<std::remove_cvref_t<UMethod>, AnySeriesMethod>) &&
             requires(UMethod method,
                      AssetSnapshot asset_snapshot,
                      SeriesOutput output,
                      AnySeriesMethodContext context) {
               // TODO: compile error on clang and emscripten
               //  {
               //    evaluate_series_method(method, asset_snapshot, context)
               //  } -> std::convertible_to<double>;
               { method == method } -> std::convertible_to<bool>;
               { method != method } -> std::convertible_to<bool>;
             }
  AnySeriesMethod(UMethod impl = UMethod{})
  : impl_{std::make_any<UMethod>(std::move(impl))}
  , evaluate_{[](const std::any& impl,
                 AssetSnapshot asset_snapshot,
                 AnySeriesMethodContext context) static -> double {
    const auto& method = *std::any_cast<UMethod>(&impl);
    return evaluate_series_method(method, asset_snapshot, context);
  }}
  , evaluate_with_output_{[](const std::any& impl,
                             SeriesOutput output,
                             AssetSnapshot asset_snapshot,
                             AnySeriesMethodContext context) static -> double {
    const auto& method = *std::any_cast<UMethod>(&impl);
    return evaluate_selected_output_series_or_nan(
     method, output, asset_snapshot, context);
  }}
  , hash_series_method_{[](const std::any& impl) static -> std::size_t {
    const auto& method = *std::any_cast<UMethod>(&impl);
    return hash_series_method(method);
  }}
  , equals_{[](const std::any& impl,
               const AnySeriesMethod& other) static -> bool {
    if(auto other_method = std::any_cast<UMethod>(&other.impl_)) {
      const auto& method = *std::any_cast<UMethod>(&impl);
      return method == *other_method;
    }
    return false;
  }}
  , not_equals_{
     [](const std::any& impl, const AnySeriesMethod& other) static -> bool {
       if(auto other_method = std::any_cast<UMethod>(&other.impl_)) {
         const auto& method = *std::any_cast<UMethod>(&impl);
         return method != *other_method;
       }
       return true;
     }}
  {
  }

  friend auto evaluate_series_method(const AnySeriesMethod& method,
                                     AssetSnapshot asset_snapshot,
                                     MethodContextable auto context) -> double
  {
    return method.evaluate_(method.impl_, asset_snapshot, context);
  }

  friend auto evaluate_series_method(SeriesOutput output,
                                     const AnySeriesMethod& method,
                                     AssetSnapshot asset_snapshot,
                                     MethodContextable auto context) -> double
  {
    return method.evaluate_with_output_(
     method.impl_, output, asset_snapshot, context);
  }

  friend auto hash_series_method(const AnySeriesMethod& method) -> size_t
  {
    return method.hash_series_method_(method.impl_);
  }

  auto operator==(this const AnySeriesMethod& self,
                  const AnySeriesMethod& other) noexcept -> bool
  {
    return self.equals_(self.impl_, other);
  }

  auto operator!=(this const AnySeriesMethod& self,
                  const AnySeriesMethod& other) noexcept -> bool
  {
    return self.not_equals_(self.impl_, other);
  }

  template<typename UMethod>
  friend auto series_method_cast(const AnySeriesMethod& method) noexcept
   -> const UMethod*
  {
    return std::any_cast<const UMethod>(&method.impl_);
  }

  template<typename UMethod>
  friend auto series_method_cast(AnySeriesMethod& method) noexcept -> UMethod*
  {
    return std::any_cast<UMethod>(&method.impl_);
  }

private:
  std::any impl_;

  std::function<
   auto(const std::any&, AssetSnapshot, AnySeriesMethodContext)->double>
   evaluate_;

  std::function<
   auto(const std::any&, SeriesOutput, AssetSnapshot, AnySeriesMethodContext)
    ->double>
   evaluate_with_output_;

  std::function<auto(const std::any&)->std::size_t> hash_series_method_;

  std::function<auto(const std::any&, const AnySeriesMethod&)->bool> equals_;

  std::function<auto(const std::any&, const AnySeriesMethod&)->bool>
   not_equals_;
};

} // namespace pludux
