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

export module pludux.apps.backtest:actions.load_asset_csv_action;

import :application_state;

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

  void operator()(this const LoadAssetCsvAction& self,
                  ApplicationState& app_state)
  {
    if constexpr(std::same_as<std::string, TSource>) {
      std::istringstream csv_stream{self.source_};
      self.load_asset_csv(self.asset_name_, csv_stream, app_state);
    } else if constexpr(std::same_as<std::filesystem::path, TSource>) {
      std::ifstream csv_stream{self.source_};
      if(!csv_stream.is_open()) {
        throw std::runtime_error{"Failed to open file: " +
                                 self.source_.string()};
      }
      self.load_asset_csv(self.asset_name_, csv_stream, app_state);
    }
  }

private:
  std::string asset_name_;
  TSource source_;

  static void load_asset_csv(const std::string& asset_name,
                             std::istream& csv_stream,
                             ApplicationState& app_state)
  {
    auto asset_ptr = std::make_shared<backtest::Asset>(asset_name);
    pludux::update_asset_from_csv(*asset_ptr, csv_stream);

    app_state.add_asset(std::move(asset_ptr));
  }
};

LoadAssetCsvAction(std::string, std::string) -> LoadAssetCsvAction<std::string>;

LoadAssetCsvAction(std::filesystem::path)
 -> LoadAssetCsvAction<std::filesystem::path>;

LoadAssetCsvAction(std::string) -> LoadAssetCsvAction<std::filesystem::path>;

} // namespace pludux::apps
