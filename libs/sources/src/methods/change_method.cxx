module;

#include <utility>

export module pludux:methods.change_method;

import :methods.ohlcv_method;

export namespace pludux {

template<typename TSourceMethod = CloseMethod>
class ChangeMethod {
public:
  ChangeMethod()
  : ChangeMethod{TSourceMethod{}}
  {
  }

  explicit ChangeMethod(TSourceMethod source)
  : source_{std::move(source)}
  {
  }

  auto operator==(const ChangeMethod& other) const noexcept -> bool = default;

  auto source(this const ChangeMethod& self) noexcept -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this ChangeMethod& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

private:
  TSourceMethod source_;
};

} // namespace pludux