module;

#include <algorithm>
#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

export module pludux.apps.backtest:charts.backtest_chart_state;

import pludux.backtest;

export namespace pludux::apps {

enum class BacktestChartViewportMode {
  FitAll,
  FollowLatest,
};

class BacktestChartState {
public:
  void select_backtest(
   this BacktestChartState& self,
   const backtest::BacktestStoreHandle& selected_backtest) noexcept
  {
    if(self.selected_backtest_ &&
       *self.selected_backtest_ == selected_backtest) {
      return;
    }

    self.selected_backtest_ = selected_backtest;
    self.pinned_bar_.reset();
    self.viewport_mode_ = BacktestChartViewportMode::FitAll;
    self.follow_bar_count_ = 150;
    self.fit_requested_ = true;
  }

  void clear_selection(this BacktestChartState& self) noexcept
  {
    self.selected_backtest_.reset();
    self.pinned_bar_.reset();
    self.viewport_mode_ = BacktestChartViewportMode::FitAll;
    self.follow_bar_count_ = 150;
    self.fit_requested_ = true;
  }

  auto pinned_bar(this const BacktestChartState& self) noexcept
   -> std::optional<std::size_t>
  {
    return self.pinned_bar_;
  }

  void pin_bar(this BacktestChartState& self,
               std::optional<std::size_t> bar) noexcept
  {
    self.pinned_bar_ = bar;
  }

  auto viewport_mode(this const BacktestChartState& self) noexcept
   -> BacktestChartViewportMode
  {
    return self.viewport_mode_;
  }

  void viewport_mode(this BacktestChartState& self,
                     BacktestChartViewportMode mode) noexcept
  {
    self.viewport_mode_ = mode;
  }

  auto follow_bar_count(this const BacktestChartState& self) noexcept
   -> std::size_t
  {
    return self.follow_bar_count_;
  }

  void follow_bar_count(this BacktestChartState& self,
                        std::size_t count) noexcept
  {
    self.follow_bar_count_ = std::clamp<std::size_t>(count, 10, 100'000);
  }

  void request_fit(this BacktestChartState& self) noexcept
  {
    self.fit_requested_ = true;
  }

  auto consume_fit_request(this BacktestChartState& self) noexcept -> bool
  {
    return std::exchange(self.fit_requested_, false);
  }

  auto show_equity(this const BacktestChartState& self) noexcept -> bool
  {
    return self.show_equity_;
  }

  auto show_volume(this const BacktestChartState& self) noexcept -> bool
  {
    return self.show_volume_;
  }

  auto show_trades(this const BacktestChartState& self) noexcept -> bool
  {
    return self.show_trades_;
  }

  auto show_risk(this const BacktestChartState& self) noexcept -> bool
  {
    return self.show_risk_;
  }

  auto show_indicators(this const BacktestChartState& self) noexcept -> bool
  {
    return self.show_indicators_;
  }

  auto show_strategy_performance(this const BacktestChartState& self) noexcept
   -> bool
  {
    return self.show_strategy_performance_;
  }

  auto show_equity(this BacktestChartState& self) noexcept -> bool&
  {
    return self.show_equity_;
  }

  auto show_volume(this BacktestChartState& self) noexcept -> bool&
  {
    return self.show_volume_;
  }

  auto show_trades(this BacktestChartState& self) noexcept -> bool&
  {
    return self.show_trades_;
  }

  auto show_risk(this BacktestChartState& self) noexcept -> bool&
  {
    return self.show_risk_;
  }

  auto show_indicators(this BacktestChartState& self) noexcept -> bool&
  {
    return self.show_indicators_;
  }

  auto show_strategy_performance(this BacktestChartState& self) noexcept
   -> bool&
  {
    return self.show_strategy_performance_;
  }

  auto row_ratios(this BacktestChartState& self, std::size_t row_count)
   -> std::vector<float>&
  {
    if(self.row_ratios_.size() != row_count) {
      self.row_ratios_.assign(row_count, 1.0f);
      auto row = std::size_t{0};
      if(self.show_equity_ && row < row_count) {
        self.row_ratios_[row++] = 1.0f;
      }
      if(row < row_count) {
        self.row_ratios_[row] = 4.0f;
      }
      self.fit_requested_ = true;
    }

    return self.row_ratios_;
  }

  void reset_row_ratios(this BacktestChartState& self) noexcept
  {
    self.row_ratios_.clear();
    self.fit_requested_ = true;
  }

private:
  std::optional<backtest::BacktestStoreHandle> selected_backtest_{};
  std::optional<std::size_t> pinned_bar_{};
  std::vector<float> row_ratios_{};
  BacktestChartViewportMode viewport_mode_{BacktestChartViewportMode::FitAll};
  std::size_t follow_bar_count_{150};
  bool fit_requested_{true};
  bool show_equity_{true};
  bool show_volume_{true};
  bool show_trades_{true};
  bool show_risk_{true};
  bool show_indicators_{true};
  bool show_strategy_performance_{false};
};

} // namespace pludux::apps
