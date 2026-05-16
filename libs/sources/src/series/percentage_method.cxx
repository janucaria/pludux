module;

#include <limits>
#include <utility>

export module pludux:series.percentage_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

import :series.ohlcv_method;

export namespace pludux {

template<typename TMethod = CloseMethod>
class PercentageMethod {
public:
  PercentageMethod()
  : PercentageMethod{TMethod{}, 100.0}
  {
  }

  explicit PercentageMethod(double percent)
  : PercentageMethod{TMethod{}, percent}
  {
  }

  PercentageMethod(TMethod base, double percent)
  : base_{std::move(base)}
  , percent_{percent}
  {
  }

  auto operator==(const PercentageMethod& other) const noexcept
   -> bool = default;

  auto base(this const PercentageMethod& self) noexcept -> const TMethod&
  {
    return self.base_;
  }

  void base(this PercentageMethod& self, TMethod new_base) noexcept
  {
    self.base_ = std::move(new_base);
  }

  auto percent(this const PercentageMethod& self) noexcept -> double
  {
    return self.percent_;
  }

  void percent(this PercentageMethod& self, double new_percent) noexcept
  {
    self.percent_ = new_percent;
  }

private:
  TMethod base_;
  double percent_;
};

} // namespace pludux