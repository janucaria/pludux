module;

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

export module pludux.backtest:profile;

export namespace pludux::backtest {

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

// ------------------------------------------------------------------------

Profile::Profile(std::string name)
: name_{std::move(name)}
, initial_capital_{0.0}
, capital_risk_{0.0}
{
}

auto Profile::name() const noexcept -> const std::string&
{
  return name_;
}

void Profile::name(std::string name) noexcept
{
  name_ = std::move(name);
}

auto Profile::initial_capital() const noexcept -> double
{
  return initial_capital_;
}

void Profile::initial_capital(double initial_capital) noexcept
{
  initial_capital_ = initial_capital;
}

auto Profile::capital_risk() const noexcept -> double
{
  return capital_risk_;
}

void Profile::capital_risk(double capital_risk) noexcept
{
  capital_risk_ = capital_risk;
}

auto Profile::get_risk_value() const noexcept -> double
{
  return initial_capital_ * capital_risk_;
}

} // namespace pludux::backtest
