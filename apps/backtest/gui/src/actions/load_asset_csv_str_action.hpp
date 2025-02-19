#ifndef PLUDUX_APPS_BACKTEST_ACTIONS_LOAD_CSV_STR_ASSET_ACTION_HPP
#define PLUDUX_APPS_BACKTEST_ACTIONS_LOAD_CSV_STR_ASSET_ACTION_HPP

#include <string>

#include "../app_state_data.hpp"

namespace pludux::apps {

class LoadAssetCsvStrAction {
public:
  LoadAssetCsvStrAction(std::string asset_name, std::string content);

  void operator()(AppStateData& state) const;

  auto get_asset_name() const noexcept -> const std::string&;

private:
  std::string asset_name_;
  std::string content_;
};

} // namespace pludux::apps

#endif
