module;

#include <cstddef>
#include <utility>

export module pludux:methods.donchian_channel_method;

import :methods.value_method;

export namespace pludux {

template<typename TPeriodMethod = ValueMethod>
class DonchianChannelMethod {
public:
  DonchianChannelMethod()
  : DonchianChannelMethod{20}
  {
  }

  DonchianChannelMethod(std::size_t period)
  : DonchianChannelMethod{ValueMethod{static_cast<double>(period)}}
  {
  }

  explicit DonchianChannelMethod(TPeriodMethod period)
  requires(!std::is_arithmetic_v<TPeriodMethod>)
  : period_{std::move(period)}
  {
  }

  auto operator==(const DonchianChannelMethod&) const noexcept
   -> bool = default;

  auto period(this const DonchianChannelMethod& self) noexcept
   -> const TPeriodMethod&
  {
    return self.period_;
  }

  void period(this DonchianChannelMethod& self, TPeriodMethod period) noexcept
  {
    self.period_ = std::move(period);
  }

private:
  TPeriodMethod period_;
};

} // namespace pludux
