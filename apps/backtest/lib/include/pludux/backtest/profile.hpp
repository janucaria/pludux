#ifndef PLUDUX_PLUDUX_BACKTEST_PROFILE_HPP
#define PLUDUX_PLUDUX_BACKTEST_PROFILE_HPP

#include <cstddef>
#include <cstdint>
#include <string>

namespace pludux::backtest {

class Profile {
public:
  Profile(std::string name);

  auto name() const noexcept -> const std::string&;

  void name(std::string name) noexcept;

  auto initial_capital() const noexcept -> double;

  void initial_capital(double initial_capital) noexcept;

  auto capital_risk() const noexcept -> double;

  void capital_risk(double capital_risk) noexcept;

  auto get_risk_value() const noexcept -> double;

private:
  std::string name_;
  double initial_capital_;
  double capital_risk_;
};

} // namespace pludux::backtest

#endif