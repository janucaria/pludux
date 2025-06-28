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

  LoadAssetCsvAction<LoadAssetCsvFileAction>::operator()(csv_stream, state);
}

auto LoadAssetCsvFileAction::get_asset_name() const noexcept -> std::string
{
  return std::filesystem::path{path_}.stem().string();
}

} // namespace pludux::apps
