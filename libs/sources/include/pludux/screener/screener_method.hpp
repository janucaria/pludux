#ifndef PLUDUX_PLUDUX_SCREENER_SCREENER_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_SCREENER_METHOD_HPP

#include <functional>
#include <memory>
#include <type_traits>
#include <vector>

#include <pludux/asset_snapshot.hpp>
#include <pludux/series.hpp>

namespace pludux::screener {

class ScreenerMethod {
public:
  template<typename UMethod>
  ScreenerMethod(UMethod impl)
  : impl_{std::make_shared<ImplModel<UMethod>>(std::move(impl))}
  {
  }

  auto operator()(AssetSnapshot asset_snapshot) const noexcept
   -> SubSeries<PolySeries<double>>;

  auto operator==(const ScreenerMethod& other) const noexcept -> bool;

  auto operator!=(const ScreenerMethod& other) const noexcept -> bool;

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

    virtual auto operator()(AssetSnapshot asset_snapshot) const noexcept
     -> SubSeries<PolySeries<double>> = 0;

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

    auto operator()(AssetSnapshot asset_snapshot) const noexcept
     -> SubSeries<PolySeries<double>> override
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

#endif