module;

#include <ctime>
#include <numeric>
#include <optional>
#include <utility>
#include <vector>

export module pludux.backtest:backtest_summary;

import :trade_session;

export namespace pludux::backtest {

class BacktestSummary {
public:
  BacktestSummary()
  : BacktestSummary{0.0, TradeSession{}}
  {
  }

  BacktestSummary(double initial_capital,
                  TradeSession trade_session = TradeSession{})
  : trade_session_{std::move(trade_session)}
  , capital_{initial_capital}
  , peak_equity_{initial_capital}
  , max_drawdown_{0.0}
  , sum_of_durations_{}
  , cumulative_investments_{0.0}
  , profit_count_{0}
  , cumulative_profits_{0.0}
  , loss_count_{0}
  , cumulative_losses_{0.0}
  , break_even_count_{0}
  {
  }

  void update_to_next_summary(this auto& self,
                              TradeSession next_trade_session) noexcept
  {
    if(next_trade_session.closed_position()) {
      const auto pnl = next_trade_session.realized_pnl();

      self.sum_of_durations_ += next_trade_session.realized_duration();
      self.cumulative_investments_ += next_trade_session.realized_investment();

      if(pnl > 0) {
        self.profit_count_++;
        self.cumulative_profits_ += pnl;
      } else if(pnl < 0) {
        self.loss_count_++;
        self.cumulative_losses_ += pnl;
      } else {
        self.break_even_count_++;
      }

      self.capital_ += pnl;
    }

    self.trade_session_ = std::move(next_trade_session);
    self.peak_equity_ = std::max(self.peak_equity_, self.equity());
    self.max_drawdown_ = std::max(self.max_drawdown_, self.drawdown());
  }

  auto trade_session(this const auto& self) noexcept -> const TradeSession&
  {
    return self.trade_session_;
  }

  void trade_session(this auto& self, TradeSession trade_session) noexcept
  {
    self.trade_session_ = std::move(trade_session);
  }

  auto capital(this const auto& self) noexcept -> double
  {
    return self.capital_;
  }

  void capital(this auto& self, double capital) noexcept
  {
    self.capital_ = capital;
  }

  auto peak_equity(this const auto& self) noexcept -> double
  {
    return self.peak_equity_;
  }

  void peak_equity(this auto& self, double peak_equity) noexcept
  {
    self.peak_equity_ = peak_equity;
  }

  auto max_drawdown(this const auto& self) noexcept -> double
  {
    return self.max_drawdown_;
  }

  void max_drawdown(this auto& self, double max_drawdown) noexcept
  {
    self.max_drawdown_ = max_drawdown;
  }

  auto cumulative_investments(this const auto& self) noexcept -> double
  {
    return self.cumulative_investments_;
  }

  void cumulative_investments(this auto& self, double investment) noexcept
  {
    self.cumulative_investments_ = investment;
  }

  auto average_investment(this const auto& self) noexcept -> double
  {
    return self.trade_count()
            ? self.cumulative_investments() / self.trade_count()
            : 0.0;
  }

  auto trade_count(this const auto& self) noexcept -> std::size_t
  {
    return self.profit_count() + self.loss_count() + self.break_even_count();
  }

  auto open_trade_count(this const auto& self) noexcept -> std::size_t
  {
    return self.trade_session_.is_open() ? 1 : 0;
  }

  auto cumulative_pnls(this const auto& self) noexcept -> double
  {
    return self.cumulative_profits() + self.cumulative_losses();
  }

  auto cumulative_durations(this const auto& self) noexcept -> std::time_t
  {
    return self.sum_of_durations_;
  }

  void cumulative_durations(this auto& self, std::time_t duration) noexcept
  {
    self.sum_of_durations_ = duration;
  }

  auto average_duration(this const auto& self) noexcept -> std::time_t
  {
    return self.trade_count() ? self.cumulative_durations() / self.trade_count()
                              : 0;
  }

  auto average_pnl(this const auto& self) noexcept -> double
  {
    return self.trade_count() ? self.cumulative_pnls() / self.trade_count()
                              : 0.0;
  }

  auto expected_value(this const auto& self) noexcept -> double
  {
    return self.profit_rate() * self.average_profit() +
           self.loss_rate() * self.average_loss();
  }

  auto expected_return(this const auto& self) noexcept -> double
  {
    return self.average_investment()
            ? self.expected_value() / self.average_investment() * 100
            : 0.0;
  }

  auto profit_count(this const auto& self) noexcept -> std::size_t
  {
    return self.profit_count_;
  }

  void profit_count(this auto& self, std::size_t count) noexcept
  {
    self.profit_count_ = count;
  }

  auto profit_rate(this const auto& self) noexcept -> double
  {
    return self.trade_count()
            ? static_cast<double>(self.profit_count()) / self.trade_count()
            : 0.0;
  }

  auto cumulative_profits(this const auto& self) noexcept -> double
  {
    return self.cumulative_profits_;
  }

  void cumulative_profits(this auto& self, double profits) noexcept
  {
    self.cumulative_profits_ = profits;
  }

  auto cumulative_profit_percent(this const auto& self) noexcept -> double
  {
    return self.cumulative_investments()
            ? self.cumulative_profits() / self.cumulative_investments() * 100.0
            : 0.0;
  }

  auto average_profit(this const auto& self) noexcept -> double
  {
    return self.profit_count() ? self.cumulative_profits() / self.profit_count()
                               : 0.0;
  }

  auto loss_count(this const auto& self) noexcept -> std::size_t
  {
    return self.loss_count_;
  }

  void loss_count(this auto& self, std::size_t count) noexcept
  {
    self.loss_count_ = count;
  }

  auto loss_rate(this const auto& self) noexcept -> double
  {
    return self.trade_count()
            ? static_cast<double>(self.loss_count()) / self.trade_count()
            : 0.0;
  }

  auto cumulative_losses(this const auto& self) noexcept -> double
  {
    return self.cumulative_losses_;
  }

  void cumulative_losses(this auto& self, double losses) noexcept
  {
    self.cumulative_losses_ = losses;
  }

  auto cumulative_loss_percent(this const auto& self) noexcept -> double
  {
    return self.cumulative_investments()
            ? self.cumulative_losses() / self.cumulative_investments() * 100.0
            : 0.0;
  }

  auto average_loss(this const auto& self) noexcept -> double
  {
    return self.loss_count() ? self.cumulative_losses() / self.loss_count()
                             : 0.0;
  }

  auto break_even_count(this const auto& self) noexcept -> std::size_t
  {
    return self.break_even_count_;
  }

  void break_even_count(this auto& self, std::size_t count) noexcept
  {
    self.break_even_count_ = count;
  }

  auto break_even_rate(this const auto& self) noexcept -> double
  {
    return self.trade_count()
            ? static_cast<double>(self.break_even_count()) / self.trade_count()
            : 0.0;
  }

  auto profit_factor(this const auto& self) noexcept -> double
  {
    if(self.cumulative_losses() == 0.0) {
      return std::numeric_limits<double>::infinity();
    }
    return self.cumulative_profits() / -self.cumulative_losses();
  }

  auto unrealized_pnl(this const auto& self) noexcept -> double
  {
    return self.trade_session_.unrealized_pnl();
  }

  auto unrealized_investment(this const auto& self) noexcept -> double
  {
    return self.trade_session_.unrealized_investment();
  }

  auto unrealized_duration(this const auto& self) noexcept -> std::time_t
  {
    return self.trade_session_.unrealized_duration();
  }

  auto equity(this const auto& self) noexcept -> double
  {
    return self.capital() + self.trade_session_.partial_realized_pnl() +
           self.trade_session_.unrealized_pnl();
  }

  auto initial_capital(this const auto& self) noexcept -> double
  {
    return self.capital_ - self.cumulative_pnls();
  }

  auto drawdown(this const auto& self) noexcept -> double
  {
    return self.peak_equity_
            ? (self.peak_equity_ - self.equity()) / self.peak_equity_ * 100.0
            : 0.0;
  }

private:
  TradeSession trade_session_;

  double capital_;
  double peak_equity_;
  double max_drawdown_;

  std::time_t sum_of_durations_;
  double cumulative_investments_;

  std::size_t profit_count_;
  double cumulative_profits_;

  std::size_t loss_count_;
  double cumulative_losses_;

  std::size_t break_even_count_;
};

} // namespace pludux::backtest
