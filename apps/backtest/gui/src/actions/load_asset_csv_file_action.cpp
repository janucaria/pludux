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

  state.asset_data.emplace(quotes.begin(), quotes.end());
  state.asset_name = get_asset_name();
  state.resource_changed = true;
}

auto LoadAssetCsvFileAction::get_asset_name() const noexcept -> std::string
{
  return std::filesystem::path{path_}.filename().string();
}

} // namespace pludux::apps
