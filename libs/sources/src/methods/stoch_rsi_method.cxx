module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:methods.stoch_rsi_method;

import :methods.ohlcv_method;

export namespace pludux {

template<typename TRsiSourceMethod = CloseMethod>
class StochRsiMethod {
public:
  StochRsiMethod()
  : StochRsiMethod{5, 3, 3}
  {
  }

  explicit StochRsiMethod(std::size_t k_period,
                          std::size_t k_smooth,
                          std::size_t d_period)
  : StochRsiMethod{TRsiSourceMethod{}, 14, k_period, k_smooth, d_period}
  {
  }

  StochRsiMethod(TRsiSourceMethod rsi_source,
                 std::size_t rsi_period,
                 std::size_t k_period,
                 std::size_t k_smooth,
                 std::size_t d_period)
  : rsi_source_{std::move(rsi_source)}
  , rsi_period_{rsi_period}
  , k_period_{k_period}
  , k_smooth_{k_smooth}
  , d_period_{d_period}
  {
  }

  auto operator==(const StochRsiMethod& other) const noexcept -> bool = default;

  auto rsi_source(this const StochRsiMethod& self) noexcept
   -> const TRsiSourceMethod&
  {
    return self.rsi_source_;
  }

  void rsi_source(this StochRsiMethod& self,
                  TRsiSourceMethod rsi_source) noexcept
  {
    self.rsi_source_ = rsi_source;
  }

  auto rsi_period(this const StochRsiMethod& self) noexcept -> std::size_t
  {
    return self.rsi_period_;
  }

  void rsi_period(this StochRsiMethod& self, std::size_t period) noexcept
  {
    self.rsi_period_ = period;
  }

  auto k_period(this const StochRsiMethod& self) noexcept -> std::size_t
  {
    return self.k_period_;
  }

  void k_period(this StochRsiMethod& self, std::size_t period) noexcept
  {
    self.k_period_ = period;
  }

  auto k_smooth(this const StochRsiMethod& self) noexcept -> std::size_t
  {
    return self.k_smooth_;
  }

  void k_smooth(this StochRsiMethod& self, std::size_t smooth) noexcept
  {
    self.k_smooth_ = smooth;
  }

  auto d_period(this const StochRsiMethod& self) noexcept -> std::size_t
  {
    return self.d_period_;
  }

  void d_period(this StochRsiMethod& self, std::size_t period) noexcept
  {
    self.d_period_ = period;
  }

private:
  TRsiSourceMethod rsi_source_;
  std::size_t rsi_period_;

  std::size_t k_period_;
  std::size_t k_smooth_;
  std::size_t d_period_;
};

} // namespace pludux
