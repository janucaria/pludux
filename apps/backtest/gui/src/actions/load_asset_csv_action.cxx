module;

#include <algorithm>
#include <concepts>
#include <filesystem>
#include <fstream>
#include <istream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <pludux/backtest.hpp>

export module pludux.apps.backtest.actions:load_asset_csv_action;

import pludux.apps.backtest.app_state_data;

export namespace pludux::apps {

template<class TSource>
  requires std::same_as<std::string, TSource> ||
           std::same_as<std::filesystem::path, TSource>
class LoadAssetCsvAction {
public:
  LoadAssetCsvAction(std::string asset_name, std::string content)
  : asset_name_{std::move(asset_name)}
  , source_{std::move(content)}
  {
  }

  LoadAssetCsvAction(std::filesystem::path source)
  : asset_name_{source.stem().string()}
  , source_{std::move(source)}
  {
  }

  LoadAssetCsvAction(std::string source)
  : LoadAssetCsvAction{std::filesystem::path{std::move(source)}}
  {
  }

  void operator()(this const auto& self, AppStateData& state)
  {
    if constexpr(std::same_as<std::string, TSource>) {
      std::istringstream csv_stream{self.source_};
      self.load_asset_csv(self.asset_name_, csv_stream, state);
    } else if constexpr(std::same_as<std::filesystem::path, TSource>) {
      std::ifstream csv_stream{self.source_};
      if(!csv_stream.is_open()) {
        throw std::runtime_error{"Failed to open file: " +
                                 self.source_.string()};
      }
      self.load_asset_csv(self.asset_name_, csv_stream, state);
    }
  }

private:
  std::string asset_name_;
  TSource source_;

  static void load_asset_csv(const std::string& asset_name,
                             std::istream& csv_stream,
                             AppStateData& state)
  {
    auto asset_history = csv_daily_stock_data(csv_stream);
    auto asset_ptr =
     std::make_shared<backtest::Asset>(asset_name, std::move(asset_history));

    auto& assets = state.assets;
    auto find_it = std::find_if(
     assets.begin(), assets.end(), [&asset_name](const auto& asset) {
       return asset->name() == asset_name;
     });
    if(find_it != assets.end()) {
      auto existing_asset_ptr = *find_it;
      *existing_asset_ptr = std::move(*asset_ptr);

      for(auto& backtest : state.backtests) {
        if(backtest.asset_ptr() && backtest.asset_ptr()->name() == asset_name) {
          backtest.reset();
        }
      }
    } else {
      assets.push_back(std::move(asset_ptr));
    }
  }
};

LoadAssetCsvAction(std::string, std::string) -> LoadAssetCsvAction<std::string>;

LoadAssetCsvAction(std::filesystem::path)
 -> LoadAssetCsvAction<std::filesystem::path>;

LoadAssetCsvAction(std::string) -> LoadAssetCsvAction<std::filesystem::path>;

} // namespace pludux::apps
