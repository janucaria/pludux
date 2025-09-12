#include <format>
#include <iterator>
#include <ranges>
#include <string>

#include <fstream>
#include <iostream>
#include <print>
#include <sstream>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>
#include <jsoncons_ext/mergepatch/mergepatch.hpp>

#include "../app_state.hpp"

#include "./strategies_window.hpp"

namespace pludux::apps {

static auto expanded_json_schema(const jsoncons::json& root_schema,
                                 const jsoncons::json& schema) -> jsoncons::json
{
  using namespace jsoncons;

  if(schema.is_object()) {
    if(schema.contains("$ref")) {
      auto ref = schema.at("$ref").as_string();
      if(!ref.empty() && ref[0] == '#') {
        auto ref_path = ref.substr(1);
        auto ref_schema = jsonpointer::get(root_schema, ref_path);
        return expanded_json_schema(root_schema, ref_schema);
      }
    } else if(schema.contains("allOf")) {
      auto all_of = schema.at("allOf").array_range();

      json result = json::object();

      for(const auto& sub_schema : all_of) {
        auto expanded_sub_schema =
         expanded_json_schema(root_schema, sub_schema);
        mergepatch::apply_merge_patch(result, expanded_sub_schema);
      }

      return result;
    } else if(schema.contains("oneOf")) {
      // For simplicity, just return the first option
      auto one_of = schema.at("oneOf").array_range();
      if(one_of.begin() != one_of.end()) {
        return expanded_json_schema(root_schema, *one_of.begin());
      }
    }
  }

  return schema;
}

StrategiesWindow::StrategiesWindow()
{
  using namespace jsoncons;

  auto json_data_stream =
   std::ifstream{"C:\\Users\\Januar-PC\\Projects\\pludux\\."
                 "vscode\\docs\\strategy.example.json"};

  auto json_data_str =
   std::string{std::istreambuf_iterator<char>{json_data_stream},
               std::istreambuf_iterator<char>{}};

  data_json = ojson::parse(json_data_str);
}

void StrategiesWindow::render(AppState& app_state)
{
  ImGui::Begin("Strategies", nullptr);

  switch(current_page_) {
  case Page::AddNew:
    render_add_new_strategy(app_state);
    break;
  case Page::List:
  default:
    render_strategies_list(app_state);
    break;
  }

  ImGui::End();
}

void StrategiesWindow::render_strategies_list(AppState& app_state)
{
  auto& state = app_state.state();
  auto& strategies = state.strategies;
  if(!strategies.empty()) {
    for(auto i = 0; i < strategies.size(); ++i) {
      const auto& strategy = *strategies[i];
      const auto& strategy_name = strategy.name();

      ImGui::PushID(i);

      ImGui::SetNextItemAllowOverlap();

      ImGui::Text("%s", strategy_name.c_str());

      ImGui::SameLine();

      ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 50);

      if(ImGui::Button("Delete")) {
        app_state.push_action([i](AppStateData& state) {
          const auto& strategies = state.strategies;
          const auto it = std::next(strategies.begin(), i);
          const auto& strategy_ptr = *it;

          auto& backtests = state.backtests;
          for(auto j = 0; j < backtests.size(); ++j) {
            auto& backtest = backtests[j];
            if(backtest.strategy_ptr()->name() == strategy_ptr->name()) {
              backtests.erase(std::next(backtests.begin(), j));

              if(state.selected_backtest_index > j ||
                 state.selected_backtest_index >= backtests.size()) {
                --state.selected_backtest_index;
              }

              // Adjust the index since we removed an element
              --j;
            }
          }

          state.strategies.erase(it);
        });
      }

      ImGui::PopID();
    }
  }
}

namespace {

struct SeriesJsonMeta {
  std::string title;
  jsoncons::ojson default_value;

  SeriesJsonMeta()
  : SeriesJsonMeta{""}
  {
  }

  SeriesJsonMeta(std::string title)
  : SeriesJsonMeta{std::move(title), jsoncons::ojson::object()}
  {
  }

  SeriesJsonMeta(std::string title, jsoncons::ojson default_value)
  : title(std::move(title))
  , default_value(std::move(default_value))
  {
  }
};

auto get_series_json_meta() -> std::map<std::string, SeriesJsonMeta>
{
  using namespace jsoncons;

  auto meta = std::map<std::string, SeriesJsonMeta>{};

  meta["SELECT_OUTPUT"] = SeriesJsonMeta{"Select Output"};
  meta["SELECT_OUTPUT"].default_value =
   ojson::object{{{"method", "SELECT_OUTPUT"},
                  {"reference", ojson::object{{{"method", "CLOSE"}}}},
                  {"output", "default"}}};

  meta["VARIABLE"] = SeriesJsonMeta{"Variable"};
  meta["VARIABLE"].default_value.insert_or_assign("method", "VARIABLE");
  meta["VARIABLE"].default_value.insert_or_assign("name", "");

  meta["CLOSE"] = SeriesJsonMeta{"Close Price"};
  meta["CLOSE"].default_value.insert_or_assign("method", "CLOSE");

  meta["OPEN"] = SeriesJsonMeta{"Open Price"};
  meta["OPEN"].default_value.insert_or_assign("method", "OPEN");

  meta["HIGH"] = SeriesJsonMeta{"High Price"};
  meta["HIGH"].default_value.insert_or_assign("method", "HIGH");

  meta["LOW"] = SeriesJsonMeta{"Low Price"};
  meta["LOW"].default_value.insert_or_assign("method", "LOW");

  meta["VOLUME"] = SeriesJsonMeta{"Volume"};
  meta["VOLUME"].default_value.insert_or_assign("method", "VOLUME");

  meta["VALUE"] = SeriesJsonMeta{"Value"};
  meta["VALUE"].default_value.insert_or_assign("method", "VALUE");
  meta["VALUE"].default_value.insert_or_assign("value", 0.0);

  meta["BB"] = SeriesJsonMeta{"Bollinger Bands (BB)"};
  meta["BB"].default_value.insert_or_assign("method", "BB");
  meta["BB"].default_value.insert_or_assign("period", 20);
  meta["BB"].default_value.insert_or_assign("stddev", 2.0);
  meta["BB"].default_value.insert_or_assign("source",
                                            ojson::object{{"method", "CLOSE"}});

  return meta;
}

void render_series_method(
 const std::map<std::string, SeriesJsonMeta>& series_meta,
 jsoncons::ojson& series_json)
{
  auto series_method = series_json.at("method").as_string();

  ImGui::PushID(series_method.c_str());

  {
    const auto combo_preview_value = series_meta.contains(series_method)
                                      ? series_meta.at(series_method).title
                                      : series_method;
    if(ImGui::BeginCombo("Series", combo_preview_value.c_str())) {
      auto filter = ImGuiTextFilter{};

      if(ImGui::IsWindowAppearing()) {
        ImGui::SetKeyboardFocusHere();
        filter.Clear();
      }
      ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F);
      filter.Draw("##Filter", -FLT_MIN);

      for(auto& [method, meta] : series_meta) {
        const bool is_selected = series_method == method;
        const auto method_title = meta.title;

        if(filter.PassFilter(method_title.c_str())) {
          if(ImGui::Selectable(method_title.c_str(), is_selected)) {
            series_json = meta.default_value;
          }
        }
      }
      ImGui::EndCombo();
    }

    ImGui::Indent();

    {
      auto series_method = series_json.at("method").as_string();
      if(series_method == "SELECT_OUTPUT") {
        const auto output_options =
         std::vector<std::string>{"default", "upper", "middle", "lower"};

        ImGui::Text("Reference:");
        render_series_method(series_meta, series_json.at("reference"));

        ImGui::Text("Output:");
        auto& output_json = series_json.at("output");
        {
          const auto output_str = output_json.as_string();
          if(ImGui::BeginCombo("##output", output_str.c_str())) {
            for(int n = 0; n < output_options.size(); n++) {
              const auto& output_option = output_options[n];
              const bool is_selected = output_str == output_option;
              if(ImGui::Selectable(output_option.c_str(), is_selected)) {
                output_json = output_option;
              }
            }
            ImGui::EndCombo();
          }
        }
      } else if(series_method == "VARIABLE") {
        auto& name_json = series_json.at("name");
        ImGui::Text("Name:");
        auto name = name_json.as_string();
        if(ImGui::InputText("##name", &name)) {
          name_json = name;
        }
      }
    }

    ImGui::Unindent();
  }
  ImGui::PopID();
}

} // namespace

void StrategiesWindow::render_add_new_strategy(AppState& app_state)
{
  using namespace jsoncons;

  const auto& state = app_state.state();

  ImGui::BeginGroup();
  ImGui::BeginChild("item view",
                    ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

  ImGui::Text("Add New Strategy");
  ImGui::Separator();
  ImGui::SetNextItemWidth(-1);

  ImGui::Text("Series:");
  {
    ImGui::Indent();
    ImGui::BeginChild("series list",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    const auto series_meta = get_series_json_meta();
    auto& series_json = data_json["series"];
    ImGui::SeparatorText("");
    for(auto id_counter = 0; auto& var_def : series_json.array_range()) {
      auto& var_name_json = var_def.at("variable");
      auto var_name = var_name_json.as_string();

      ImGui::PushID(id_counter++);

      ImGui::Text("Variable:");
      ImGui::SameLine();
      if(ImGui::InputText("##variable", &var_name)) {
        var_name_json = var_name;
      }

      ImGui::Text("Definition:");
      render_series_method(series_meta, var_def.at("definition"));

      ImGui::SeparatorText("");
      ImGui::PopID();
    }

    if(ImGui::Button("Add Series")) {
      auto& series_array = data_json["series"];
      auto new_series = ojson::object();
      new_series.insert_or_assign(
       "variable", std::format("new_var_{}", series_array.size() + 1));
      new_series.insert_or_assign("definition",
                                  series_meta.at("CLOSE").default_value);
      series_array.push_back(new_series);
    }
    ImGui::Indent();
    ImGui::EndChild();
  }

  ImGui::EndChild();

  if(ImGui::Button("Create Strategy")) {
  }

  ImGui::SameLine();
  if(ImGui::Button("Cancel")) {
    current_page_ = Page::List;
  }

  ImGui::EndGroup();
}

} // namespace pludux::apps
