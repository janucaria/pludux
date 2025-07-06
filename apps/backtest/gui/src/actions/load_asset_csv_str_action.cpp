#include <filesystem>
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

  LoadAssetCsvAction<LoadAssetCsvStrAction>::operator()(csv_stream, state);
}

auto LoadAssetCsvStrAction::get_asset_name() const noexcept -> std::string
{
  return std::filesystem::path{asset_name_}.stem().string();
}

} // namespace pludux::apps
