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
  auto asset_ptr = std::make_unique<backtest::Asset>(get_asset_name(),
                                                     std::move(asset_history));
  state.assets.emplace_back(std::move(asset_ptr));
  if(state.strategy.has_value()) {
    const auto& asset = *state.assets.back();
    state.backtests.emplace_back(
     state.strategy.value(), asset, state.quote_access);
  }
  state.selected_asset_index = state.assets.size() - 1;
}

auto LoadAssetCsvStrAction::get_asset_name() const noexcept
 -> const std::string&
{
  return asset_name_;
}

} // namespace pludux::apps
