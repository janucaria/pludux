#ifndef PLUDUX_PLUDUX_SERIES_POLY_SERIES_HPP
#define PLUDUX_PLUDUX_SERIES_POLY_SERIES_HPP

#include <cstddef>
#include <initializer_list>
#include <memory>
#include <utility>

#include "data_series.hpp"

namespace pludux {

template<typename T>
class PolySeries {
public:
  using ValueType = T;

  template<typename USeries>
    requires requires(USeries impl) {
      { impl.size() } -> std::convertible_to<std::size_t>;
      { impl[std::declval<std::size_t>()] } -> std::convertible_to<ValueType>;
    }
  PolySeries(USeries impl)
  : impl_{std::make_shared<ImplModel<USeries>>(std::move(impl))}
  {
  }

  PolySeries(std::initializer_list<ValueType> data)
  : PolySeries(DataSeries<ValueType>(data))
  {
  }

  auto operator[](std::size_t index) const noexcept -> ValueType
  {
    return static_cast<double>(impl_->operator[](index));
  }

  auto size() const noexcept -> std::size_t
  {
    return impl_->size();
  }

  template<typename USeries>
  friend auto screener_cast(const PolySeries* method) noexcept -> const USeries*
  {
    auto model =
     std::dynamic_pointer_cast<const ImplModel<USeries>>(method->impl_);
    return model ? &model->impl : nullptr;
  }

private:
  struct ImplConcept {
    virtual ~ImplConcept() = default;

    virtual auto operator[](std::size_t index) const noexcept -> ValueType = 0;

    virtual auto size() const noexcept -> std::size_t = 0;
  };

  template<typename TSeries>
  struct ImplModel final : ImplConcept {
    TSeries impl;

    explicit ImplModel(TSeries impl)
    : impl{std::move(impl)}
    {
    }

    auto operator[](std::size_t index) const noexcept -> ValueType override
    {
      return static_cast<ValueType>(impl[index]);
    }

    virtual auto size() const noexcept -> std::size_t override
    {
      return impl.size();
    }
  };

  std::shared_ptr<const ImplConcept> impl_;
};

} // namespace pludux

#endif