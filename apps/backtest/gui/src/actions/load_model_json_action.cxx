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

export module pludux.apps.backtest:actions.load_model_json_action;

import pludux.backtest;
import pludux.apps.backtest.application_state;

export namespace pludux::apps {

template<typename TSource>
  requires std::same_as<std::string, TSource> ||
           std::same_as<std::filesystem::path, TSource>
class LoadModelJsonAction {
public:
  LoadModelJsonAction(std::string config_name, std::string source)
    : model_name_{
     std::filesystem::path{std::move(config_name)}.stem().string()}
  , source_{std::move(source)}
  {
  }

  LoadModelJsonAction(std::filesystem::path source)
  : model_name_{source.stem().string()}
  , source_{std::move(source)}
  {
  }

  LoadModelJsonAction(std::string source)
  : LoadModelJsonAction{std::filesystem::path{std::move(source)}}
  {
  }

  void operator()(this const LoadModelJsonAction& self,
                  ApplicationState& app_state)
  {
    if constexpr(std::same_as<TSource, std::string>) {
      auto json_stream = std::istringstream{self.source_};
      self.model_json_load(self.model_name_, json_stream, app_state);
    } else if constexpr(std::same_as<TSource, std::filesystem::path>) {
      auto json_file = std::ifstream{self.source_};
      if(!json_file) {
        const auto error_message = std::format(
          "Failed to open model file '{}'", self.source_.string());
        throw std::invalid_argument{error_message};
      }
      self.model_json_load(self.model_name_, json_file, app_state);
    }
  }

private:
  std::string model_name_;
  TSource source_;

  static void model_json_load(const std::string& model_name,
                              std::istream& new_json_stream,
                              ApplicationState& app_state)
  {
    auto parsed_model = [&]() {
      try {
        return backtest::parse_model(model_name, new_json_stream);
      } catch(const std::exception& e) {
        const auto error_message = std::format(
          "Failed to parse model '{}':\n{}", model_name, e.what());
        throw std::invalid_argument{error_message};
      }
    }();

    app_state.add_model(std::move(parsed_model));
  }
};

LoadModelJsonAction(std::string, std::string)
 -> LoadModelJsonAction<std::string>;

LoadModelJsonAction(std::filesystem::path)
 -> LoadModelJsonAction<std::filesystem::path>;

LoadModelJsonAction(std::string)
 -> LoadModelJsonAction<std::filesystem::path>;

} // namespace pludux::apps
