module;

#include <cassert>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:methods.atr_method;

import :methods.adaptive_ma_method;
import :methods.value_method;

export namespace pludux {

template<typename TPeriodMethod = ValueMethod>
class AtrMethod {
public:
  AtrMethod()
  : AtrMethod{14}
  {
  }

  explicit AtrMethod(std::size_t period,
                     MaMethodType ma_smoothing_type = MaMethodType::Rma)
  : AtrMethod{ValueMethod{static_cast<double>(period)}, ma_smoothing_type}
  {
  }

  explicit AtrMethod(TPeriodMethod period,
                     MaMethodType ma_smoothing_type = MaMethodType::Rma)
    requires(!std::is_arithmetic_v<TPeriodMethod>)
  : period_{std::move(period)}
  , ma_smoothing_type_{ma_smoothing_type}
  {
  }

  auto operator==(const AtrMethod& other) const noexcept -> bool = default;

  auto period(this const AtrMethod& self) noexcept -> const TPeriodMethod&
  {
    return self.period_;
  }

  void period(this AtrMethod& self, TPeriodMethod new_period) noexcept
  {
    self.period_ = std::move(new_period);
  }

  auto ma_smoothing_type(this const AtrMethod& self) noexcept -> MaMethodType
  {
    return self.ma_smoothing_type_;
  }

  void ma_smoothing_type(this AtrMethod& self, MaMethodType new_type) noexcept
  {
    self.ma_smoothing_type_ = new_type;
  }

private:
  TPeriodMethod period_;
  MaMethodType ma_smoothing_type_;
};

} // namespace pludux
