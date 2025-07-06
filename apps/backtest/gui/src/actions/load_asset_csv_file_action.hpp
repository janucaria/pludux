#ifndef PLUDUX_APPS_BACKTEST_ACTIONS_LOAD_ASSET_CSV_FILE_ACTION_HPP
#define PLUDUX_APPS_BACKTEST_ACTIONS_LOAD_ASSET_CSV_FILE_ACTION_HPP

#include <string>

#include "../app_state_data.hpp"
#include "./load_asset_csv_action.hpp"

namespace pludux::apps {

class LoadAssetCsvFileAction
: public LoadAssetCsvAction<LoadAssetCsvFileAction> {
public:
  LoadAssetCsvFileAction(std::string path);

  void operator()(AppStateData& state) const;

  auto get_asset_name() const noexcept -> std::string;

private:
  std::string path_;
};

} // namespace pludux::apps

#endif
