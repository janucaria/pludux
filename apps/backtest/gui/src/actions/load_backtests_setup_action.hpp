#ifndef PLUDUX_APPS_BACKTEST_ACTIONS_LOAD_BACKTESTS_SETUP_ACTION_HPP
#define PLUDUX_APPS_BACKTEST_ACTIONS_LOAD_BACKTESTS_SETUP_ACTION_HPP

#include <filesystem>
#include <string>
#include <utility>

#include <rapidcsv.h>

#include <pludux/backtest.hpp>

#include "../app_state_data.hpp"

namespace pludux::apps {

namespace details {

template<class TImpl>
class LoadBacktestsSetupAction {
protected:
  void operator()(std::istream& csv_stream, AppStateData& state) const
  {
    auto csv_doc = rapidcsv::Document(csv_stream);
    const auto column_names = csv_doc.GetColumnNames();

    if(column_names.empty()) {
      return;
    }

    const auto asset_column_index = csv_doc.GetColumnIdx("Asset");
    const auto strategy_column_index = csv_doc.GetColumnIdx("Strategy");

    if(asset_column_index == -1 || strategy_column_index == -1) {
      throw std::runtime_error(
       "CSV file must contain 'asset' and 'strategy' columns.");
    }

    const auto& assets = state.assets;
    const auto& strategies = state.strategies;

    for(auto i = 0; i < csv_doc.GetRowCount(); ++i) {
      const auto asset_name =
       csv_doc.GetCell<std::string>(asset_column_index, i);
      const auto strategy_name =
       csv_doc.GetCell<std::string>(strategy_column_index, i);

      if(asset_name.empty() || strategy_name.empty()) {
        continue;
      }

      const auto backtest_name_column_index = csv_doc.GetColumnIdx("Name");

      std::string backtest_name;
      if(backtest_name_column_index != -1) {
        backtest_name =
         csv_doc.GetCell<std::string>(backtest_name_column_index, i);
      } else {
        backtest_name = asset_name + " / " + strategy_name;
      }

      auto asset_it = std::find_if(
       assets.begin(), assets.end(), [&asset_name](const auto& asset_ptr) {
         return asset_ptr && asset_ptr->name() == asset_name;
       });

      if(asset_it == assets.end()) {
        continue;
      }

      auto asset_ptr = *asset_it;

      auto strategy_it = std::find_if(
       strategies.begin(),
       strategies.end(),
       [&strategy_name](const auto& strategy_ptr) {
         return strategy_ptr && strategy_ptr->name() == strategy_name;
       });
      if(strategy_it == strategies.end()) {
        continue;
      }

      auto strategy_ptr = *strategy_it;

      state.backtests.emplace_back(
       backtest_name, std::move(strategy_ptr), std::move(asset_ptr));
    }

    if(state.selected_backtest_index < 0 && !state.backtests.empty()) {
      state.selected_backtest_index = 0;
    }
  }
};

} // namespace details

} // namespace pludux::apps

#endif