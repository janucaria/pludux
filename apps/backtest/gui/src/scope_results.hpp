#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <ctime>
#include <limits>
#include <map>
#include <optional>
#include <tuple>
#include <vector>

namespace pludux::apps {

struct ScopeTradeOrder {
  bool open{};
  std::time_t timestamp{};
  std::size_t backtest_index{};
  std::size_t strategy_index{};
  std::size_t trade_id{};
};

inline auto portfolio_trade_before(const ScopeTradeOrder& lhs,
                                   const ScopeTradeOrder& rhs) -> bool
{
  if(lhs.open != rhs.open)
    return lhs.open;
  if(lhs.timestamp != rhs.timestamp)
    return lhs.timestamp > rhs.timestamp;
  return std::tie(lhs.backtest_index, lhs.strategy_index, lhs.trade_id) <
         std::tie(rhs.backtest_index, rhs.strategy_index, rhs.trade_id);
}

template<typename Results, typename Selection, typename EventType>
auto scope_event_indices(const Results& results,
                         const Selection& selected,
                         const std::optional<EventType>& type)
 -> std::vector<std::size_t>
{
  auto matches = std::vector<std::size_t>{};
  for(auto index = std::size_t{}; index < results.events().size(); ++index) {
    const auto ref = results.events()[index];
    const auto& run = results.backtests()[ref.backtest_index];
    const auto& event =
     run.timeline().trade_events(ref.bar_index)[ref.event_index];
    if((!selected || (run.key() == selected->run &&
                      event.strategy_index() == selected->strategy_index)) &&
       (!type || event.type() == *type ||
        (*type == EventType::scale_out && event.is_scale_out()))) {
      matches.push_back(index);
    }
  }
  // The source reference sequence carries execution order within a timestamp.
  std::stable_sort(matches.begin(), matches.end(), [&](auto lhs, auto rhs) {
    const auto timestamp = [&](auto index) {
      const auto ref = results.events()[index];
      return results.backtests()[ref.backtest_index]
       .timeline()
       .trade_events(ref.bar_index)[ref.event_index]
       .timestamp();
    };
    return timestamp(lhs) > timestamp(rhs);
  });
  return matches;
}

struct SingleAssetSummary {
  double realized_pnl{};
  double completed_pnl{};
  double deployed_capital{};
  double profits{};
  double losses{};
  double unrealized_pnl{};
  std::size_t closed_count{};
  std::size_t open_count{};
  std::size_t wins{};
  std::size_t losses_count{};
  std::size_t break_even_count{};
  std::time_t closed_duration{};
  std::time_t open_duration{};
};

template<typename Event>
auto realized_exit_pnl(const Event& event) -> double
{
  // The pre-exit average already includes the entry and scale-in fees.
  return event.position_size() *
          (event.price() - event.average_price_before()) -
         event.fees();
}

struct SingleAssetPnlPoint {
  double amount{};
  double percentage{};
  double drawdown_percentage{};
};

template<typename Timeline>
auto single_asset_pnl(const Timeline& timeline,
                      std::size_t strategy_index,
                      double initial_capital)
 -> std::vector<SingleAssetPnlPoint>
{
  auto points = std::vector<SingleAssetPnlPoint>{};
  points.reserve(timeline.size());
  auto realized_pnl = 0.0;
  auto peak_percentage = 0.0;
  for(auto index = std::size_t{}; index < timeline.size(); ++index) {
    for(const auto& event : timeline.trade_events(index)) {
      if(event.strategy_index() == strategy_index &&
         (event.is_exit() || event.is_scale_out())) {
        realized_pnl += realized_exit_pnl(event);
      }
    }
    const auto& open = timeline.open_position(index);
    const auto unrealized_pnl = open && open->strategy_index() == strategy_index
                                 ? open->unrealized_pnl()
                                 : 0.0;
    const auto amount = realized_pnl + unrealized_pnl;
    const auto percentage = initial_capital > 0.0
                             ? amount / initial_capital * 100.0
                             : std::numeric_limits<double>::quiet_NaN();
    peak_percentage = std::max(peak_percentage, percentage);
    points.push_back({amount, percentage, peak_percentage - percentage});
  }
  return points;
}

template<typename Timeline>
auto has_open_position(const Timeline& timeline, std::size_t strategy_index)
 -> bool
{
  if(timeline.empty()) {
    return false;
  }
  const auto& position = timeline.open_position(timeline.size() - 1);
  return position && position->strategy_index() == strategy_index;
}

template<typename Timeline>
auto summarize_single_asset(const Timeline& timeline,
                            std::size_t strategy_index) -> SingleAssetSummary
{
  struct Trade {
    double pnl{};
    double investment{};
    std::time_t entry_timestamp{};
  };
  auto trades = std::map<std::size_t, Trade>{};
  auto summary = SingleAssetSummary{};
  for(auto index = std::size_t{}; index < timeline.size(); ++index) {
    for(const auto& event : timeline.trade_events(index)) {
      if(event.strategy_index() != strategy_index)
        continue;
      using Type = decltype(event.type());
      switch(event.type()) {
      case Type::entry:
        trades[event.trade_id()].entry_timestamp = event.timestamp();
        [[fallthrough]];
      case Type::scale_in:
        trades[event.trade_id()].investment +=
         std::abs(event.investment_after() - event.investment_before());
        break;
      case Type::scale_out:
      case Type::exit_signal:
      case Type::stop_loss:
      case Type::take_profit: {
        auto& trade = trades[event.trade_id()];
        const auto pnl = realized_exit_pnl(event);
        trade.pnl += pnl;
        summary.realized_pnl += pnl;
        if(event.position_size_after() == 0.0) {
          ++summary.closed_count;
          summary.closed_duration += event.timestamp() - trade.entry_timestamp;
          summary.completed_pnl += trade.pnl;
          summary.deployed_capital += trade.investment;
          if(trade.pnl > 0.0) {
            ++summary.wins;
            summary.profits += trade.pnl;
          } else if(trade.pnl < 0.0) {
            ++summary.losses_count;
            summary.losses -= trade.pnl;
          } else {
            ++summary.break_even_count;
          }
          trades.erase(event.trade_id());
        }
        break;
      }
      default:
        break;
      }
    }
  }
  if(!timeline.empty()) {
    const auto& open = timeline.open_position(timeline.size() - 1);
    if(open && open->strategy_index() == strategy_index) {
      summary.open_count = 1;
      summary.unrealized_pnl = open->unrealized_pnl();
      summary.open_duration = open->duration();
    }
  }
  return summary;
}

} // namespace pludux::apps
