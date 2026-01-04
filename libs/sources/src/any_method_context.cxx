module;

#include <any>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <type_traits>

#include <any>
#include <functional>
#include <memory>
#include <type_traits>
#include <variant>
#include <vector>

export module pludux:any_method_context;

import :asset_snapshot;
import :series_output;

export namespace pludux {

class AnySeriesMethodContext {
public:
  using DispatchResultType = double;

  AnySeriesMethodContext() = default;

  template<typename UImpl>
  AnySeriesMethodContext(UImpl impl)
  : impl_{std::move(impl)}
  , call_series_method_no_output_{[](const std::any& impl,
                                     const std::string& name,
                                     AssetSnapshot asset_snapshot)
                                   -> DispatchResultType {
    return std::any_cast<UImpl>(impl).call_series_method(
     name, std::move(asset_snapshot));
  }}
  , call_series_method_with_output_{
     [](const std::any& impl,
        const std::string& name,
        AssetSnapshot asset_snapshot,
        SeriesOutput output) -> DispatchResultType {
       return std::any_cast<UImpl>(impl).call_series_method(
        name, std::move(asset_snapshot), output);
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
                          SeriesOutput output_name) noexcept
   -> DispatchResultType
  {
    return self.call_series_method_with_output_(
     self.impl_, name, std::move(asset_snapshot), output_name);
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
   auto(const std::any&, const std::string&, AssetSnapshot)->DispatchResultType>
   call_series_method_no_output_;

  std::function<
   auto(const std::any&, const std::string&, AssetSnapshot, SeriesOutput)
    ->DispatchResultType>
   call_series_method_with_output_;
};

} // namespace pludux
