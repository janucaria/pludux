module;

#include <any>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

export module pludux:any_method_context;

import :asset_snapshot;
import :method_key;

import :methods.select_output_method;

export namespace pludux {

class AnySeriesMethodContext {
public:
  using DispatchResultType = double;

  AnySeriesMethodContext() = default;

  template<typename UImpl>
  AnySeriesMethodContext(UImpl impl)
  : impl_{std::move(impl)}
  , get_series_result_{[](const std::any& impl,
                          const std::string& name,
                          std::size_t result_index) -> DispatchResultType {
    return std::any_cast<UImpl>(impl).get_series_result(name, result_index);
  }}
  , get_series_results_{[](std::any& impl,
                           MethodKey method_key) -> std::vector<double>& {
    auto* context = std::any_cast<UImpl>(&impl);
    return context->get_series_results(method_key);
  }}
  , call_series_method_no_output_{[](const std::any& impl,
                                     const std::string& name,
                                     AssetSnapshot asset_snapshot)
                                   -> DispatchResultType {
    return std::any_cast<UImpl>(impl).call_series_method(
     name, std::move(asset_snapshot));
  }}
  , call_series_method_with_output_{[](const std::any& impl,
                                       const std::string& name,
                                       AssetSnapshot asset_snapshot,
                                       MethodOutput output)
                                     -> DispatchResultType {
    return std::any_cast<UImpl>(impl).call_series_method(
     name, std::move(asset_snapshot), output);
  }}
  , get_index_func_{[](const std::any& impl) -> std::size_t {
    return std::any_cast<UImpl>(impl).index();
  }}
  , get_input_value_{
     [](const std::any& impl, const std::string& key) -> double {
       return std::any_cast<UImpl>(impl).get_input_value(key);
     }}
  {
  }

  auto call_series_method(this const AnySeriesMethodContext& self,
                          const std::string& name,
                          AssetSnapshot asset_snapshot) noexcept
   -> DispatchResultType
  {
    return self.call_series_method_no_output_(
     self.impl_, name, std::move(asset_snapshot));
  }

  auto call_series_method(this const AnySeriesMethodContext& self,
                          const std::string& name,
                          AssetSnapshot asset_snapshot,
                          MethodOutput output_name) noexcept
   -> DispatchResultType
  {
    return self.call_series_method_with_output_(
     self.impl_, name, std::move(asset_snapshot), output_name);
  }

  auto get_series_result(this const AnySeriesMethodContext& self,
                         const std::string& name,
                         std::size_t result_index) noexcept
   -> DispatchResultType
  {
    return self.get_series_result_(self.impl_, name, result_index);
  }

  auto get_series_results(this AnySeriesMethodContext& self,
                          MethodKey method_key) noexcept -> std::vector<double>&
  {
    return self.get_series_results_(self.impl_, method_key);
  }

  auto index(this const AnySeriesMethodContext& self) noexcept -> std::size_t
  {
    return self.get_index_func_(self.impl_);
  }

  auto get_input_value(this const AnySeriesMethodContext& self,
                       const std::string& key) noexcept -> double
  {
    return self.get_input_value_(self.impl_, key);
  }

  template<typename UImpl>
  friend auto
  series_method_context_cast(const AnySeriesMethodContext& method) noexcept
   -> const UImpl*
  {
    return std::any_cast<const UImpl>(&method.impl_);
  }

  template<typename UImpl>
  friend auto
  series_method_context_cast(AnySeriesMethodContext& method) noexcept -> UImpl*
  {
    return std::any_cast<UImpl>(&method.impl_);
  }

private:
  std::any impl_;

  std::function<
   auto(const std::any&, const std::string&, std::size_t)->DispatchResultType>
   get_series_result_;

  std::function<auto(std::any&, MethodKey)->std::vector<double>&>
   get_series_results_;

  std::function<
   auto(const std::any&, const std::string&, AssetSnapshot)->DispatchResultType>
   call_series_method_no_output_;

  std::function<
   auto(const std::any&, const std::string&, AssetSnapshot, MethodOutput)
    ->DispatchResultType>
   call_series_method_with_output_;

  std::function<auto(const std::any&)->std::size_t> get_index_func_;

  std::function<auto(const std::any&, const std::string&)->double>
   get_input_value_;
};

} // namespace pludux
