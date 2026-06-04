module;

#include <cstddef>

export module pludux:methods.donchian_channel_method;

export namespace pludux {

class DonchianChannelMethod {
public:
  DonchianChannelMethod()
  : DonchianChannelMethod{20}
  {
  }

  DonchianChannelMethod(std::size_t period)
  : period_{period}
  {
  }

  auto operator==(const DonchianChannelMethod&) const noexcept
   -> bool = default;

  auto period(this const DonchianChannelMethod& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this DonchianChannelMethod& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

private:
  std::size_t period_;
};

} // namespace pludux