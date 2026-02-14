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

export module pludux.apps.backtest:actions.load_strategy_json_action;

import :application_state;

export namespace pludux::apps {

template<typename TSource>
  requires std::same_as<std::string, TSource> ||
           std::same_as<std::filesystem::path, TSource>
class LoadStrategyJsonAction {
public:
  LoadStrategyJsonAction(std::string config_name, std::string source)
  : strategy_name_{std::filesystem::path{std::move(config_name)}
                    .stem()
                    .string()}
  , source_{std::move(source)}
  {
  }

  LoadStrategyJsonAction(std::filesystem::path source)
  : strategy_name_{source.stem().string()}
  , source_{std::move(source)}
  {
  }

  LoadStrategyJsonAction(std::string source)
  : LoadStrategyJsonAction{std::filesystem::path{std::move(source)}}
  {
  }

  void operator()(this const LoadStrategyJsonAction& self,
                  ApplicationState& app_state)
  {
    if constexpr(std::same_as<TSource, std::string>) {
      auto json_stream = std::istringstream{self.source_};
      self.strategy_json_load(self.strategy_name_, json_stream, app_state);
    } else if constexpr(std::same_as<TSource, std::filesystem::path>) {
      auto json_file = std::ifstream{self.source_};
      if(!json_file) {
        const auto error_message = std::format(
         "Failed to open strategy file '{}'", self.source_.string());
        throw std::invalid_argument{error_message};
      }
      self.strategy_json_load(self.strategy_name_, json_file, app_state);
    }
  }

private:
  std::string strategy_name_;
  TSource source_;

  static void strategy_json_load(const std::string& strategy_name,
                                 std::istream& new_json_stream,
                                 ApplicationState& app_state)
  {
    auto parsed_strategy = [&]() {
      try {
        return backtest::parse_backtest_strategy_json(strategy_name,
                                                      new_json_stream);
      } catch(const std::exception& e) {
        const auto error_message = std::format(
         "Failed to parse strategy '{}':\n{}", strategy_name, e.what());
        throw std::invalid_argument{error_message};
      }
    }();

    auto strategy_ptr =
     std::make_shared<backtest::Strategy>(std::move(parsed_strategy));
    app_state.add_strategy(std::move(strategy_ptr));
  }
};

LoadStrategyJsonAction(std::string, std::string)
 -> LoadStrategyJsonAction<std::string>;

LoadStrategyJsonAction(std::filesystem::path)
 -> LoadStrategyJsonAction<std::filesystem::path>;

LoadStrategyJsonAction(std::string)
 -> LoadStrategyJsonAction<std::filesystem::path>;

} // namespace pludux::apps
