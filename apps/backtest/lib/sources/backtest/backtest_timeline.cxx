module;

#include <algorithm>
#include <ctime>
#include <limits>
#include <numeric>
#include <optional>
#include <utility>
#include <vector>

export module pludux.backtest:backtest_timeline;

import :closed_trade;
import :open_position_snapshot;
import :trade_event;

export namespace pludux::backtest {

class BacktestTimeline {
public:
  BacktestTimeline() = default;

  struct Row {
    std::time_t market_timestamp{};
    double market_price{};
    std::size_t market_lookback{};
    std::vector<TradeEvent> trade_events{};
    std::vector<ClosedTrade> closed_trades{};
    std::optional<OpenPositionSnapshot> open_position{};

    double capital{};
    double equity{};
    double peak_equity{};
    double drawdown{};
    double max_drawdown{};

    std::time_t cumulative_duration{};
    double cumulative_investment{};

    std::size_t profit_count{};
    double cumulative_profit{};

    std::size_t loss_count{};
    double cumulative_loss{};

    std::size_t break_even_count{};

    std::size_t open_trade_count{};
    double unrealized_pnl{};
    double unrealized_investment{};
    std::time_t unrealized_duration{};
  };

  auto operator==(const BacktestTimeline&) const noexcept -> bool = default;

  auto size(this const BacktestTimeline& self) noexcept -> std::size_t
  {
    return self.capitals_.size();
  }

  auto empty(this const BacktestTimeline& self) noexcept -> bool
  {
    return self.capitals_.empty();
  }

  void clear(this BacktestTimeline& self) noexcept
  {
    self.market_timestamps_.clear();
    self.market_prices_.clear();
    self.market_lookbacks_.clear();
    self.trade_events_.clear();
    self.closed_trades_.clear();
    self.open_positions_.clear();
    self.capitals_.clear();
    self.equities_.clear();
    self.peak_equities_.clear();
    self.drawdowns_.clear();
    self.max_drawdowns_.clear();
    self.sum_of_durations_.clear();
    self.cumulative_investments_.clear();
    self.profit_counts_.clear();
    self.cumulative_profits_.clear();
    self.loss_counts_.clear();
    self.cumulative_losses_.clear();
    self.break_even_counts_.clear();
    self.open_trade_counts_.clear();
    self.unrealized_pnls_.clear();
    self.unrealized_investments_.clear();
    self.unrealized_durations_.clear();
    self.current_winning_streak_ = 0;
    self.current_losing_streak_ = 0;
    self.maximum_winning_streak_ = 0;
    self.maximum_losing_streak_ = 0;
  }

  void reserve(this BacktestTimeline& self, std::size_t size)
  {
    self.market_timestamps_.reserve(size);
    self.market_prices_.reserve(size);
    self.market_lookbacks_.reserve(size);
    self.trade_events_.reserve(size);
    self.closed_trades_.reserve(size);
    self.open_positions_.reserve(size);
    self.capitals_.reserve(size);
    self.equities_.reserve(size);
    self.peak_equities_.reserve(size);
    self.drawdowns_.reserve(size);
    self.max_drawdowns_.reserve(size);
    self.sum_of_durations_.reserve(size);
    self.cumulative_investments_.reserve(size);
    self.profit_counts_.reserve(size);
    self.cumulative_profits_.reserve(size);
    self.loss_counts_.reserve(size);
    self.cumulative_losses_.reserve(size);
    self.break_even_counts_.reserve(size);
    self.open_trade_counts_.reserve(size);
    self.unrealized_pnls_.reserve(size);
    self.unrealized_investments_.reserve(size);
    self.unrealized_durations_.reserve(size);
  }

  void append(this BacktestTimeline& self, Row row)
  {
    self.update_streaks(row.closed_trades);
    self.market_timestamps_.push_back(row.market_timestamp);
    self.market_prices_.push_back(row.market_price);
    self.market_lookbacks_.push_back(row.market_lookback);
    self.trade_events_.push_back(std::move(row.trade_events));
    self.closed_trades_.push_back(std::move(row.closed_trades));
    self.open_positions_.push_back(std::move(row.open_position));
    self.capitals_.push_back(row.capital);
    self.equities_.push_back(row.equity);
    self.peak_equities_.push_back(row.peak_equity);
    self.drawdowns_.push_back(row.drawdown);
    self.max_drawdowns_.push_back(row.max_drawdown);
    self.sum_of_durations_.push_back(row.cumulative_duration);
    self.cumulative_investments_.push_back(row.cumulative_investment);
    self.profit_counts_.push_back(row.profit_count);
    self.cumulative_profits_.push_back(row.cumulative_profit);
    self.loss_counts_.push_back(row.loss_count);
    self.cumulative_losses_.push_back(row.cumulative_loss);
    self.break_even_counts_.push_back(row.break_even_count);
    self.open_trade_counts_.push_back(row.open_trade_count);
    self.unrealized_pnls_.push_back(row.unrealized_pnl);
    self.unrealized_investments_.push_back(row.unrealized_investment);
    self.unrealized_durations_.push_back(row.unrealized_duration);
  }

  auto market_timestamp(this const BacktestTimeline& self,
                        std::size_t index) noexcept -> std::time_t
  {
    return self.market_timestamps_[index];
  }

  auto market_price(this const BacktestTimeline& self,
                    std::size_t index) noexcept -> double
  {
    return self.market_prices_[index];
  }

  auto market_lookback(this const BacktestTimeline& self,
                       std::size_t index) noexcept -> std::size_t
  {
    return self.market_lookbacks_[index];
  }

  auto trade_events(this const BacktestTimeline& self,
                    std::size_t index) noexcept
   -> const std::vector<TradeEvent>&
  {
    return self.trade_events_[index];
  }

  auto closed_trades(this const BacktestTimeline& self,
                     std::size_t index) noexcept
   -> const std::vector<ClosedTrade>&
  {
    return self.closed_trades_[index];
  }

  auto open_position(this const BacktestTimeline& self,
                     std::size_t index) noexcept
   -> const std::optional<OpenPositionSnapshot>&
  {
    return self.open_positions_[index];
  }

  auto capital(this const BacktestTimeline& self, std::size_t index) noexcept
   -> double
  {
    return self.capitals_[index];
  }

  auto peak_equity(this const BacktestTimeline& self,
                   std::size_t index) noexcept -> double
  {
    return self.peak_equities_[index];
  }

  auto max_drawdown(this const BacktestTimeline& self,
                    std::size_t index) noexcept -> double
  {
    return self.max_drawdowns_[index];
  }

  auto cumulative_investments(this const BacktestTimeline& self,
                              std::size_t index) noexcept -> double
  {
    return self.cumulative_investments_[index];
  }

  auto average_investment(this const BacktestTimeline& self,
                          std::size_t index) noexcept -> double
  {
    return self.trade_count(index)
            ? self.cumulative_investments(index) / self.trade_count(index)
            : 0.0;
  }

  auto trade_count(this const BacktestTimeline& self,
                   std::size_t index) noexcept -> std::size_t
  {
    return self.profit_count(index) + self.loss_count(index) +
           self.break_even_count(index);
  }

  auto open_trade_count(this const BacktestTimeline& self,
                        std::size_t index) noexcept -> std::size_t
  {
    return self.open_trade_counts_[index];
  }

  auto cumulative_pnls(this const BacktestTimeline& self,
                       std::size_t index) noexcept -> double
  {
    return self.cumulative_profits(index) + self.cumulative_losses(index);
  }

  auto cumulative_durations(this const BacktestTimeline& self,
                            std::size_t index) noexcept -> std::time_t
  {
    return self.sum_of_durations_[index];
  }

  auto average_duration(this const BacktestTimeline& self,
                        std::size_t index) noexcept -> std::time_t
  {
    return self.trade_count(index)
            ? self.cumulative_durations(index) / self.trade_count(index)
            : 0;
  }

  auto average_pnl(this const BacktestTimeline& self,
                   std::size_t index) noexcept -> double
  {
    return self.trade_count(index)
            ? self.cumulative_pnls(index) / self.trade_count(index)
            : 0.0;
  }

  auto expected_value(this const BacktestTimeline& self,
                      std::size_t index) noexcept -> double
  {
    return self.profit_rate(index) * self.average_profit(index) +
           self.loss_rate(index) * self.average_loss(index);
  }

  auto expected_return(this const BacktestTimeline& self,
                       std::size_t index) noexcept -> double
  {
    return self.average_investment(index)
            ? self.expected_value(index) / self.average_investment(index) * 100
            : 0.0;
  }

  auto profit_count(this const BacktestTimeline& self,
                    std::size_t index) noexcept -> std::size_t
  {
    return self.profit_counts_[index];
  }

  auto profit_rate(this const BacktestTimeline& self,
                   std::size_t index) noexcept -> double
  {
    return self.trade_count(index)
            ? static_cast<double>(self.profit_count(index)) /
               self.trade_count(index)
            : 0.0;
  }

  auto cumulative_profits(this const BacktestTimeline& self,
                          std::size_t index) noexcept -> double
  {
    return self.cumulative_profits_[index];
  }

  auto cumulative_profit_percent(this const BacktestTimeline& self,
                                 std::size_t index) noexcept -> double
  {
    return self.cumulative_investments(index)
            ? self.cumulative_profits(index) /
               self.cumulative_investments(index) * 100.0
            : 0.0;
  }

  auto average_profit(this const BacktestTimeline& self,
                      std::size_t index) noexcept -> double
  {
    return self.profit_count(index)
            ? self.cumulative_profits(index) / self.profit_count(index)
            : 0.0;
  }

  auto loss_count(this const BacktestTimeline& self, std::size_t index) noexcept
   -> std::size_t
  {
    return self.loss_counts_[index];
  }

  auto loss_rate(this const BacktestTimeline& self, std::size_t index) noexcept
   -> double
  {
    return self.trade_count(index)
            ? static_cast<double>(self.loss_count(index)) /
               self.trade_count(index)
            : 0.0;
  }

  auto cumulative_losses(this const BacktestTimeline& self,
                         std::size_t index) noexcept -> double
  {
    return self.cumulative_losses_[index];
  }

  auto cumulative_loss_percent(this const BacktestTimeline& self,
                               std::size_t index) noexcept -> double
  {
    return self.cumulative_investments(index)
            ? self.cumulative_losses(index) /
               self.cumulative_investments(index) * 100.0
            : 0.0;
  }

  auto average_loss(this const BacktestTimeline& self,
                    std::size_t index) noexcept -> double
  {
    return self.loss_count(index)
            ? self.cumulative_losses(index) / self.loss_count(index)
            : 0.0;
  }

  auto break_even_count(this const BacktestTimeline& self,
                        std::size_t index) noexcept -> std::size_t
  {
    return self.break_even_counts_[index];
  }

  auto break_even_rate(this const BacktestTimeline& self,
                       std::size_t index) noexcept -> double
  {
    return self.trade_count(index)
            ? static_cast<double>(self.break_even_count(index)) /
               self.trade_count(index)
            : 0.0;
  }

  auto current_winning_streak(this const BacktestTimeline& self) noexcept
   -> std::size_t
  {
    return self.current_winning_streak_;
  }

  auto current_losing_streak(this const BacktestTimeline& self) noexcept
   -> std::size_t
  {
    return self.current_losing_streak_;
  }

  auto maximum_winning_streak(this const BacktestTimeline& self) noexcept
   -> std::size_t
  {
    return self.maximum_winning_streak_;
  }

  auto maximum_losing_streak(this const BacktestTimeline& self) noexcept
   -> std::size_t
  {
    return self.maximum_losing_streak_;
  }

  auto profit_factor(this const BacktestTimeline& self,
                     std::size_t index) noexcept -> double
  {
    if(self.cumulative_losses(index) == 0.0) {
      return std::numeric_limits<double>::infinity();
    }
    return self.cumulative_profits(index) / -self.cumulative_losses(index);
  }

  auto unrealized_pnl(this const BacktestTimeline& self,
                      std::size_t index) noexcept -> double
  {
    return self.unrealized_pnls_[index];
  }

  auto unrealized_investment(this const BacktestTimeline& self,
                             std::size_t index) noexcept -> double
  {
    return self.unrealized_investments_[index];
  }

  auto unrealized_duration(this const BacktestTimeline& self,
                           std::size_t index) noexcept -> std::time_t
  {
    return self.unrealized_durations_[index];
  }

  auto equity(this const BacktestTimeline& self, std::size_t index) noexcept
   -> double
  {
    return self.equities_[index];
  }

  auto initial_capital(this const BacktestTimeline& self,
                       std::size_t index) noexcept -> double
  {
    return self.capital(index) - self.cumulative_pnls(index);
  }

  auto drawdown(this const BacktestTimeline& self, std::size_t index) noexcept
   -> double
  {
    return self.drawdowns_[index];
  }

private:
  void update_streaks(this BacktestTimeline& self,
                      const std::vector<ClosedTrade>& closed_trades) noexcept
  {
    for(const auto& trade : closed_trades) {
      const auto pnl = trade.pnl();
      if(pnl > 0.0) {
        ++self.current_winning_streak_;
        self.current_losing_streak_ = 0;
        self.maximum_winning_streak_ =
         std::max(self.maximum_winning_streak_, self.current_winning_streak_);
      } else if(pnl < 0.0) {
        self.current_winning_streak_ = 0;
        ++self.current_losing_streak_;
        self.maximum_losing_streak_ =
         std::max(self.maximum_losing_streak_, self.current_losing_streak_);
      } else {
        self.current_winning_streak_ = 0;
        self.current_losing_streak_ = 0;
      }
    }
  }

  std::vector<std::time_t> market_timestamps_;
  std::vector<double> market_prices_;
  std::vector<std::size_t> market_lookbacks_;
  std::vector<std::vector<TradeEvent>> trade_events_;
  std::vector<std::vector<ClosedTrade>> closed_trades_;
  std::vector<std::optional<OpenPositionSnapshot>> open_positions_;

  std::vector<double> capitals_;
  std::vector<double> equities_;
  std::vector<double> peak_equities_;
  std::vector<double> drawdowns_;
  std::vector<double> max_drawdowns_;

  std::vector<std::time_t> sum_of_durations_;
  std::vector<double> cumulative_investments_;

  std::vector<std::size_t> profit_counts_;
  std::vector<double> cumulative_profits_;

  std::vector<std::size_t> loss_counts_;
  std::vector<double> cumulative_losses_;

  std::vector<std::size_t> break_even_counts_;

  std::vector<std::size_t> open_trade_counts_;
  std::vector<double> unrealized_pnls_;
  std::vector<double> unrealized_investments_;
  std::vector<std::time_t> unrealized_durations_;

  std::size_t current_winning_streak_{};
  std::size_t current_losing_streak_{};
  std::size_t maximum_winning_streak_{};
  std::size_t maximum_losing_streak_{};
};

} // namespace pludux::backtest
