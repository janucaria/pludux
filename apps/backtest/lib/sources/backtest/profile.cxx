module;

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

export module pludux.backtest:profile;

export namespace pludux::backtest {

class Profile {
public:
  Profile(std::string name)
  : name_{std::move(name)}
  , initial_capital_{0.0}
  , capital_risk_{0.0}
  {
  }

  auto name(this const auto& self) noexcept -> const std::string&
  {
    return self.name_;
  }

  void name(this auto& self, std::string name) noexcept
  {
    self.name_ = std::move(name);
  }

  auto initial_capital(this const auto& self) noexcept -> double
  {
    return self.initial_capital_;
  }

  void initial_capital(this auto& self, double initial_capital) noexcept
  {
    self.initial_capital_ = initial_capital;
  }

  auto capital_risk(this const auto& self) noexcept -> double
  {
    return self.capital_risk_;
  }

  void capital_risk(this auto& self, double capital_risk) noexcept
  {
    self.capital_risk_ = capital_risk;
  }

  auto get_risk_value(this const auto& self) noexcept -> double
  {
    return self.initial_capital_ * self.capital_risk_;
  }

private:
  std::string name_;
  double initial_capital_;
  double capital_risk_;
};

} // namespace pludux::backtest
