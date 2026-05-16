module;

#include <cassert>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:series.atr_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

import :series.ma_method_type;

export namespace pludux {

class AtrMethod {
public:
  AtrMethod()
  : AtrMethod{14}
  {
  }

  explicit AtrMethod(std::size_t period,
                     MaMethodType ma_smoothing_type = MaMethodType::Rma)
  : period_{period}
  , ma_smoothing_type_{ma_smoothing_type}
  {
  }

  auto operator==(const AtrMethod& other) const noexcept -> bool = default;

  auto period(this AtrMethod self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this AtrMethod& self, std::size_t new_period) noexcept
  {
    self.period_ = new_period;
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
  std::size_t period_;
  MaMethodType ma_smoothing_type_;
};

} // namespace pludux
