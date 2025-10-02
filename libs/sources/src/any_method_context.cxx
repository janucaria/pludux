module;

#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <type_traits>

export module pludux:any_method_context;

import :asset_snapshot;
import :series_output;

export namespace pludux {

class AnySeriesMethodContext {
public:
  using DispatchResultType = double;

  AnySeriesMethodContext()
  : impl_{nullptr}
  {
  }

  template<typename UImpl>
  AnySeriesMethodContext(UImpl impl)
  : impl_{std::make_shared<ImplModel<UImpl>>(std::move(impl))}
  {
  }

  auto call_series_method(this const auto& self,
                          const std::string& name,
                          AssetSnapshot asset_snapshot) noexcept
   -> DispatchResultType
  {
    return self.impl_
            ? self.impl_->call_series_method(name, std::move(asset_snapshot))
            : std::numeric_limits<DispatchResultType>::quiet_NaN();
  }

  auto call_series_method(this const auto& self,
                          const std::string& name,
                          AssetSnapshot asset_snapshot,
                          SeriesOutput output_name) noexcept
   -> DispatchResultType
  {
    return self.impl_ ? self.impl_->call_series_method(
                         name, std::move(asset_snapshot), output_name)
                      : std::numeric_limits<DispatchResultType>::quiet_NaN();
  }

  template<typename UImpl>
  friend auto method_context_cast(const AnySeriesMethodContext& method) noexcept
   -> const UImpl*
  {
    auto model =
     std::dynamic_pointer_cast<const ImplModel<UImpl>>(method.impl_);
    return model ? &model->impl : nullptr;
  }

private:
  struct ImplConcept {
    virtual ~ImplConcept() = default;

    virtual auto call_series_method(const std::string& name,
                                    AssetSnapshot asset_snapshot) const noexcept
     -> DispatchResultType = 0;

    virtual auto call_series_method(const std::string& name,
                                    AssetSnapshot asset_snapshot,
                                    SeriesOutput output_name) const noexcept
     -> DispatchResultType = 0;
  };

  template<typename TImpl>
  struct ImplModel final : ImplConcept {
    TImpl impl;

    explicit ImplModel(TImpl impl)
    : impl{std::move(impl)}
    {
    }

    auto call_series_method(const std::string& name,
                            AssetSnapshot asset_snapshot) const noexcept
     -> DispatchResultType override
    {
      return impl.call_series_method(name, std::move(asset_snapshot));
    }

    auto call_series_method(const std::string& name,
                            AssetSnapshot asset_snapshot,
                            SeriesOutput output_name) const noexcept
     -> DispatchResultType override
    {
      return impl.call_series_method(
       name, std::move(asset_snapshot), output_name);
    }
  };

  std::shared_ptr<const ImplConcept> impl_;
};

} // namespace pludux
