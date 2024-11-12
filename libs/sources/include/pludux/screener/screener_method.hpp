#ifndef PLUDUX_PLUDUX_SCREENER_SCREENER_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_SCREENER_METHOD_HPP

#include <memory>
#include <vector>

#include <pludux/asset.hpp>
#include <pludux/series.hpp>

namespace pludux::screener {

class ScreenerMethod {
public:
  template<typename T>
  ScreenerMethod(T impl)
  : impl_{std::make_shared<ImplModel<T>>(std::move(impl))}
  {
  }

  auto run_one(const Asset& asset) const -> double;

  auto run_all(const Asset& asset) const -> Series;

private:
  struct ImplConcept {
    virtual ~ImplConcept() = default;
    
    virtual auto run_one(const Asset& asset) const -> double = 0;

    virtual auto run_all(const Asset& asset) const -> Series = 0;
  };

  template<typename T>
  struct ImplModel final : ImplConcept {
    T impl;

    explicit ImplModel(T impl)
    : impl{std::move(impl)}
    {
    }

    auto run_one(const Asset& asset) const -> double final
    {
      return impl.run_one(asset);
    }

    auto run_all(const Asset& asset) const -> Series final
    {
      return impl.run_all(asset);
    }
  };

  std::shared_ptr<const ImplConcept> impl_;
};

} // namespace pludux::screener

#endif // PLUDUX_PLUDUX_SCREENER_SCREENER_METHOD_HPP
