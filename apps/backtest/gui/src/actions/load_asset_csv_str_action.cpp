#include <sstream>
#include <string>
#include <utility>

#include <pludux/backtest.hpp>

#include "../app_state_data.hpp"

#include "./load_asset_csv_str_action.hpp"

namespace pludux::apps {

LoadAssetCsvStrAction::LoadAssetCsvStrAction(std::string asset_name,
                                             std::string content)
: asset_name_(std::move(asset_name))
, content_(std::move(content))
{
}

void LoadAssetCsvStrAction::operator()(AppStateData& state) const
{
  auto csv_stream = std::stringstream{content_};

  const auto quotes = csv_daily_stock_data(csv_stream);
  auto asset_history = AssetHistory(quotes.begin(), quotes.end());
  state.assets.emplace_back(get_asset_name(), std::move(asset_history));
  state.backtests.emplace_back(state.strategy.value(), state.assets.back());
  state.selected_backtest_index = state.backtests.size() - 1;
}

auto LoadAssetCsvStrAction::get_asset_name() const noexcept
 -> const std::string&
{
  return asset_name_;
}

} // namespace pludux::apps
