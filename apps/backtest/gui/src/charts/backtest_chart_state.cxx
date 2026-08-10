module;

#include <algorithm>
#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

export module pludux.apps.backtest:charts.backtest_chart_state;

import pludux.backtest;
import pludux.apps.backtest.portfolio_backtest_setup_selections;

export namespace pludux::apps {

enum class BacktestChartViewportMode {
  FitAll,
  FollowLatest,
};

enum class BacktestTopPlot {
  Equity,
  Drawdown,
  ShadowReturn,
  FrequentistPerformance,
  CurrentStreaks,
  BayesianWin,
  BayesianPayoff,
};

class BacktestChartState {
public:
  void display_backtest_setup(this BacktestChartState& self,
                              backtest::PortfolioStoreHandle portfolio_handle,
                              PortfolioBacktestSetupKey setup) noexcept
  {
    if(self.displayed_portfolio_ == portfolio_handle &&
       self.displayed_setup_ == setup) {
      return;
    }

    self.displayed_portfolio_ = portfolio_handle;
    self.displayed_setup_ = setup;
    self.pinned_bar_.reset();
    self.viewport_mode_ = BacktestChartViewportMode::FitAll;
    self.follow_bar_count_ = 150;
    self.fit_requested_ = true;
  }

  void clear_display_backtest_setup(this BacktestChartState& self) noexcept
  {
    self.displayed_portfolio_.reset();
    self.displayed_setup_.reset();
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

  auto top_plot(this const BacktestChartState& self) noexcept -> BacktestTopPlot
  {
    return self.top_plot_;
  }

  void top_plot(this BacktestChartState& self, BacktestTopPlot plot) noexcept
  {
    if(self.top_plot_ == plot) {
      return;
    }

    self.top_plot_ = plot;
    self.fit_requested_ = true;
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

  auto row_ratios(this BacktestChartState& self, std::size_t row_count)
   -> std::vector<float>&
  {
    if(self.row_ratios_.size() != row_count) {
      self.row_ratios_.assign(row_count, 1.0f);
      if(row_count > 1) {
        self.row_ratios_[1] = 4.0f;
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
  std::optional<backtest::PortfolioStoreHandle> displayed_portfolio_{};
  std::optional<PortfolioBacktestSetupKey> displayed_setup_{};
  std::optional<std::size_t> pinned_bar_{};
  std::vector<float> row_ratios_{};
  BacktestChartViewportMode viewport_mode_{BacktestChartViewportMode::FitAll};
  BacktestTopPlot top_plot_{BacktestTopPlot::Equity};
  std::size_t follow_bar_count_{150};
  bool fit_requested_{true};
  bool show_volume_{true};
  bool show_trades_{true};
  bool show_risk_{true};
  bool show_indicators_{true};
};

} // namespace pludux::apps
