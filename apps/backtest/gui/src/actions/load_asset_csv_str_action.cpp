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

  state.asset_data.emplace(quotes.begin(), quotes.end());
  state.asset_name = get_asset_name();
  
  if(state.backtest.has_value()) {
    state.backtest->reset();
  }
}

auto LoadAssetCsvStrAction::get_asset_name() const noexcept
 -> const std::string&
{
  return asset_name_;
}

} // namespace pludux::apps

