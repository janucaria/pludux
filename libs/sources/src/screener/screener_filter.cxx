module;

#include <functional>
#include <memory>
#include <type_traits>
#include <vector>

export module pludux.screener.screener_filter;

import pludux.asset_snapshot;

export namespace pludux::screener {

class ScreenerFilter {
public:
  template<typename UImpl>
    requires std::is_invocable_r_v<bool, UImpl, AssetSnapshot>
  ScreenerFilter(UImpl impl)
  : impl_{std::make_shared<ImplModel<UImpl>>(std::move(impl))}
  {
  }

  auto operator()(AssetSnapshot asset_snapshot) const noexcept -> bool
  {
    return impl_->operator()(std::move(asset_snapshot));
  }

  auto operator==(const ScreenerFilter& other) const noexcept -> bool
  {
    return impl_->operator==(other);
  }

  auto operator!=(const ScreenerFilter& other) const noexcept -> bool
  {
    return impl_->operator!=(other);
  }

  template<typename UImpl>
  friend auto screener_filter_cast(const ScreenerFilter& method) noexcept
   -> const UImpl*
  {
    auto model =
     std::dynamic_pointer_cast<const ImplModel<UImpl>>(method.impl_);
    return model ? &model->impl : nullptr;
  }

private:
  struct ImplConcept {
    virtual ~ImplConcept() = default;

    virtual auto operator()(AssetSnapshot asset_snapshot) const noexcept
     -> bool = 0;

    virtual auto operator==(const ScreenerFilter& other) const noexcept
     -> bool = 0;

    virtual auto operator!=(const ScreenerFilter& other) const noexcept
     -> bool = 0;
  };

  template<typename TImpl>
  struct ImplModel final : ImplConcept {
    TImpl impl;

    explicit ImplModel(TImpl impl)
    : impl{std::move(impl)}
    {
    }

    auto operator()(AssetSnapshot asset_snapshot) const noexcept
     -> bool override
    {
      return impl(std::move(asset_snapshot));
    }

    auto operator==(const ScreenerFilter& other) const noexcept -> bool override
    {
      auto other_model = screener_filter_cast<TImpl>(other);
      return other_model && impl == *other_model;
    }

    auto operator!=(const ScreenerFilter& other) const noexcept -> bool override
    {
      auto other_model = screener_filter_cast<TImpl>(other);
      return !other_model || impl != *other_model;
    }
  };

  std::shared_ptr<const ImplConcept> impl_;
};

} // namespace pludux::screener
