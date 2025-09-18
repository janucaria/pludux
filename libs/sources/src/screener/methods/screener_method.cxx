module;

#include <functional>
#include <memory>
#include <type_traits>
#include <vector>

export module pludux:screener.screener_method;

import :asset_snapshot;
import :series;

export namespace pludux::screener {

class ScreenerMethod {
public:
  template<typename UMethod>
    requires std::is_invocable_r_v<PolySeries<double>, UMethod, AssetSnapshot>
  ScreenerMethod(UMethod impl)
  : impl_{std::make_shared<ImplModel<UMethod>>(std::move(impl))}
  {
  }

  auto operator()(AssetSnapshot asset_snapshot) const -> PolySeries<double>
  {
    return impl_->operator()(std::move(asset_snapshot));
  }

  auto operator==(const ScreenerMethod& other) const noexcept -> bool
  {
    return impl_->operator==(other);
  }

  auto operator!=(const ScreenerMethod& other) const noexcept -> bool
  {
    return impl_->operator!=(other);
  }

  template<typename UMethod>
  friend auto screener_method_cast(const ScreenerMethod& method) noexcept
   -> const UMethod*
  {
    auto model =
     std::dynamic_pointer_cast<const ImplModel<UMethod>>(method.impl_);
    return model ? &model->impl : nullptr;
  }

private:
  struct ImplConcept {
    virtual ~ImplConcept() = default;

    virtual auto operator()(AssetSnapshot asset_snapshot) const
     -> PolySeries<double> = 0;

    virtual auto operator==(const ScreenerMethod& other) const noexcept
     -> bool = 0;

    virtual auto operator!=(const ScreenerMethod& other) const noexcept
     -> bool = 0;
  };

  template<typename TMethod>
  struct ImplModel final : ImplConcept {
    TMethod impl;

    explicit ImplModel(TMethod impl)
    : impl{std::move(impl)}
    {
    }

    auto operator()(AssetSnapshot asset_snapshot) const
     -> PolySeries<double> override
    {
      return impl(std::move(asset_snapshot));
    }

    auto operator==(const ScreenerMethod& other) const noexcept -> bool override
    {
      auto other_model = screener_method_cast<TMethod>(other);
      return other_model && impl == *other_model;
    }

    auto operator!=(const ScreenerMethod& other) const noexcept -> bool override
    {
      auto other_model = screener_method_cast<TMethod>(other);
      return !other_model || impl != *other_model;
    }
  };

  std::shared_ptr<const ImplConcept> impl_;
};

} // namespace pludux::screener