#ifndef PLUDUX_APPS_BACKTEST_ACTIONS_CHANGE_CONFIG_JSON_ACTION_HPP
#define PLUDUX_APPS_BACKTEST_ACTIONS_CHANGE_CONFIG_JSON_ACTION_HPP

#include <filesystem>
#include <string>
#include <utility>

#include <pludux/backtest.hpp>

#include "../app_state_data.hpp"

namespace pludux::apps {

template<class TImpl>
class ChangeStrategyJsonAction {
protected:
  void operator()(std::istream& json_stream, AppStateData& state) const
  {
    const auto strategy_name =
     static_cast<const TImpl*>(this)->get_strategy_name();
    auto parsed_strategy = parse_backtest_strategy_json(
     strategy_name, json_stream, state.quote_access);
    auto strategy_ptr =
     std::make_shared<backtest::Strategy>(std::move(parsed_strategy));

    auto& strategies = state.strategies;
    auto find_it = std::find_if(strategies.begin(),
                                strategies.end(),
                                [&strategy_name](const auto& strategy_ptr) {
                                  return strategy_ptr &&
                                         strategy_ptr->name() == strategy_name;
                                });

    if(find_it != strategies.end()) {
      auto& existing_strategy_ptr = *find_it;
      *existing_strategy_ptr = std::move(*strategy_ptr);

      for(auto& backtest : state.backtests) {
        if(backtest.strategy_ptr() &&
           backtest.strategy_ptr()->name() == strategy_name) {
          backtest.reset();
        }
      }
    } else {
      strategies.emplace_back(std::move(strategy_ptr));
    }
  }
};

} // namespace pludux::apps

#endif