module;

#include <algorithm>
#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

export module pludux.apps.backtest:charts.backtest_chart_state;

import pludux.backtest;
import pludux.apps.backtest.portfolio_strategy_selections;

export namespace pludux::apps {

enum class BacktestChartViewportMode {
  FitAll,
  FollowLatest,
};

enum class BacktestTopPlot {
  Equity,
  Drawdown,
  NetPnl,
  PnlDrawdown,
  ShadowReturn,
  FrequentistPerformance,
  CurrentStreaks,
  BayesianWin,
  BayesianPayoff,
};

constexpr auto is_drawdown_plot(BacktestTopPlot plot) noexcept -> bool
{
  return plot == BacktestTopPlot::Drawdown ||
         plot == BacktestTopPlot::PnlDrawdown;
}

class BacktestChartState {
public:
  void display(this BacktestChartState& self,
               backtest::PortfolioStoreHandle portfolio,
               std::optional<PortfolioStrategyKey> strategy) noexcept
  {
    if(self.displayed_portfolio_ == portfolio &&
       self.displayed_strategy_ == strategy) {
      return;
    }

    self.displayed_portfolio_ = portfolio;
    self.displayed_strategy_ = strategy;
    self.reset_view();
  }

  void clear_display(this BacktestChartState& self) noexcept
  {
    self.displayed_portfolio_.reset();
    self.displayed_strategy_.reset();
    self.reset_view();
  }

  auto update_result_count(this BacktestChartState& self,
                           std::size_t count) noexcept -> bool
  {
    if(count < self.result_count_) {
      self.clear_inspection();
    }
    return std::exchange(self.result_count_, count) != count;
  }

  auto inspected_bar(this const BacktestChartState& self) noexcept
   -> std::optional<std::size_t>
  {
    if(self.result_count_ == 0) {
      return std::nullopt;
    }
    return self.pinned_bar_.or_else([&self] { return self.hovered_bar_; })
     .value_or(self.result_count_ - 1);
  }

  void invalidate_results(this BacktestChartState& self) noexcept
  {
    self.clear_inspection();
    self.result_count_ = 0;
    self.request_fit();
  }

  void update_inspection(this BacktestChartState& self,
                         std::optional<std::size_t> hovered_bar,
                         bool clicked,
                         bool released,
                         bool dragging,
                         bool legend_hovered) noexcept
  {
    self.hovered_bar_ = hovered_bar;
    if(clicked && self.pinned_bar_) {
      self.pending_unpin_ = true;
      self.pending_pin_bar_.reset();
    } else if(legend_hovered) {
      self.pending_pin_bar_.reset();
      self.pending_unpin_ = false;
    } else if(clicked) {
      self.pending_pin_bar_ = hovered_bar;
    }

    if(dragging) {
      self.pending_pin_bar_.reset();
      self.pending_unpin_ = false;
    } else if(released) {
      if(self.pending_unpin_) {
        self.pinned_bar_.reset();
      } else if(self.pending_pin_bar_) {
        self.pinned_bar_ = self.pending_pin_bar_;
      }
      self.pending_pin_bar_.reset();
      self.pending_unpin_ = false;
    }
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
    const auto clamped = std::clamp<std::size_t>(count, 10, 100'000);
    if(self.follow_bar_count_ != clamped) {
      self.follow_bar_count_ = clamped;
      self.request_fit();
    }
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
    auto& ratios = self.current_row_ratios();
    if(ratios.size() != row_count) {
      ratios.assign(row_count, 1.0f);
      if(self.displayed_strategy_ && row_count > 1) {
        ratios[1] = 4.0f;
      }
      self.fit_requested_ = true;
    }

    return ratios;
  }

  void reset_row_ratios(this BacktestChartState& self) noexcept
  {
    self.current_row_ratios().clear();
    self.fit_requested_ = true;
  }

private:
  auto current_row_ratios(this BacktestChartState& self) noexcept
   -> std::vector<float>&
  {
    return self.displayed_strategy_ ? self.asset_row_ratios_
                                    : self.portfolio_row_ratios_;
  }

  void reset_view(this BacktestChartState& self) noexcept
  {
    self.invalidate_results();
    self.viewport_mode_ = BacktestChartViewportMode::FitAll;
    self.follow_bar_count_ = 150;
  }

  void clear_inspection(this BacktestChartState& self) noexcept
  {
    self.pinned_bar_.reset();
    self.hovered_bar_.reset();
    self.pending_pin_bar_.reset();
    self.pending_unpin_ = false;
  }

  std::optional<backtest::PortfolioStoreHandle> displayed_portfolio_{};
  std::optional<PortfolioStrategyKey> displayed_strategy_{};
  std::optional<std::size_t> pinned_bar_{};
  std::optional<std::size_t> hovered_bar_{};
  std::optional<std::size_t> pending_pin_bar_{};
  bool pending_unpin_{};
  std::size_t result_count_{};
  std::vector<float> asset_row_ratios_{};
  std::vector<float> portfolio_row_ratios_{};
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
