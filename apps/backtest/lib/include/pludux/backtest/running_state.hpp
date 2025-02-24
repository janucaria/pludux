#ifndef PLUDUX_PLUDUX_BACKTEST_RUNNING_STATE_HPP
#define PLUDUX_PLUDUX_BACKTEST_RUNNING_STATE_HPP

#include <pludux/asset_quote.hpp>

namespace pludux::backtest {

class RunningState {
public:
  RunningState(AssetQuote asset_quote, std::size_t current_index);

  auto asset_quote() const noexcept -> const AssetQuote&;

  auto asset_index() const noexcept -> std::size_t;

  auto capital_risk() const noexcept -> double;

  void capital_risk(double capital_risk) noexcept;

  auto asset_snapshot() const noexcept -> const AssetSnapshot&;

  auto price() const noexcept -> double;

  auto timestamp() const noexcept -> double;

  auto open() const noexcept -> double;

  auto high() const noexcept -> double;

  auto low() const noexcept -> double;

  auto close() const noexcept -> double;

  auto volume() const noexcept -> double;

private:
  std::size_t asset_index_;
  double capital_risk_;
  AssetQuote asset_quote_;
};

} // namespace pludux::backtest

#endif
