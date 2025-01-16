#ifndef PLUDUX_PLUDUX_SERIES_POLY_SERIES_HPP
#define PLUDUX_PLUDUX_SERIES_POLY_SERIES_HPP

#include <cstddef>
#include <memory>
#include <utility>

namespace pludux {

template<typename T>
class PolySeries {
public:
  using ValueType = T;

  template<typename T>
  PolySeries(T impl)
  : impl_{std::make_shared<ImplModel<T>>(std::move(impl))}
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

  template<typename T>
  friend auto
  screener_method_cast(const PolySeries* method) noexcept -> const T*
  {
    auto model = std::dynamic_pointer_cast<const ImplModel<T>>(method->impl_);
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
    using ValueType = typename TSeries::ValueType;

    TSeries impl;

    explicit ImplModel(TSeries impl)
    : impl{std::move(impl)}
    {
    }

    auto operator[](std::size_t index) const noexcept -> ValueType override
    {
      return impl[index];
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