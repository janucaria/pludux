module;

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>

export module pludux.backtest:profile;

import pludux;

export namespace pludux::backtest {

class Profile {
public:
  enum class RDistance : int { Atr, Percentage, Price };

  Profile()
  : Profile{""}
  {
  }

  Profile(std::string name)
  : Profile{std::move(name), 0.0, 0.0}
  {
  }

  Profile(std::string name,
          double initial_capital,
          double capital_risk,
          RDistance r_distance_mode = RDistance::Atr,
          std::pair<std::size_t, double> r_mode_atr = {14, 2.0},
          double r_mode_percentage = 10.0,
          double r_mode_price = 1000.0)
  : name_{std::move(name)}
  , initial_capital_{initial_capital}
  , capital_risk_{capital_risk}
  , r_distance_mode_{r_distance_mode}
  , r_mode_atr_{r_mode_atr}
  , r_mode_percentage_{r_mode_percentage}
  , r_mode_price_{r_mode_price}
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

  auto r_distance_mode(this const Profile& self) noexcept -> RDistance
  {
    return self.r_distance_mode_;
  }

  void r_distance_mode(this Profile& self, RDistance r_distance_mode) noexcept
  {
    self.r_distance_mode_ = r_distance_mode;
  }

  auto r_mode_atr(this const Profile& self) noexcept
   -> const std::pair<std::size_t, double>&
  {
    return self.r_mode_atr_;
  }

  void r_mode_atr(this Profile& self,
                  std::pair<std::size_t, double> r_mode_atr) noexcept
  {
    self.r_mode_atr_ = std::move(r_mode_atr);
  }

  auto r_mode_percentage(this const Profile& self) noexcept -> double
  {
    return self.r_mode_percentage_;
  }

  void r_mode_percentage(this Profile& self, double r_mode_percentage) noexcept
  {
    self.r_mode_percentage_ = r_mode_percentage;
  }

  auto r_mode_price(this const Profile& self) noexcept -> double
  {
    return self.r_mode_price_;
  }

  void r_mode_price(this Profile& self, double r_mode_price) noexcept
  {
    self.r_mode_price_ = r_mode_price;
  }

  auto get_risk_value(this const Profile& self) noexcept -> double
  {
    return self.initial_capital_ * self.capital_risk_;
  }

  auto get_r_distance(this const Profile& self,
                      double entry_price,
                      const AssetSnapshot& prev_snapshot,
                      MethodContextable auto context) noexcept -> double
  {
    switch(self.r_distance_mode_) {
    case RDistance::Atr:
      return self.r_mode_atr_.second *
             AtrMethod{static_cast<std::size_t>(self.r_mode_atr_.first)}(
              prev_snapshot, context);
    case RDistance::Percentage:
      return entry_price * (self.r_mode_percentage_ / 100.0);
    case RDistance::Price:
      return self.r_mode_price_;
    }

    return std::numeric_limits<double>::quiet_NaN();
  }

  auto equal_rules(this const Profile& self, const Profile& other) noexcept
   -> bool
  {
    if(self.initial_capital_ == other.initial_capital_ &&
       self.capital_risk_ == other.capital_risk_ &&
       self.r_distance_mode_ == other.r_distance_mode_) {
      switch(self.r_distance_mode_) {
      case RDistance::Atr:
        return self.r_mode_atr_ == other.r_mode_atr_;
      case RDistance::Percentage:
        return self.r_mode_percentage_ == other.r_mode_percentage_;
      case RDistance::Price:
        return self.r_mode_price_ == other.r_mode_price_;
      }
    }

    return false;
  }

private:
  std::string name_;
  double initial_capital_;
  double capital_risk_;

  RDistance r_distance_mode_;

  std::pair<std::size_t, double> r_mode_atr_;
  double r_mode_percentage_;
  double r_mode_price_;
};

} // namespace pludux::backtest
