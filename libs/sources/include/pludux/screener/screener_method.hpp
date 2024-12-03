#ifndef PLUDUX_PLUDUX_SCREENER_SCREENER_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_SCREENER_METHOD_HPP

#include <memory>
#include <vector>

#include <pludux/asset.hpp>
#include <pludux/series.hpp>
#include <pludux/screener/asset_data_provider.hpp>

namespace pludux::screener {

class ScreenerMethod {
public:
  template<typename T>
  ScreenerMethod(T impl)
  : impl_{std::make_shared<ImplModel<T>>(std::move(impl))}
  {
  }

  auto run_one(const AssetDataProvider& asset_data) const -> double;

  auto run_all(const AssetDataProvider& asset_data) const -> Series;

  template<typename T>
  friend auto screener_method_cast(const ScreenerMethod* method) noexcept -> const T*
  {
    auto model = std::dynamic_pointer_cast<const ImplModel<T>>(method->impl_);
    return model ? &model->impl : nullptr;
  }

private:
  struct ImplConcept {
    virtual ~ImplConcept() = default;

    virtual auto run_one(const AssetDataProvider& asset_data) const -> double = 0;
    
    virtual auto run_all(const AssetDataProvider& asset_data) const -> Series = 0;
  };

  template<typename T>
  struct ImplModel final : ImplConcept {
    T impl;

    explicit ImplModel(T impl)
    : impl{std::move(impl)}
    {
    }

    auto run_one(const AssetDataProvider& asset_data) const -> double final
    {
      return impl.run_one(asset_data);
    }

    auto run_all(const AssetDataProvider& asset_data) const -> Series final
    {
      return impl.run_all(asset_data);
    }
  };

  std::shared_ptr<const ImplConcept> impl_;
};

} // namespace pludux::screener

#endif // PLUDUX_PLUDUX_SCREENER_SCREENER_METHOD_HPP
