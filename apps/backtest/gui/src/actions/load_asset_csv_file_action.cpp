#include <filesystem>
#include <fstream>
#include <string>
#include <utility>

#include <pludux/backtest.hpp>

#include "../app_state_data.hpp"

#include "./load_asset_csv_file_action.hpp"

namespace pludux::apps {

LoadAssetCsvFileAction::LoadAssetCsvFileAction(std::string path)
: path_(std::move(path))
{
}

void LoadAssetCsvFileAction::operator()(AppStateData& state) const
{
  auto csv_stream = std::ifstream{path_};
  if(!csv_stream.is_open()) {
    const auto error_message = "Failed to open file: " + path_;
    throw std::runtime_error{error_message};
  }

  const auto quotes = csv_daily_stock_data(csv_stream);
  auto asset_history = AssetHistory(quotes.begin(), quotes.end());
  const auto asset_name = get_asset_name();
  auto asset_ptr =
   std::make_unique<backtest::Asset>(asset_name, std::move(asset_history));

  const auto& asset = *(state.assets.emplace_back(std::move(asset_ptr)).get());
  state.selected_asset_index = state.assets.size() - 1;

  if(state.selected_strategy_index >= 0 && state.strategies.size() > 0) {
    const auto& strategy = *state.strategies[state.selected_strategy_index];
    state.backtests.emplace_back(strategy, asset, state.quote_access);
  }
}

auto LoadAssetCsvFileAction::get_asset_name() const noexcept -> std::string
{
  return std::filesystem::path{path_}.filename().string();
}

} // namespace pludux::apps
