#ifndef PLUDUX_APPS_BACKTEST_ACTIONS_LOAD_CSV_ASSET_ACTION_HPP
#define PLUDUX_APPS_BACKTEST_ACTIONS_LOAD_CSV_ASSET_ACTION_HPP

#include <string>

#include "../app_state_data.hpp"

namespace pludux::apps {

class LoadAssetCsvFileAction {
public:
  LoadAssetCsvFileAction(std::string path);

  void operator()(AppStateData& state) const;

  auto get_asset_name() const noexcept -> std::string;

private:
  std::string path_;
};

} // namespace pludux::apps

#endif
