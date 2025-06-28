#ifndef PLUDUX_APPS_BACKTEST_ACTIONS_LOAD_ASSET_CSV_ACTION_HPP
#define PLUDUX_APPS_BACKTEST_ACTIONS_LOAD_ASSET_CSV_ACTION_HPP

#include <iostream>
#include <memory>
#include <string>
#include <utility>

#include <pludux/backtest.hpp>

#include "../app_state_data.hpp"

namespace pludux::apps {

template<class TImpl>
class LoadAssetCsvAction {
protected:
  void operator()(std::istream& csv_stream, AppStateData& state) const
  {
    const auto quotes = csv_daily_stock_data(csv_stream);
    auto asset_history = AssetHistory(quotes.begin(), quotes.end());
    const auto asset_name = static_cast<const TImpl*>(this)->get_asset_name();
    auto asset_ptr = std::make_shared<backtest::Asset>(
     asset_name, std::move(asset_history), state.quote_access);

    auto find_it = state.assets.find(asset_name);
    if(find_it != state.assets.end()) {
      auto& existing_asset_ptr = *find_it;
      *existing_asset_ptr = std::move(*asset_ptr);

      auto& backtests = state.backtests;
      for(auto& backtest : backtests) {
        if(backtest.asset_ptr() && backtest.asset_ptr()->name() == asset_name) {
          backtest.reset();
        }
      }
    } else {
      state.assets.emplace(asset_ptr);
    }
  }
};

} // namespace pludux::apps

#endif
