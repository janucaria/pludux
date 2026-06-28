module;

#include <string>
#include <utility>

export module pludux.backtest:profile;

import pludux;

export namespace pludux::backtest {

class Profile {
public:
  Profile()
  : Profile{""}
  {
  }

  Profile(std::string name)
  : Profile{std::move(name), 0.0}
  {
  }

  Profile(std::string name, double capital_risk)
  : name_{std::move(name)}
  , capital_risk_{capital_risk}
  {
  }

  auto operator==(const Profile&) const noexcept -> bool = default;

  auto name(this const Profile& self) noexcept -> const std::string&
  {
    return self.name_;
  }

  void name(this Profile& self, std::string name) noexcept
  {
    self.name_ = std::move(name);
  }

  auto capital_risk(this const Profile& self) noexcept -> double
  {
    return self.capital_risk_;
  }

  void capital_risk(this Profile& self, double capital_risk) noexcept
  {
    self.capital_risk_ = capital_risk;
  }

  auto equivalent_rules(this const Profile& self, const Profile& other) noexcept
   -> bool
  {
    return self.capital_risk_ == other.capital_risk_;
  }

private:
  std::string name_;
  double capital_risk_;
};

} // namespace pludux::backtest
