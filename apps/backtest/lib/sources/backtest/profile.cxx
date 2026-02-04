module;

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

export module pludux.backtest:profile;

export namespace pludux::backtest {

class Profile {
public:
  Profile()
  : Profile{""}
  {
  }

  Profile(std::string name)
  : Profile{std::move(name), 0.0, 0.0}
  {
  }

  Profile(std::string name, double initial_capital, double capital_risk)
  : name_{std::move(name)}
  , initial_capital_{initial_capital}
  , capital_risk_{capital_risk}
  {
  }

  auto name(this const Profile& self) noexcept -> const std::string&
  {
    return self.name_;
  }

  void name(this Profile& self, std::string name) noexcept
  {
    self.name_ = std::move(name);
  }

  auto initial_capital(this const Profile& self) noexcept -> double
  {
    return self.initial_capital_;
  }

  void initial_capital(this Profile& self, double initial_capital) noexcept
  {
    self.initial_capital_ = initial_capital;
  }

  auto capital_risk(this const Profile& self) noexcept -> double
  {
    return self.capital_risk_;
  }

  void capital_risk(this Profile& self, double capital_risk) noexcept
  {
    self.capital_risk_ = capital_risk;
  }

  auto get_risk_value(this const Profile& self) noexcept -> double
  {
    return self.initial_capital_ * self.capital_risk_;
  }

private:
  std::string name_;
  double initial_capital_;
  double capital_risk_;
};

} // namespace pludux::backtest
