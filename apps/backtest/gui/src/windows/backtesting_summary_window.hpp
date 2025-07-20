#ifndef PLUDUX_APPS_BACKTEST_WINDOWS_BACKTEST_SUMMARY_WINDOW_HPP
#define PLUDUX_APPS_BACKTEST_WINDOWS_BACKTEST_SUMMARY_WINDOW_HPP

#include <cstddef>
#include <string_view>

#include "../app_state.hpp"

namespace pludux::apps {

class BacktestSummaryWindow {
public:
  BacktestSummaryWindow();

  void render(AppState& app_state);

private:
  void draw_row(std::string_view label, std::string_view value) const;

  void draw_duration_row(std::string_view label, std::size_t duration) const;

  void draw_datetime_row(std::string_view label, std::time_t timestamp) const;

  void draw_empty_row() const;

  void draw_spacer_row() const;

  void draw_float_row(std::string_view label, double value) const;

  void draw_count_row(std::string_view label, std::size_t value) const;

  void draw_count_row_with_rate(std::string_view label,
                                std::size_t value,
                                double rate) const;

  void draw_currency_row(std::string_view label, double value) const;

  void draw_currency_with_rate_row(std::string_view label,
                                   double value,
                                   double rate) const;

  void draw_currency_with_percent_row(std::string_view label,
                                      double value,
                                      double total) const;
};

} // namespace pludux::apps

#endif
