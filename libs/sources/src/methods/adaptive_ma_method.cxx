module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:methods.adaptive_ma_method;

import :methods.ohlcv_method;

export namespace pludux {

enum class MaMethodType { Sma, Ema, Wma, Rma, Hma };

template<typename TSourceMethod = CloseMethod>
class AdaptiveMaMethod {
public:
  AdaptiveMaMethod()
  : AdaptiveMaMethod{20}
  {
  }

  explicit AdaptiveMaMethod(std::size_t period)
  : AdaptiveMaMethod{TSourceMethod{}, period}
  {
  }

  AdaptiveMaMethod(TSourceMethod source, std::size_t period)
  : AdaptiveMaMethod{std::move(source), MaMethodType::Sma, period}
  {
  }

  AdaptiveMaMethod(TSourceMethod source,
                   MaMethodType ma_type,
                   std::size_t period)
  : source_{std::move(source)}
  , ma_type_{ma_type}
  , period_{period}
  {
  }

  auto operator==(const AdaptiveMaMethod& other) const noexcept
   -> bool = default;

  auto ma_type(this const AdaptiveMaMethod& self) noexcept -> MaMethodType
  {
    return self.ma_type_;
  }

  void ma_type(this AdaptiveMaMethod& self, MaMethodType ma_type) noexcept
  {
    self.ma_type_ = ma_type;
  }

  auto source(this const AdaptiveMaMethod& self) noexcept
   -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this AdaptiveMaMethod& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const AdaptiveMaMethod& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this AdaptiveMaMethod& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

private:
  TSourceMethod source_;
  MaMethodType ma_type_;
  std::size_t period_;
};

} // namespace pludux
