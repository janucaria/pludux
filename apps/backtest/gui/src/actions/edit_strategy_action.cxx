module;

#include <algorithm>
#include <concepts>
#include <filesystem>
#include <format>
#include <fstream>
#include <istream>
#include <memory>
#include <ranges>
#include <sstream>
#include <string>
#include <utility>

export module pludux.apps.backtest:actions.edit_strategy_action;

import pludux.backtest;
import :application_state;

export namespace pludux::apps {

class EditStrategyAction {
public:
  EditStrategyAction(std::shared_ptr<backtest::Strategy> strategy_ptr,
                     backtest::Strategy strategy)
  : strategy_ptr_{std::move(strategy_ptr)}
  , new_strategy{std::move(strategy)}
  {
  }

  void operator()(this const EditStrategyAction& self,
                  ApplicationState& app_state)
  {
    const auto strategy_ptr = self.strategy_ptr_;
    *strategy_ptr = self.new_strategy;

    for(auto& backtest : app_state.backtests()) {
      if(backtest->strategy_ptr() == strategy_ptr) {
        backtest->reset();
      }
    }
  }

private:
  std::shared_ptr<backtest::Strategy> strategy_ptr_;
  backtest::Strategy new_strategy;
};

} // namespace pludux::apps
