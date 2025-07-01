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
  const auto asset_name = get_asset_name();
  auto asset_ptr =
   std::make_shared<backtest::Asset>(asset_name, std::move(asset_history));

  state.assets.emplace_back(std::move(asset_ptr));
}

auto LoadAssetCsvStrAction::get_asset_name() const noexcept
 -> const std::string&
{
  return asset_name_;
}

} // namespace pludux::apps
