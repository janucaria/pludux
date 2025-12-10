module;

#include <algorithm>
#include <concepts>
#include <filesystem>
#include <format>
#include <fstream>
#include <istream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <rapidcsv.h>

export module pludux.apps.backtest:actions.load_backtests_setup_action;

import :app_state_data;

export namespace pludux::apps {

template<class TSource>
  requires std::same_as<TSource, std::string> ||
           std::same_as<TSource, std::filesystem::path>
class LoadBacktestsSetupAction {
public:
  LoadBacktestsSetupAction(TSource source)
  : source_{std::move(source)}
  {
  }

  void operator()(this const LoadBacktestsSetupAction& self,
                  AppStateData& state)
  {
    if constexpr(std::same_as<TSource, std::string>) {
      auto stream = std::istringstream{self.source_};
      self.load_backtests_setup(stream, state);
    } else if constexpr(std::same_as<TSource, std::filesystem::path>) {
      auto file_stream = std::ifstream{self.source_};
      if(!file_stream.is_open()) {
        throw std::runtime_error(
         std::format("Failed to open file: '{}'", self.source_.string()));
      }
      self.load_backtests_setup(file_stream, state);
    }
  }

private:
  TSource source_;

  static void load_backtests_setup(std::istream& csv_stream,
                                   AppStateData& state)
  {
    auto csv_doc = rapidcsv::Document(csv_stream);
    const auto column_names = csv_doc.GetColumnNames();

    if(column_names.empty()) {
      return;
    }

    const auto asset_column_index = csv_doc.GetColumnIdx("Asset");
    const auto strategy_column_index = csv_doc.GetColumnIdx("Strategy");

    if(asset_column_index == -1 || strategy_column_index == -1) {
      throw std::runtime_error(
       "CSV file must contain 'asset' and 'strategy' columns.");
    }

    const auto profile_column_index = csv_doc.GetColumnIdx("Profile");
    const auto broker_column_index = csv_doc.GetColumnIdx("Broker");

    const auto& assets = state.assets;
    const auto& strategies = state.strategies;

    for(auto i = 0; i < csv_doc.GetRowCount(); ++i) {
      const auto asset_name =
       csv_doc.GetCell<std::string>(asset_column_index, i);
      const auto strategy_name =
       csv_doc.GetCell<std::string>(strategy_column_index, i);

      if(asset_name.empty() || strategy_name.empty()) {
        continue;
      }

      const auto broker_name =
       broker_column_index != -1
        ? csv_doc.GetCell<std::string>(broker_column_index, i)
        : std::string{"Default"};

      const auto profile_name =
       profile_column_index != -1
        ? csv_doc.GetCell<std::string>(profile_column_index, i)
        : std::string{"Default"};

      const auto backtest_name_column_index = csv_doc.GetColumnIdx("Name");

      auto backtest_name = std::string{};
      if(backtest_name_column_index != -1) {
        backtest_name =
         csv_doc.GetCell<std::string>(backtest_name_column_index, i);
      } else {
        backtest_name =
         std::format("{} / {} / {}", asset_name, strategy_name, profile_name);
      }

      auto asset_it = std::find_if(
       assets.begin(), assets.end(), [&asset_name](const auto& asset_ptr) {
         return asset_ptr && asset_ptr->name() == asset_name;
       });

      if(asset_it == assets.end()) {
        continue;
      }

      auto asset_ptr = *asset_it;

      auto strategy_it = std::find_if(
       strategies.begin(),
       strategies.end(),
       [&strategy_name](const auto& strategy_ptr) {
         return strategy_ptr && strategy_ptr->name() == strategy_name;
       });
      if(strategy_it == strategies.end()) {
        continue;
      }

      auto strategy_ptr = *strategy_it;

      auto broker_it =
       std::find_if(state.brokers.begin(),
                    state.brokers.end(),
                    [&broker_name](const auto& broker_ptr) {
                      return broker_ptr && broker_ptr->name() == broker_name;
                    });

      if(broker_it == state.brokers.end()) {
        continue;
      }

      auto broker_ptr = *broker_it;

      auto profile_it =
       std::find_if(state.profiles.begin(),
                    state.profiles.end(),
                    [&profile_name](const auto& profile_ptr) {
                      return profile_ptr && profile_ptr->name() == profile_name;
                    });

      if(profile_it == state.profiles.end()) {
        continue;
      }

      auto profile_ptr = *profile_it;

      state.backtests.emplace_back(backtest_name,
                                   std::move(strategy_ptr),
                                   std::move(asset_ptr),
                                   std::move(broker_ptr),
                                   std::move(profile_ptr));
    }

    if(state.selected_backtest_index < 0 && !state.backtests.empty()) {
      state.selected_backtest_index = 0;
    }
  }
};

} // namespace pludux::apps
