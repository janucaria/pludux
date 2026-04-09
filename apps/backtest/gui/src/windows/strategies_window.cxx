module;
#include <array>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#ifdef __EMSCRIPTEN__
#include <cstdlib>

#include <emscripten.h>
#include <emscripten/val.h>
#else
#include <nfd.hpp>
#endif

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

export module pludux.apps.backtest:windows.strategies_window;

import pludux.backtest;
import :window_context;

namespace pludux::apps {

using SelectOutputMethod = pludux::SelectOutputMethod<AnySeriesMethod>;
using SeriesNodeMethod = pludux::SeriesNodeMethod;
using SeriesValueMethod = pludux::SeriesValueMethod;
using BbMethod = pludux::BbMethod<AnySeriesMethod>;
using KcMethod = pludux::KcMethod<AnySeriesMethod>;
using StochMethod = pludux::StochMethod;
using StochRsiMethod = pludux::StochRsiMethod<AnySeriesMethod>;
using SmaMethod = pludux::SmaMethod<AnySeriesMethod>;
using CachedResultsEmaMethod = pludux::CachedResultsEmaMethod<AnySeriesMethod>;
using WmaMethod = pludux::WmaMethod<AnySeriesMethod>;
using HmaMethod = pludux::HmaMethod<AnySeriesMethod>;
using RsiMethod = pludux::RsiMethod<AnySeriesMethod>;
using RocMethod = pludux::RocMethod<AnySeriesMethod>;
using RvolMethod = pludux::RvolMethod;
using MacdMethod = pludux::MacdMethod<AnySeriesMethod>;
using AtrMethod = pludux::AtrMethod;
using ValueMethod = pludux::ValueMethod;
using LookbackMethod = pludux::LookbackMethod<AnySeriesMethod>;
using ChangeMethod = pludux::ChangeMethod<AnySeriesMethod>;
using DataMethod = pludux::DataMethod;
using SqrtAnyMethod = pludux::SqrtMethod<AnySeriesMethod>;
using StddevAnyMethod = pludux::StddevMethod<AnySeriesMethod>;

using AddMethod = pludux::AddMethod<AnySeriesMethod, AnySeriesMethod>;
using SubtractMethod = pludux::SubtractMethod<AnySeriesMethod, AnySeriesMethod>;
using MultiplyMethod = pludux::MultiplyMethod<AnySeriesMethod, AnySeriesMethod>;
using DivideMethod = pludux::DivideMethod<AnySeriesMethod, AnySeriesMethod>;
using NegateMethod = pludux::NegateMethod<AnySeriesMethod>;
using PercentageMethod = pludux::PercentageMethod<AnySeriesMethod>;
using AbsDiffMethod = pludux::AbsDiffMethod<AnySeriesMethod, AnySeriesMethod>;

using pludux::backtest::AnyPlotMethod;
using pludux::backtest::AnyPlotSourceMethod;
using pludux::backtest::HLinePlotMethod;
using LinePlotMethod = pludux::backtest::LinePlotMethod<AnyPlotSourceMethod>;
using HistogramPlotMethod =
 pludux::backtest::HistogramPlotMethod<AnyPlotSourceMethod>;
using pludux::backtest::ConstantPlotSourceMethod;
using pludux::backtest::SeriesPlotSourceMethod;

auto get_default_series_method(const std::string& series_id) -> AnySeriesMethod
{
  if(series_id == "OPEN") {
    return OpenMethod{};
  } else if(series_id == "CLOSE") {
    return CloseMethod{};
  } else if(series_id == "HIGH") {
    return HighMethod{};
  } else if(series_id == "LOW") {
    return LowMethod{};
  } else if(series_id == "VOLUME") {
    return VolumeMethod{};
  } else if(series_id == "CHANGE") {
    return ChangeMethod{CloseMethod{}};
  } else if(series_id == "DATA") {
    return DataMethod{};
  } else if(series_id == "SMA") {
    return SmaMethod{CloseMethod{}, 14};
  } else if(series_id == "EMA") {
    return CachedResultsEmaMethod{CloseMethod{}, 14};
  } else if(series_id == "WMA") {
    return WmaMethod{CloseMethod{}, 14};
  } else if(series_id == "HMA") {
    return HmaMethod{CloseMethod{}, 14};
  } else if(series_id == "RSI") {
    return RsiMethod{CloseMethod{}, 14};
  } else if(series_id == "MACD") {
    return MacdMethod{CloseMethod{}, 12, 26, 9};
  } else if(series_id == "ATR") {
    return AtrMethod{14};
  } else if(series_id == "STDDEV") {
    return StddevAnyMethod{CloseMethod{}, 14};
  } else if(series_id == "BB") {
    return BbMethod{MaMethodType::Sma, CloseMethod{}, 20, 2.0};
  } else if(series_id == "KC") {
    return KcMethod{
     CloseMethod{}, MaMethodType::Ema, 20, KcBandMethodType::Atr, 14, 1.5};
  } else if(series_id == "STOCH") {
    return StochMethod{14, 3, 3};
  } else if(series_id == "STOCH_RSI") {
    return StochRsiMethod{CloseMethod{}, 14, 14, 3, 3};
  } else if(series_id == "SERIES_REFERENCE") {
    return SeriesNodeMethod{""};
  } else if(series_id == "SERIES_RESULT") {
    return SeriesValueMethod{""};
  } else if(series_id == "VALUE") {
    return ValueMethod{0.0};
  } else if(series_id == "LOOKBACK") {
    return LookbackMethod{CloseMethod{}, 1};
  } else if(series_id == "ADD") {
    return AddMethod{CloseMethod{}, CloseMethod{}};
  } else if(series_id == "SUBTRACT") {
    return SubtractMethod{CloseMethod{}, CloseMethod{}};
  } else if(series_id == "MULTIPLY") {
    return MultiplyMethod{CloseMethod{}, CloseMethod{}};
  } else if(series_id == "DIVIDE") {
    return DivideMethod{CloseMethod{}, CloseMethod{}};
  } else if(series_id == "NEGATE") {
    return NegateMethod{CloseMethod{}};
  } else if(series_id == "SQRT") {
    return SqrtAnyMethod{CloseMethod{}};
  } else if(series_id == "PERCENTAGE") {
    return PercentageMethod{CloseMethod{}, 100.0};
  } else if(series_id == "ABS_DIFF") {
    return AbsDiffMethod{CloseMethod{}, CloseMethod{}};
  } else if(series_id == "SELECT_OUTPUT") {
    return SelectOutputMethod{CloseMethod{}, SeriesOutput::MiddleBand};
  }

  throw std::invalid_argument{
   std::format("Unknown series method id: {}", series_id)};
}

auto get_series_method_id(const AnySeriesMethod& method) -> std::string
{
  if(series_method_cast<SelectOutputMethod>(method)) {
    return "SELECT_OUTPUT";
  } else if(series_method_cast<SeriesNodeMethod>(method)) {
    return "SERIES_REFERENCE";
  } else if(series_method_cast<SeriesValueMethod>(method)) {
    return "SERIES_RESULT";
  } else if(series_method_cast<CloseMethod>(method)) {
    return "CLOSE";
  } else if(series_method_cast<OpenMethod>(method)) {
    return "OPEN";
  } else if(series_method_cast<HighMethod>(method)) {
    return "HIGH";
  } else if(series_method_cast<LowMethod>(method)) {
    return "LOW";
  } else if(series_method_cast<VolumeMethod>(method)) {
    return "VOLUME";
  } else if(series_method_cast<ChangeMethod>(method)) {
    return "CHANGE";
  } else if(series_method_cast<DataMethod>(method)) {
    return "DATA";
  } else if(series_method_cast<ValueMethod>(method)) {
    return "VALUE";
  } else if(series_method_cast<StddevAnyMethod>(method)) {
    return "STDDEV";
  } else if(series_method_cast<BbMethod>(method)) {
    return "BB";
  } else if(series_method_cast<KcMethod>(method)) {
    return "KC";
  } else if(series_method_cast<SmaMethod>(method)) {
    return "SMA";
  } else if(series_method_cast<CachedResultsEmaMethod>(method)) {
    return "EMA";
  } else if(series_method_cast<WmaMethod>(method)) {
    return "WMA";
  } else if(series_method_cast<HmaMethod>(method)) {
    return "HMA";
  } else if(series_method_cast<RsiMethod>(method)) {
    return "RSI";
  } else if(series_method_cast<MacdMethod>(method)) {
    return "MACD";
  } else if(series_method_cast<AtrMethod>(method)) {
    return "ATR";
  } else if(series_method_cast<StochMethod>(method)) {
    return "STOCH";
  } else if(series_method_cast<StochRsiMethod>(method)) {
    return "STOCH_RSI";
  } else if(series_method_cast<AddMethod>(method)) {
    return "ADD";
  } else if(series_method_cast<SubtractMethod>(method)) {
    return "SUBTRACT";
  } else if(series_method_cast<MultiplyMethod>(method)) {
    return "MULTIPLY";
  } else if(series_method_cast<DivideMethod>(method)) {
    return "DIVIDE";
  } else if(series_method_cast<NegateMethod>(method)) {
    return "NEGATE";
  } else if(series_method_cast<SqrtAnyMethod>(method)) {
    return "SQRT";
  } else if(series_method_cast<PercentageMethod>(method)) {
    return "PERCENTAGE";
  } else if(series_method_cast<AbsDiffMethod>(method)) {
    return "ABS_DIFF";
  } else if(series_method_cast<LookbackMethod>(method)) {
    return "LOOKBACK";
  }

  return "UNKNOWN";
}

auto get_series_method_title(const std::string& series_id) -> std::string
{
  if(series_id == "OPEN") {
    return "Open Price";
  } else if(series_id == "CLOSE") {
    return "Close Price";
  } else if(series_id == "HIGH") {
    return "High Price";
  } else if(series_id == "LOW") {
    return "Low Price";
  } else if(series_id == "VOLUME") {
    return "Volume";
  } else if(series_id == "CHANGE") {
    return "Change";
  } else if(series_id == "DATA") {
    return "Data";
  } else if(series_id == "SMA") {
    return "Simple Moving Average (SMA)";
  } else if(series_id == "EMA") {
    return "Exponential Moving Average (EMA)";
  } else if(series_id == "WMA") {
    return "Weighted Moving Average (WMA)";
  } else if(series_id == "HMA") {
    return "Hull Moving Average (HMA)";
  } else if(series_id == "RSI") {
    return "Relative Strength Index (RSI)";
  } else if(series_id == "MACD") {
    return "Moving Average Convergence Divergence (MACD)";
  } else if(series_id == "ATR") {
    return "Average True Range (ATR)";
  } else if(series_id == "STDDEV") {
    return "Standard Deviation (STDDEV)";
  } else if(series_id == "BB") {
    return "Bollinger Bands";
  } else if(series_id == "KC") {
    return "Keltner Channel (KC)";
  } else if(series_id == "STOCH") {
    return "Stochastic Oscillator";
  } else if(series_id == "STOCH_RSI") {
    return "Stochastic RSI";
  } else if(series_id == "SERIES_REFERENCE") {
    return "Series Node";
  } else if(series_id == "SERIES_RESULT") {
    return "Series Value";
  } else if(series_id == "VALUE") {
    return "Value";
  } else if(series_id == "LOOKBACK") {
    return "Lookback";
  } else if(series_id == "ADD") {
    return "Addition";
  } else if(series_id == "SUBTRACT") {
    return "Subtraction";
  } else if(series_id == "MULTIPLY") {
    return "Multiplication";
  } else if(series_id == "DIVIDE") {
    return "Division";
  } else if(series_id == "NEGATE") {
    return "Negation";
  } else if(series_id == "SQRT") {
    return "Square Root";
  } else if(series_id == "PERCENTAGE") {
    return "Percentage";
  } else if(series_id == "ABS_DIFF") {
    return "Absolute Difference";
  } else if(series_id == "SELECT_OUTPUT") {
    return "Select Output";
  }

  return "Unknown";
}

auto get_condition_method_id(const AnyConditionMethod& method) -> std::string
{
  if(condition_method_cast<AllOfMethod>(method)) {
    return "ALL_OF";
  } else if(condition_method_cast<AnyOfMethod>(method)) {
    return "ANY_OF";
  } else if(condition_method_cast<AlwaysMethod>(method)) {
    return "ALWAYS";
  } else if(condition_method_cast<NeverMethod>(method)) {
    return "NEVER";
  } else if(condition_method_cast<LessThanMethod>(method)) {
    return "LESS_THAN";
  } else if(condition_method_cast<GreaterThanMethod>(method)) {
    return "GREATER_THAN";
  } else if(condition_method_cast<LessEqualMethod>(method)) {
    return "LESS_EQUAL";
  } else if(condition_method_cast<GreaterEqualMethod>(method)) {
    return "GREATER_EQUAL";
  } else if(condition_method_cast<EqualMethod>(method)) {
    return "EQUAL";
  } else if(condition_method_cast<NotEqualMethod>(method)) {
    return "NOT_EQUAL";
  } else if(condition_method_cast<CrossoverMethod>(method)) {
    return "CROSSOVER";
  } else if(condition_method_cast<CrossunderMethod>(method)) {
    return "CROSSUNDER";
  } else if(condition_method_cast<NotMethod>(method)) {
    return "NOT";
  } else if(condition_method_cast<AndMethod>(method)) {
    return "AND";
  } else if(condition_method_cast<OrMethod>(method)) {
    return "OR";
  } else if(condition_method_cast<XorMethod>(method)) {
    return "XOR";
  }

  return "UNKNOWN";
}

auto get_condition_method_title(const std::string& condition_id) -> std::string
{
  if(condition_id == "ALL_OF") {
    return "All Of";
  } else if(condition_id == "ANY_OF") {
    return "Any Of";
  } else if(condition_id == "ALWAYS") {
    return "Always";
  } else if(condition_id == "NEVER") {
    return "Never";
  } else if(condition_id == "LESS_THAN") {
    return "Less Than";
  } else if(condition_id == "GREATER_THAN") {
    return "Greater Than";
  } else if(condition_id == "LESS_EQUAL") {
    return "Less Equal";
  } else if(condition_id == "GREATER_EQUAL") {
    return "Greater Equal";
  } else if(condition_id == "EQUAL") {
    return "Equal";
  } else if(condition_id == "NOT_EQUAL") {
    return "Not Equal";
  } else if(condition_id == "CROSSOVER") {
    return "Crossover";
  } else if(condition_id == "CROSSUNDER") {
    return "Crossunder";
  } else if(condition_id == "NOT") {
    return "Not";
  } else if(condition_id == "AND") {
    return "And";
  } else if(condition_id == "OR") {
    return "Or";
  } else if(condition_id == "XOR") {
    return "Xor";
  }

  return "Unknown";
}

auto get_default_condition_method(const std::string& condition_id)
 -> AnyConditionMethod
{
  if(condition_id == "ALL_OF") {
    return AllOfMethod{};
  } else if(condition_id == "ANY_OF") {
    return AnyOfMethod{};
  } else if(condition_id == "ALWAYS") {
    return AlwaysMethod{};
  } else if(condition_id == "NEVER") {
    return NeverMethod{};
  } else if(condition_id == "LESS_THAN") {
    return LessThanMethod{CloseMethod{}, CloseMethod{}};
  } else if(condition_id == "GREATER_THAN") {
    return GreaterThanMethod{CloseMethod{}, CloseMethod{}};
  } else if(condition_id == "LESS_EQUAL") {
    return LessEqualMethod{CloseMethod{}, CloseMethod{}};
  } else if(condition_id == "GREATER_EQUAL") {
    return GreaterEqualMethod{CloseMethod{}, CloseMethod{}};
  } else if(condition_id == "EQUAL") {
    return EqualMethod{CloseMethod{}, CloseMethod{}};
  } else if(condition_id == "NOT_EQUAL") {
    return NotEqualMethod{CloseMethod{}, CloseMethod{}};
  } else if(condition_id == "CROSSOVER") {
    return CrossoverMethod{CloseMethod{}, CloseMethod{}};
  } else if(condition_id == "CROSSUNDER") {
    return CrossunderMethod{CloseMethod{}, CloseMethod{}};
  } else if(condition_id == "NOT") {
    return NotMethod{NeverMethod{}};
  } else if(condition_id == "AND") {
    return AndMethod{NeverMethod{}, NeverMethod{}};
  } else if(condition_id == "OR") {
    return OrMethod{NeverMethod{}, NeverMethod{}};
  } else if(condition_id == "XOR") {
    return XorMethod{NeverMethod{}, NeverMethod{}};
  }

  throw std::invalid_argument{
   std::format("Unknown condition method id: {}", condition_id)};
}

auto get_plot_method_id(const AnyPlotMethod& method) -> std::string
{
  if(plot_method_cast<HLinePlotMethod>(method)) {
    return "HLINE";
  }

  if(plot_method_cast<LinePlotMethod>(method)) {
    return "LINE";
  }

  if(plot_method_cast<HistogramPlotMethod>(method)) {
    return "HISTOGRAM";
  }

  return "UNKNOWN";
}

auto get_plot_method_title(const std::string& plot_id) -> std::string
{
  if(plot_id == "LINE") {
    return "Line";
  } else if(plot_id == "HLINE") {
    return "Horizontal Line";
  } else if(plot_id == "HISTOGRAM") {
    return "Histogram";
  }

  return "Unknown";
}

auto get_default_plot_method(const std::string& plot_id) -> AnyPlotMethod
{
  if(plot_id == "LINE") {
    return LinePlotMethod{ConstantPlotSourceMethod{0.0}, 0xFFFFFFFF};
  }

  if(plot_id == "HLINE") {
    return HLinePlotMethod{0.0, 0xFFFFFFFF};
  }

  if(plot_id == "HISTOGRAM") {
    return HistogramPlotMethod{ConstantPlotSourceMethod{0.0}, 0xFFFFFFFF};
  }

  throw std::invalid_argument{
   std::format("Unknown plot method id: {}", plot_id)};
}

auto get_plot_source_method_id(const AnyPlotSourceMethod& method) -> std::string
{
  if(plot_source_method_cast<ConstantPlotSourceMethod>(method)) {
    return "CONSTANT";
  } else if(plot_source_method_cast<SeriesPlotSourceMethod>(method)) {
    return "SERIES";
  }

  return "UNKNOWN";
}

auto get_plot_source_method_title(const std::string& plot_source_id)
 -> std::string
{
  if(plot_source_id == "CONSTANT") {
    return "Constant Value";
  }

  if(plot_source_id == "SERIES") {
    return "Series Name";
  }

  return "Unknown";
}

auto get_default_plot_source_method(const std::string& plot_source_id)
 -> AnyPlotSourceMethod
{
  if(plot_source_id == "CONSTANT") {
    return ConstantPlotSourceMethod{0.0};
  }

  if(plot_source_id == "SERIES") {
    return SeriesPlotSourceMethod{""};
  }

  throw std::invalid_argument{
   std::format("Unknown plot source method id: {}", plot_source_id)};
}

export class StrategiesWindow {
public:
  StrategiesWindow()
  {
  }

  void render(this auto& self, WindowContext& context)
  {
    ImGui::Begin("Strategies", nullptr);

    switch(self.current_page_) {
    case Page::AddNew:
      self.render_add_new_strategy(context);
      break;
    case Page::Edit:
      self.render_edit_strategy(context);
      break;
    case Page::List:
    default:
      self.render_list_strategies(context);
      break;
    }

    ImGui::End();
  }

private:
  enum class Page { List, AddNew, Edit } current_page_{Page::List};

  std::shared_ptr<backtest::Strategy> selected_strategy_ptr_;
  std::shared_ptr<backtest::Strategy> editing_strategy_ptr_;

  std::vector<std::string> available_series_names_;

  void render_list_strategies(this auto& self, WindowContext& context)
  {
    const auto& backtest = context.app_state().selected_backtest();
    auto& strategies = context.strategies();

    ImGui::BeginGroup();
    ImGui::BeginChild(
     "item view",
     ImVec2(
      0,
      -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us

    if(!strategies.empty()) {
      for(auto i = 0; i < strategies.size(); ++i) {
        const auto& strategy_ptr = strategies[i];
        const auto& strategy_name = strategy_ptr->name();

        ImGui::PushID(i);

        ImGui::SetNextItemAllowOverlap();
        auto is_selected = backtest && backtest->strategy_ptr() == strategy_ptr;
        ImGui::Selectable(strategy_name.c_str(), &is_selected);

        ImGui::SameLine();

        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 150);

        if(ImGui::Button("Export")) {
          auto serialized_strategy =
           stringify_backtest_strategy(*strategy_ptr).to_string();

#ifdef __EMSCRIPTEN__
          const auto file_name = "pludux-strategy-" + strategy_name + ".json";
          const auto& file_content = serialized_strategy;

          EM_ASM(
           {
             var fileSave = document.createElement('a');
             fileSave.download = Module.UTF8ToString($0);
             fileSave.style.display = 'none';
             var data = new Blob([Module.UTF8ToString($1)], {
               type:
                 'application/json'
             });
             fileSave.href = URL.createObjectURL(data);
             fileSave.click();
             URL.revokeObjectURL(fileSave.href);
           },
           file_name.c_str(),
           file_content.c_str());

#else
          auto nfd_guard = NFD::Guard{};
          auto out_path = NFD::UniquePath{};

          const auto filter_item =
           std::array<nfdfilteritem_t, 1>{{"JSON Files", "json"}};

          auto result =
           NFD::SaveDialog(out_path, filter_item.data(), filter_item.size());

          if(result == NFD_OKAY) {
            const auto saved_path = std::string(out_path.get());
            context.push_action(
             [saved_path, serialized_strategy](ApplicationState& app_state) {
               auto out_stream = std::ofstream{saved_path};

               if(!out_stream.is_open()) {
                 const auto error_message =
                  std::format("Failed to open '{}' for writing.", saved_path);
                 throw std::runtime_error(error_message);
               }

               out_stream << serialized_strategy;
             });
          } else if(result == NFD_CANCEL) {
            // User cancelled the save dialog
          } else {
            const auto error_message =
             std::format("Error '{}': {}", "Export", NFD::GetError());
            throw std::runtime_error(error_message);
          }
#endif
        }
        ImGui::SameLine();
        if(ImGui::Button("Edit")) {
          self.selected_strategy_ptr_ = strategy_ptr;
          self.editing_strategy_ptr_ =
           std::make_shared<backtest::Strategy>(*self.selected_strategy_ptr_);
          self.current_page_ = Page::Edit;
        }
        ImGui::SameLine();
        if(ImGui::Button("Delete")) {
          context.push_action([i](ApplicationState& app_state) {
            app_state.remove_strategy_at_index(i);
          });
        }

        ImGui::PopID();
      }
    }

    ImGui::EndChild();
    if(ImGui::Button("Add New Strategy")) {
      self.current_page_ = Page::AddNew;

      self.selected_strategy_ptr_ = nullptr;

      self.editing_strategy_ptr_ = std::make_shared<backtest::Strategy>();
    }

    ImGui::SameLine();

    if(ImGui::Button("Import Strategies")) {
#ifdef __EMSCRIPTEN__

      using JsOnOpenedFileContentReady =
       std::function<void(const std::string&, const std::string&, void*)>;

      static const auto callback =
       JsOnOpenedFileContentReady{[](const std::string& file_name,
                                     const std::string& file_data,
                                     void* user_data) {
         auto& context = *reinterpret_cast<WindowContext*>(user_data);

         auto action = LoadStrategyJsonAction{file_name, file_data};
         context.push_action(std::move(action));
       }};

      EM_ASM(
       {
         var fileSelector = document.createElement('input');
         fileSelector.type = 'file';
         fileSelector.multiple = true;
         fileSelector.accept = '.json';
         fileSelector.onchange = function(event)
         {
           var files = event.target.files;
           for(var i = 0; i < files.length; i++) {
             var file = files[i];

             var reader = new FileReader();
             reader.onload = function(event)
             {
               var reader = event.target;
               var fileName = reader.onload.prototype.fileName;
               var data = reader.result;
               var decoder = new TextDecoder('utf-8');
               var decodedData = decoder.decode(data);

               // transfer the data to the C++ side
               var name_ptr = Module.stringToNewUTF8(fileName);
               var data_ptr = Module.stringToNewUTF8(decodedData);

               // call the C++ function
               Module._pludux_apps_backtest_js_opened_file_content_ready(
                name_ptr, data_ptr, $0, $1);
             };
             reader.onload.prototype.fileName = file.name;

             reader.readAsArrayBuffer(file);
           }
         };
         fileSelector.click();
       },
       &callback,
       &context);
#else
      auto nfd_guard = NFD::Guard{};
      auto in_paths = NFD::UniquePathSet{};

      const auto filter_item =
       std::array<nfdfilteritem_t, 1>{{"JSON Files", "json"}};

      auto result = NFD::OpenDialogMultiple(
       in_paths, filter_item.data(), filter_item.size());

      try {
        if(result == NFD_OKAY) {
          auto paths_count = nfdpathsetsize_t{};

          result = NFD::PathSet::Count(in_paths, paths_count);
          if(result == NFD_ERROR) {
            const auto error_message =
             std::format("Error '{}': {}", "Import", NFD::GetError());
            throw std::runtime_error(error_message);
          }

          for(nfdpathsetsize_t i = 0; i < paths_count; ++i) {
            auto in_path = NFD::UniquePathSetPath{};
            result = NFD::PathSet::GetPath(in_paths, i, in_path);

            if(result == NFD_ERROR) {
              const auto error_message =
               std::format("Error '{}': {}", "Import", NFD::GetError());
              throw std::runtime_error(error_message);
            } else {
              const auto selected_path = std::string(in_path.get());
              context.push_action(LoadStrategyJsonAction{selected_path});
            }
          }

        } else if(result == NFD_CANCEL) {
          // User cancelled the open dialog
        } else {
          const auto error_message =
           std::format("Error '{}': {}", "Import", NFD::GetError());
          throw std::runtime_error(error_message);
        }
      } catch(const std::exception& ex) {
        const auto error_message = std::string(ex.what());
        context.push_action([error_message](ApplicationState& app_state) {
          app_state.alert(error_message);
        });
      }
#endif
    }

    ImGui::EndGroup();
  }

  void render_add_new_strategy(this auto& self, WindowContext& context)
  {
    const auto& strategies = context.strategies();

    ImGui::BeginGroup();
    ImGui::BeginChild("add_new_strategy",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    ImGui::Text("Add New Strategy");
    ImGui::Separator();
    ImGui::SetNextItemWidth(-1);

    {
      self.edit_strategy_form(context);
    }

    ImGui::EndChild();

    if(ImGui::Button("Create")) {
      self.submit_strategy_changes(context);
      self.reset();
    }

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.reset();
    }

    ImGui::EndGroup();
  }

  void render_edit_strategy(this auto& self, WindowContext& context)
  {
    const auto& strategies = context.strategies();

    ImGui::BeginGroup();
    ImGui::BeginChild("edit_strategy",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    ImGui::Text("Edit Strategy");
    ImGui::Separator();
    ImGui::SetNextItemWidth(-1);

    {
      self.edit_strategy_form(context);
    }

    ImGui::EndChild();

    const auto same_strategy =
     *(self.selected_strategy_ptr_) == *(self.editing_strategy_ptr_);

    if(ImGui::Button("OK")) {
      self.submit_strategy_changes(context);
      self.reset();
    }

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.reset();
    }

    ImGui::BeginDisabled(same_strategy);
    ImGui::SameLine();
    if(ImGui::Button("Apply")) {
      self.submit_strategy_changes(context);
    }
    ImGui::EndDisabled();

    ImGui::EndGroup();
  }

  void edit_strategy_form(this auto& self, WindowContext& context)
  {
    ImGui::BeginChild("edit_content",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    {
      ImGui::Text("Strategy Name:");
      ImGui::SameLine();
      auto strategy_name = self.editing_strategy_ptr_->name();
      ImGui::InputText("##strategy_name", &strategy_name);
      self.editing_strategy_ptr_->name(strategy_name);
      ImGui::Text("");
    }

    {
      ImGui::SeparatorText("Series Methods");

      auto& series_registry = self.editing_strategy_ptr_->series_registry();
      ImGui::Separator();
      auto changed_names = std::vector<std::pair<std::string, std::string>>{};
      self.available_series_names_.clear();
      for(auto id_counter = 0;
          auto& [series_name, series_method] : series_registry) {
        ImGui::PushID(id_counter++);

        ImGui::Text("Name:");
        ImGui::SameLine();
        auto input_series_name = series_name;
        ImGui::InputText("##series_name", &input_series_name);
        if(ImGui::IsItemDeactivatedAfterEdit()) {
          if(input_series_name != series_name) {
            changed_names.emplace_back(series_name, input_series_name);
          }
        }

        ImGui::Text("Method:");
        ImGui::SameLine();
        self.render_series_method(series_method, context);
        series_registry.set(series_name, series_method);

        ImGui::Separator();
        ImGui::PopID();

        self.available_series_names_.push_back(series_name);
      }

      for(auto [old_name, new_name] : changed_names) {
        // TODO: handle the errors
        [[maybe_unused]]
        const auto result = series_registry.rename(old_name, new_name);
      }

      if(ImGui::Button("Add Series")) {
        auto new_series_name =
         std::format("new_var_{}", series_registry.size() + 1);
        auto new_series_method = get_default_series_method("CLOSE");
        series_registry.set(new_series_name, new_series_method);
      }

      ImGui::Text("");
    }

    {
      ImGui::SeparatorText("Positions");

      {
        ImGui::Text("Long Position:");
        ImGui::PushID("long_position");
        ImGui::Indent();

        {
          ImGui::Text("Entry Condition:");
          ImGui::PushID("long_entry");
          const auto& any_long_entry =
           self.editing_strategy_ptr_->long_entry_filter();
          auto changed_method =
           self.render_condition_method(any_long_entry, context);
          self.editing_strategy_ptr_->long_entry_filter(
           std::move(changed_method));
          ImGui::PopID();
        }

        ImGui::Separator();

        {
          ImGui::Text("Exit Condition:");
          ImGui::PushID("long_exit");
          const auto& any_long_exit =
           self.editing_strategy_ptr_->long_exit_filter();
          auto changed_method =
           self.render_condition_method(any_long_exit, context);
          self.editing_strategy_ptr_->long_exit_filter(
           std::move(changed_method));
          ImGui::PopID();
        }

        ImGui::Unindent();
        ImGui::PopID();
      }

      ImGui::Text("");

      {
        ImGui::Text("Short Position:");
        ImGui::PushID("short_position");
        ImGui::Indent();

        {
          ImGui::Text("Entry Condition:");
          ImGui::PushID("short_entry");
          const auto& any_short_entry =
           self.editing_strategy_ptr_->short_entry_filter();
          auto changed_method =
           self.render_condition_method(any_short_entry, context);
          self.editing_strategy_ptr_->short_entry_filter(
           std::move(changed_method));
          ImGui::PopID();
        }

        ImGui::Separator();

        {
          ImGui::Text("Exit Condition:");
          ImGui::PushID("short_exit");
          const auto& any_short_exit =
           self.editing_strategy_ptr_->short_exit_filter();
          auto changed_method =
           self.render_condition_method(any_short_exit, context);
          self.editing_strategy_ptr_->short_exit_filter(
           std::move(changed_method));
          ImGui::PopID();
        }

        ImGui::Unindent();
        ImGui::PopID();
      }
      ImGui::Text("");
    }

    {
      ImGui::SeparatorText("Stop Loss");

      auto stop_loss_enabled = self.editing_strategy_ptr_->stop_loss_enabled();
      auto stop_loss_trailing_enabled =
       self.editing_strategy_ptr_->stop_loss_trailing_enabled();

      ImGui::Checkbox("Enable Stop Loss", &stop_loss_enabled);
      ImGui::Checkbox("Enable Trailing Stop Loss", &stop_loss_trailing_enabled);

      self.editing_strategy_ptr_->stop_loss_enabled(stop_loss_enabled);
      self.editing_strategy_ptr_->stop_loss_trailing_enabled(
       stop_loss_trailing_enabled);

      ImGui::Text("");
    }

    {
      ImGui::SeparatorText("Take Profit");

      auto take_profit_enabled =
       self.editing_strategy_ptr_->take_profit_enabled();
      auto take_profit_r_multiple =
       self.editing_strategy_ptr_->take_profit_r_multiple();

      ImGui::Checkbox("Enable Take Profit", &take_profit_enabled);
      ImGui::Text("R-Multiple:");
      ImGui::SameLine();
      if(ImGui::InputDouble("##take_profit_r_multiple",
                            &take_profit_r_multiple,
                            0.1,
                            1.0,
                            "%.2f")) {
        if(take_profit_r_multiple < 0.1) {
          take_profit_r_multiple = 0.1;
        }
      }

      self.editing_strategy_ptr_->take_profit_enabled(take_profit_enabled);
      self.editing_strategy_ptr_->take_profit_r_multiple(
       take_profit_r_multiple);

      ImGui::Text("");
    }

    {
      ImGui::SeparatorText("Plots");

      auto plot_groups = self.editing_strategy_ptr_->plots();
      for(auto i = 0; i < plot_groups.size(); ++i) {
        auto& plot_group = plot_groups[i];

        ImGui::PushID(i);

        ImGui::Text("Plot Name:");
        ImGui::SameLine();
        auto plot_name = plot_group.name();
        ImGui::InputText("##plot_name", &plot_name);
        plot_group.name(plot_name);

        ImGui::Text("Overlays");
        ImGui::SameLine();
        auto overlay_enabled = plot_group.is_overlay();
        ImGui::Checkbox("##overlay_enabled", &overlay_enabled);
        plot_group.is_overlay(overlay_enabled);

        ImGui::Separator();

        {
          auto plot_items = plot_group.items();
          for(auto j = 0; j < plot_items.size(); ++j) {
            auto& plot_method = plot_items[j];
            ImGui::PushID(j);

            self.render_plot_method(plot_method, context);

            if(ImGui::Button("Remove Item")) {
              plot_items.erase(plot_items.begin() + j);
              --j; // Adjust index after removal
            }

            ImGui::Separator();
            ImGui::PopID();
          }

          if(ImGui::Button("Add Item")) {
            plot_items.emplace_back(get_default_plot_method("HLINE"));
          }

          plot_group.items(plot_items);
        }

        if(ImGui::Button("Remove Plot")) {
          plot_groups.erase(plot_groups.begin() + i);
          --i; // Adjust index after removal
        }

        ImGui::Separator();
        ImGui::PopID();
      }

      if(ImGui::Button("Add Plot")) {
        plot_groups.emplace_back("New Plot");
      }

      self.editing_strategy_ptr_->plots(std::move(plot_groups));
    }

    ImGui::EndChild();
  }

  void render_series_method(this auto& self,
                            AnySeriesMethod& series_method,
                            WindowContext& context)
  {
    static const std::vector<std::string> series_ids = {"OPEN",
                                                        "CLOSE",
                                                        "HIGH",
                                                        "LOW",
                                                        "VOLUME",
                                                        "CHANGE",
                                                        "ADD",
                                                        "SUBTRACT",
                                                        "MULTIPLY",
                                                        "DIVIDE",
                                                        "NEGATE",
                                                        "SQRT",
                                                        "PERCENTAGE",
                                                        "ABS_DIFF",
                                                        "DATA",
                                                        "SMA",
                                                        "EMA",
                                                        "WMA",
                                                        "HMA",
                                                        "RSI",
                                                        "MACD",
                                                        "ATR",
                                                        "STDDEV",
                                                        "BB",
                                                        "KC",
                                                        "STOCH",
                                                        "STOCH_RSI",
                                                        "SELECT_OUTPUT",
                                                        "SERIES_REFERENCE",
                                                        "SERIES_RESULT",
                                                        "VALUE",
                                                        "LOOKBACK"};

    auto method_series_id = get_series_method_id(series_method);

    ImGui::PushID(method_series_id.c_str());
    {
      const auto combo_preview_value =
       get_series_method_title(method_series_id);
      if(ImGui::BeginCombo("##Series", combo_preview_value.c_str())) {
        static auto filter = ImGuiTextFilter{};

        if(ImGui::IsWindowAppearing()) {
          ImGui::SetKeyboardFocusHere();
          filter.Clear();
        }

        filter.Draw("##Filter", -FLT_MIN);

        for(const auto& series_id : series_ids) {
          const auto series_title = get_series_method_title(series_id);
          const bool is_selected = series_id == method_series_id;

          if(filter.PassFilter(series_title.c_str())) {
            if(ImGui::Selectable(series_title.c_str(), is_selected)) {
              if((series_id == "SERIES_REFERENCE" ||
                  series_id == "SERIES_RESULT") &&
                 self.available_series_names_.empty()) {
                const auto series_reference_method_title =
                 get_series_method_title(series_id);
                const auto error_message =
                 std::format("Cannot select '{}' when there are no available "
                             "series other than the current one.",
                             series_reference_method_title);
                context.push_action(
                 [error_message](ApplicationState& app_state) {
                   app_state.alert(error_message);
                 });
              } else {
                series_method = get_default_series_method(series_id);
              }
            }
          }
        }
        ImGui::EndCombo();
      }
    }
    {
      ImGui::Indent();
      self.render_series_method_params(series_method, context);
      ImGui::Unindent();
    }
    ImGui::PopID();
  }

  void render_series_method_params(this auto& self,
                                   AnySeriesMethod& method,
                                   WindowContext& context)
  {
    ([&]<typename... Ts>() mutable {
      ([&]() mutable -> bool {
        if(auto specific_method = series_method_cast<Ts>(method)) {
          self.render_series_method_params(*specific_method, context);
          return true;
        }

        return false;
      }() || ...);
    }.template operator()<SelectOutputMethod,
                          SeriesNodeMethod,
                          SeriesValueMethod,
                          DataMethod,
                          LookbackMethod,

                          BbMethod,
                          KcMethod,
                          StochMethod,
                          StochRsiMethod,
                          SmaMethod,
                          CachedResultsEmaMethod,
                          WmaMethod,
                          HmaMethod,
                          RsiMethod,

                          MacdMethod,
                          AtrMethod,
                          ValueMethod,
                          ChangeMethod,
                          AddMethod,
                          SubtractMethod,
                          MultiplyMethod,
                          DivideMethod,
                          PercentageMethod,
                          AbsDiffMethod,
                          NegateMethod,
                          SqrtAnyMethod,
                          StddevAnyMethod>());
  }

  void render_series_method_params(this auto& self,
                                   SelectOutputMethod& method,
                                   WindowContext& context)
  {
    {
      const auto output_options =
       std::vector<SeriesOutput>{SeriesOutput::UpperBand,
                                 SeriesOutput::MiddleBand,
                                 SeriesOutput::LowerBand,
                                 SeriesOutput::MacdLine,
                                 SeriesOutput::SignalLine,
                                 SeriesOutput::Histogram,
                                 SeriesOutput::KPercent,
                                 SeriesOutput::DPercent};

      const auto output_map = std::unordered_map<SeriesOutput, std::string>{
       {SeriesOutput::UpperBand, "Upper Band"},
       {SeriesOutput::MiddleBand, "Middle Band"},
       {SeriesOutput::LowerBand, "Lower Band"},
       {SeriesOutput::MacdLine, "MACD Line"},
       {SeriesOutput::SignalLine, "Signal Line"},
       {SeriesOutput::Histogram, "Histogram"},
       {SeriesOutput::KPercent, "%K"},
       {SeriesOutput::DPercent, "%D"}};

      ImGui::Text("Output:");
      ImGui::SameLine();
      auto output = method.output();
      {
        const auto output_str = output_map.at(output);
        if(ImGui::BeginCombo("##output", output_str.c_str())) {
          for(auto output_option : output_options) {
            const auto output_option_str = output_map.at(output_option);
            const bool is_selected = output == output_option;
            if(ImGui::Selectable(output_option_str.c_str(), is_selected)) {
              method.output(output_option);
            }
          }
          ImGui::EndCombo();
        }
      }
    }
    {
      ImGui::Text("Source:");
      ImGui::SameLine();
      auto output_source = method.source();
      self.render_series_method(output_source, context);
      method.source(std::move(output_source));
    }
  }

  void render_series_method_params(this auto& self,
                                   SeriesNodeMethod& method,
                                   WindowContext& context)
  {
    if(std::ranges::find(self.available_series_names_, method.name()) ==
       self.available_series_names_.end()) {
      const auto new_name = self.available_series_names_.empty()
                             ? ""
                             : self.available_series_names_.front();
      method.name(new_name);
    }

    ImGui::Text("Name:");
    ImGui::SameLine();

    const auto display_name = method.name();
    if(ImGui::BeginCombo("##named_series", display_name.c_str())) {
      for(const auto& name_option : self.available_series_names_) {
        ImGui::PushID(name_option.c_str());

        const bool is_selected = display_name == name_option;
        if(ImGui::Selectable(name_option.c_str(), is_selected)) {
          method.name(name_option);
        }

        ImGui::PopID();
      }
      ImGui::EndCombo();
    }
  }

  void render_series_method_params(this auto& self,
                                   SeriesValueMethod& method,
                                   WindowContext& context)
  {
    if(std::ranges::find(self.available_series_names_, method.name()) ==
       self.available_series_names_.end()) {
      const auto new_name = self.available_series_names_.empty()
                             ? ""
                             : self.available_series_names_.front();
      method.name(new_name);
    }

    ImGui::Text("Name:");
    ImGui::SameLine();

    const auto display_name = method.name();
    if(ImGui::BeginCombo("##named_series", display_name.c_str())) {
      for(const auto& name_option : self.available_series_names_) {
        ImGui::PushID(name_option.c_str());

        const bool is_selected = display_name == name_option;
        if(ImGui::Selectable(name_option.c_str(), is_selected)) {
          method.name(name_option);
        }

        ImGui::PopID();
      }
      ImGui::EndCombo();
    }
  }

  void render_series_method_params(this auto& self,
                                   BbMethod& method,
                                   WindowContext& context)
  {
    ImGui::Text("MA Type:");
    ImGui::SameLine();
    {
      const auto ma_type_options = std::vector<MaMethodType>{MaMethodType::Sma,
                                                             MaMethodType::Ema,
                                                             MaMethodType::Wma,
                                                             MaMethodType::Hma,
                                                             MaMethodType::Rma};

      const auto get_ma_type_string =
       [](MaMethodType ma_type) static -> std::string {
        switch(ma_type) {
        case MaMethodType::Sma:
          return "SMA";
        case MaMethodType::Ema:
          return "EMA";
        case MaMethodType::Wma:
          return "WMA";
        case MaMethodType::Hma:
          return "HMA";
        case MaMethodType::Rma:
          return "RMA";
        }

        std::unreachable();
      };

      const auto ma_type_str = get_ma_type_string(method.ma_type());
      if(ImGui::BeginCombo("##ma_type", ma_type_str.c_str())) {
        for(const auto& ma_type_option : ma_type_options) {
          const auto ma_type_option_str = get_ma_type_string(ma_type_option);
          const bool is_selected = ma_type_str == ma_type_option_str;
          if(ImGui::Selectable(ma_type_option_str.c_str(), is_selected)) {
            method.ma_type(ma_type_option);
          }
        }
        ImGui::EndCombo();
      }
    }

    ImGui::Text("Period:");
    ImGui::SameLine();
    auto period = static_cast<int>(method.period());
    if(ImGui::InputInt("##bb_period", &period)) {
      if(period < 1) {
        period = 1;
      }
      method.period(static_cast<std::size_t>(period));
    }

    ImGui::Text("StdDev:");
    ImGui::SameLine();
    auto stddev_multiplier = method.stddev();
    if(ImGui::InputDouble("##bb_stddev_multiplier", &stddev_multiplier)) {
      if(stddev_multiplier < 0.1) {
        stddev_multiplier = 0.1;
      }
      method.stddev(stddev_multiplier);
    }

    ImGui::Text("Source:");
    ImGui::SameLine();
    auto source = method.ma_source();
    self.render_series_method(source, context);
    method.ma_source(std::move(source));
  }

  void render_series_method_params(this auto& self,
                                   KcMethod& method,
                                   WindowContext& context)
  {
    {
      ImGui::Text("Length:");
      ImGui::SameLine();
      auto length = static_cast<int>(method.ma_period());
      if(ImGui::InputInt("##kc_length", &length)) {
        if(length < 1) {
          length = 1;
        }
        method.ma_period(static_cast<std::size_t>(length));
      }
    }
    {
      ImGui::Text("MA Type:");
      ImGui::SameLine();
      {
        const auto ma_type_options =
         std::unordered_map<MaMethodType, std::string>{
          {MaMethodType::Sma, "SMA"},
          {MaMethodType::Ema, "EMA"},
          {MaMethodType::Wma, "WMA"},
          {MaMethodType::Hma, "HMA"},
          {MaMethodType::Rma, "RMA"}};

        const auto ma_type_str = ma_type_options.at(method.ma_method_type());
        if(ImGui::BeginCombo("##kc_ma_type", ma_type_str.c_str())) {
          for(const auto& [ma_type_option, ma_type_option_str] :
              ma_type_options) {
            const bool is_selected = ma_type_str == ma_type_option_str;
            if(ImGui::Selectable(ma_type_option_str.c_str(), is_selected)) {
              method.ma_method_type(ma_type_option);
            }
          }
          ImGui::EndCombo();
        }
      }
    }
    {
      ImGui::Text("Source:");
      ImGui::SameLine();
      auto source = method.ma_source();
      self.render_series_method(source, context);
      method.ma_source(std::move(source));
    }
    {
      ImGui::Text("Band Type:");
      ImGui::SameLine();
      {
        const auto band_type_options =
         std::unordered_map<KcBandMethodType, std::string>{
          {KcBandMethodType::Atr, "ATR"},
          {KcBandMethodType::Tr, "True Range"},
          {KcBandMethodType::RangeHighLow, "Range (High-Low)"}};

        const auto band_type_str =
         band_type_options.at(method.band_method_type());
        if(ImGui::BeginCombo("##kc_band_type", band_type_str.c_str())) {
          for(const auto& [band_type_option, band_type_option_str] :
              band_type_options) {
            const bool is_selected = band_type_str == band_type_option_str;
            if(ImGui::Selectable(band_type_option_str.c_str(), is_selected)) {
              method.band_method_type(band_type_option);
            }
          }
          ImGui::EndCombo();
        }
      }
    }
    {
      ImGui::Text("ATR Length:");
      ImGui::SameLine();
      auto atr_length = static_cast<int>(method.band_atr_period());
      if(ImGui::InputInt("##kc_atr_length", &atr_length)) {
        if(atr_length < 1) {
          atr_length = 1;
        }
        method.band_atr_period(static_cast<std::size_t>(atr_length));
      }
    }
    {
      ImGui::Text("Multiplier:");
      ImGui::SameLine();
      auto multiplier = method.multiplier();
      if(ImGui::InputDouble("##kc_multiplier", &multiplier, 0.1, 1.0, "%.2f")) {
        if(multiplier < 0.1) {
          multiplier = 0.1;
        }
        method.multiplier(multiplier);
      }
    }
  }

  void render_series_method_params(this auto& self,
                                   StochMethod& method,
                                   WindowContext& context)
  {
    {
      ImGui::Text("D Period:");
      ImGui::SameLine();
      auto d_period = static_cast<int>(method.d_period());
      if(ImGui::InputInt("##stoch_d_period", &d_period)) {
        if(d_period < 1) {
          d_period = 1;
        }
        method.d_period(static_cast<std::size_t>(d_period));
      }
    }
    {
      ImGui::Text("K Period:");
      ImGui::SameLine();
      auto k_period = static_cast<int>(method.k_period());
      if(ImGui::InputInt("##stoch_k_period", &k_period)) {
        if(k_period < 1) {
          k_period = 1;
        }
        method.k_period(static_cast<std::size_t>(k_period));
      }
    }
    {
      ImGui::Text("K Smooth:");
      ImGui::SameLine();
      auto slowing_period = static_cast<int>(method.k_smooth());
      if(ImGui::InputInt("##stoch_slowing_period", &slowing_period)) {
        if(slowing_period < 1) {
          slowing_period = 1;
        }
        method.k_smooth(static_cast<std::size_t>(slowing_period));
      }
    }
  }

  void render_series_method_params(this auto& self,
                                   StochRsiMethod& method,
                                   WindowContext& context)
  {
    {
      ImGui::Text("D Period:");
      ImGui::SameLine();
      auto d_period = static_cast<int>(method.d_period());
      if(ImGui::InputInt("##stoch_rsi_d_period", &d_period)) {
        if(d_period < 1) {
          d_period = 1;
        }
        method.d_period(static_cast<std::size_t>(d_period));
      }
    }
    {
      ImGui::Text("K Period:");
      ImGui::SameLine();
      auto k_period = static_cast<int>(method.k_period());
      if(ImGui::InputInt("##stoch_rsi_k_period", &k_period)) {
        if(k_period < 1) {
          k_period = 1;
        }
        method.k_period(static_cast<std::size_t>(k_period));
      }
    }
    {
      ImGui::Text("K Smooth:");
      ImGui::SameLine();
      auto k_smooth = static_cast<int>(method.k_smooth());
      if(ImGui::InputInt("##stoch_rsi_k_smooth", &k_smooth)) {
        if(k_smooth < 1) {
          k_smooth = 1;
        }
        method.k_smooth(static_cast<std::size_t>(k_smooth));
      }
    }
    {
      ImGui::Text("RSI Period:");
      ImGui::SameLine();
      auto rsi_period = static_cast<int>(method.rsi_period());
      if(ImGui::InputInt("##stoch_rsi_rsi_period", &rsi_period)) {
        if(rsi_period < 1) {
          rsi_period = 1;
        }
        method.rsi_period(static_cast<std::size_t>(rsi_period));
      }
    }
    {
      ImGui::Text("RSI Source:");
      ImGui::SameLine();
      auto rsi_source = method.rsi_source();
      self.render_series_method(rsi_source, context);
      method.rsi_source(std::move(rsi_source));
    }
  }

  template<typename TMethodWithPeriod>
    requires std::same_as<TMethodWithPeriod, SmaMethod> ||
             std::same_as<TMethodWithPeriod, CachedResultsEmaMethod> ||
             std::same_as<TMethodWithPeriod, WmaMethod> ||
             std::same_as<TMethodWithPeriod, HmaMethod> ||
             std::same_as<TMethodWithPeriod, RsiMethod> ||
             std::same_as<TMethodWithPeriod, RocMethod> ||
             std::same_as<TMethodWithPeriod, RvolMethod> ||
             std::same_as<TMethodWithPeriod, StddevAnyMethod>
  void render_series_method_params(this auto& self,
                                   TMethodWithPeriod& method,
                                   WindowContext& context)
  {
    ImGui::Text("Period:");
    ImGui::SameLine();
    auto period = static_cast<int>(method.period());
    if(ImGui::InputInt("##period", &period)) {
      if(period < 1) {
        period = 1;
      }
      method.period(static_cast<std::size_t>(period));
    }

    ImGui::Text("Source:");
    ImGui::SameLine();
    auto source = method.source();
    self.render_series_method(source, context);
    method.source(std::move(source));
  }

  template<typename TBinaryOpMethod>
    requires std::same_as<TBinaryOpMethod, AddMethod> ||
             std::same_as<TBinaryOpMethod, SubtractMethod> ||
             std::same_as<TBinaryOpMethod, MultiplyMethod> ||
             std::same_as<TBinaryOpMethod, DivideMethod> ||
             std::same_as<TBinaryOpMethod, AbsDiffMethod>
  void render_series_method_params(this auto& self,
                                   TBinaryOpMethod& method,
                                   WindowContext& context)
  {
    {
      ImGui::PushID("left");
      ImGui::Text("Left:");
      ImGui::SameLine();
      auto left = method.left();
      self.render_series_method(left, context);
      method.left(std::move(left));
      ImGui::PopID();
    }

    {
      ImGui::PushID("right");
      ImGui::Text("Right:");
      ImGui::SameLine();
      auto right = method.right();
      self.render_series_method(right, context);
      method.right(std::move(right));
      ImGui::PopID();
    }
  }

  template<typename TUnaryOpMethod>
    requires std::same_as<TUnaryOpMethod, NegateMethod> ||
             std::same_as<TUnaryOpMethod, SqrtAnyMethod>
  void render_series_method_params(this auto& self,
                                   TUnaryOpMethod& method,
                                   WindowContext& context)
  {
    ImGui::Text("Value:");
    ImGui::SameLine();
    auto value = method.operand();
    self.render_series_method(value, context);
    method.operand(std::move(value));
  }

  void render_series_method_params(this auto& self,
                                   PercentageMethod& method,
                                   WindowContext& context)
  {
    {
      ImGui::PushID("percent");
      ImGui::Text("Percent:");
      ImGui::SameLine();
      auto percent = method.percent();
      if(ImGui::InputDouble("##percent", &percent, 0.1, 1.0, "%.2f")) {
        method.percent(std::move(percent));
      }
      ImGui::PopID();
    }

    {
      ImGui::PushID("base");
      ImGui::Text("Base:");
      ImGui::SameLine();
      auto base = method.base();
      self.render_series_method(base, context);
      method.base(std::move(base));
      ImGui::PopID();
    }
  }

  void render_series_method_params(this auto& self,
                                   ChangeMethod& method,
                                   WindowContext& context)
  {
    ImGui::Text("Source:");
    ImGui::SameLine();
    auto source = method.source();
    self.render_series_method(source, context);
    method.source(std::move(source));
  }

  void render_series_method_params(this auto& self,
                                   DataMethod& method,
                                   WindowContext& context)
  {
    ImGui::Text("Field:");
    ImGui::SameLine();
    auto field = method.field();
    if(ImGui::InputText("##field", &field)) {
      method.field(field);
    }
  }

  void render_series_method_params(this auto& self,
                                   MacdMethod& method,
                                   WindowContext& context)
  {
    {
      ImGui::Text("Fast Period:");
      ImGui::SameLine();
      auto fast_period = static_cast<int>(method.fast_period());
      if(ImGui::InputInt("##macd_fast_period", &fast_period)) {
        if(fast_period < 1) {
          fast_period = 1;
        }
        method.fast_period(static_cast<std::size_t>(fast_period));
      }
    }
    {
      ImGui::Text("Slow Period:");
      ImGui::SameLine();
      auto slow_period = static_cast<int>(method.slow_period());
      if(ImGui::InputInt("##macd_slow_period", &slow_period)) {
        if(slow_period < 1) {
          slow_period = 1;
        }
        method.slow_period(static_cast<std::size_t>(slow_period));
      }
    }
    {
      ImGui::Text("Signal Period:");
      ImGui::SameLine();
      auto signal_period = static_cast<int>(method.signal_period());
      if(ImGui::InputInt("##macd_signal_period", &signal_period)) {
        if(signal_period < 1) {
          signal_period = 1;
        }
        method.signal_period(static_cast<std::size_t>(signal_period));
      }
    }
    {
      ImGui::Text("Source:");
      ImGui::SameLine();
      auto source = method.source();
      self.render_series_method(source, context);
      method.source(std::move(source));
    }
  }

  void render_series_method_params(this auto& self,
                                   AtrMethod& method,
                                   WindowContext& context)
  {
    ImGui::Text("Period:");
    ImGui::SameLine();
    auto period = static_cast<int>(method.period());
    if(ImGui::InputInt("##atr_period", &period)) {
      if(period < 1) {
        period = 1;
      }
      method.period(static_cast<std::size_t>(period));
    }

    ImGui::Text("Smoothing:");
    ImGui::SameLine();

    const auto ma_types = std::unordered_map<MaMethodType, std::string>{
     {MaMethodType::Sma, "SMA"},
     {MaMethodType::Ema, "EMA"},
     {MaMethodType::Wma, "WMA"},
     {MaMethodType::Hma, "HMA"},
     {MaMethodType::Rma, "RMA"},
    };

    const auto current_ma_title = ma_types.at(method.ma_smoothing_type());
    if(ImGui::BeginCombo("##atr_smoothing_type", current_ma_title.c_str())) {
      for(const auto& [ma_type, ma_title] : ma_types) {
        const bool is_selected = method.ma_smoothing_type() == ma_type;
        if(ImGui::Selectable(ma_title.c_str(), is_selected)) {
          method.ma_smoothing_type(ma_type);
        }
      }
      ImGui::EndCombo();
    }
  }

  void render_series_method_params(this auto& self,
                                   ValueMethod& method,
                                   WindowContext& context)
  {
    ImGui::Text("Value:");
    ImGui::SameLine();
    auto value = method.value();
    if(ImGui::InputDouble("##value", &value, 0.1, 1.0, "%.2f")) {
      method.value(value);
    }
  }

  void render_series_method_params(this auto& self,
                                   LookbackMethod& method,
                                   WindowContext& context)
  {
    ImGui::Text("Periods:");
    ImGui::SameLine();
    auto periods = static_cast<int>(method.period());
    if(ImGui::InputInt("##lookback_periods", &periods)) {
      if(periods < 1) {
        periods = 1;
      }
      method.period(static_cast<std::size_t>(periods));
    }
  }

  auto draw_condition_method_combo(this auto& self,
                                   const AnyConditionMethod& condition)
   -> std::string
  {
    static const auto condition_ids = std::vector<std::string>{"EQUAL",
                                                               "NOT_EQUAL",
                                                               "GREATER_THAN",
                                                               "LESS_THAN",
                                                               "GREATER_EQUAL",
                                                               "LESS_EQUAL",
                                                               "CROSSOVER",
                                                               "CROSSUNDER",
                                                               "ALL_OF",
                                                               "ANY_OF",
                                                               "NOT",
                                                               "AND",
                                                               "OR",
                                                               "XOR",
                                                               "ALWAYS",
                                                               "NEVER"};

    auto result_condition_id = get_condition_method_id(condition);

    ImGui::PushID(result_condition_id.c_str());
    {
      const auto combo_preview_value =
       get_condition_method_title(result_condition_id);
      if(ImGui::BeginCombo("##Conditions", combo_preview_value.c_str())) {
        static auto filter = ImGuiTextFilter{};

        if(ImGui::IsWindowAppearing()) {
          ImGui::SetKeyboardFocusHere();
          filter.Clear();
        }

        filter.Draw("##Filter", -FLT_MIN);

        for(const auto& condition_id : condition_ids) {
          const bool is_selected = condition_id == result_condition_id;
          const auto condition_title = get_condition_method_title(condition_id);

          if(filter.PassFilter(condition_title.c_str())) {
            if(ImGui::Selectable(condition_title.c_str(), is_selected)) {
              result_condition_id = condition_id;
            }
          }
        }
        ImGui::EndCombo();
      }
    }

    ImGui::PopID();

    return result_condition_id;
  }

  auto make_condition_method_from_other(this auto& self,
                                        const std::string& condition_id,
                                        auto other_condition)
   -> AnyConditionMethod
  {
    const auto get_condition_series_params =
     []<typename TMethod>(
      const TMethod& method) -> std::pair<AnySeriesMethod, AnySeriesMethod> {
      if constexpr(requires {
                     {
                       method.signal()
                     } -> std::convertible_to<AnySeriesMethod>;
                     {
                       method.reference()
                     } -> std::convertible_to<AnySeriesMethod>;
                   }) {
        return {method.signal(), method.reference()};
      } else if constexpr(requires {
                            {
                              method.target()
                            } -> std::convertible_to<AnySeriesMethod>;
                            {
                              method.threshold()
                            } -> std::convertible_to<AnySeriesMethod>;
                          }) {
        return {method.target(), method.threshold()};
      } else {
        return {CloseMethod{}, CloseMethod{}};
      }
    };

    if(condition_id == "EQUAL") {
      auto [lhs_series_param, rhs_series_param] =
       get_condition_series_params(other_condition);
      return EqualMethod{std::move(lhs_series_param),
                         std::move(rhs_series_param)};
    }

    if(condition_id == "NOT_EQUAL") {
      auto [lhs_series_param, rhs_series_param] =
       get_condition_series_params(other_condition);
      return NotEqualMethod{std::move(lhs_series_param),
                            std::move(rhs_series_param)};
    }

    if(condition_id == "GREATER_THAN") {
      auto [lhs_series_param, rhs_series_param] =
       get_condition_series_params(other_condition);
      return GreaterThanMethod{std::move(lhs_series_param),
                               std::move(rhs_series_param)};
    }

    if(condition_id == "LESS_THAN") {
      auto [lhs_series_param, rhs_series_param] =
       get_condition_series_params(other_condition);
      return LessThanMethod{std::move(lhs_series_param),
                            std::move(rhs_series_param)};
    }

    if(condition_id == "GREATER_EQUAL") {
      auto [lhs_series_param, rhs_series_param] =
       get_condition_series_params(other_condition);
      return GreaterEqualMethod{std::move(lhs_series_param),
                                std::move(rhs_series_param)};
    }

    if(condition_id == "LESS_EQUAL") {
      auto [lhs_series_param, rhs_series_param] =
       get_condition_series_params(other_condition);
      return LessEqualMethod{std::move(lhs_series_param),
                             std::move(rhs_series_param)};
    }

    if(condition_id == "CROSSOVER") {
      auto [lhs_series_param, rhs_series_param] =
       get_condition_series_params(other_condition);
      return CrossoverMethod{std::move(lhs_series_param),
                             std::move(rhs_series_param)};
    }

    if(condition_id == "CROSSUNDER") {
      auto [lhs_series_param, rhs_series_param] =
       get_condition_series_params(other_condition);
      return CrossunderMethod{std::move(lhs_series_param),
                              std::move(rhs_series_param)};
    }

    const auto get_conditions_param =
     []<typename TMethod>(
      const TMethod& method) -> std::vector<AnyConditionMethod> {
      if constexpr(requires {
                     {
                       method.conditions()
                     } -> std::convertible_to<std::vector<AnyConditionMethod>>;
                   }) {
        return method.conditions();
      } else if constexpr(requires {
                            {
                              method.other_condition()
                            } -> std::convertible_to<AnyConditionMethod>;
                          }) {
        return {method.other_condition()};
      } else if constexpr(requires {
                            {
                              method.first_condition()
                            } -> std::convertible_to<AnyConditionMethod>;
                            {
                              method.second_condition()
                            } -> std::convertible_to<AnyConditionMethod>;
                          }) {
        return {method.first_condition(), method.second_condition()};
      } else {
        return {method};
      }
    };

    if(condition_id == "ALL_OF") {
      const auto conditions = get_conditions_param(other_condition);
      return AllOfMethod{conditions};
    }

    if(condition_id == "ANY_OF") {
      const auto conditions = get_conditions_param(other_condition);
      return AnyOfMethod{conditions};
    }

    const auto get_first_condition_param =
     []<typename TMethod>(const TMethod& method) -> AnyConditionMethod {
      if constexpr(requires {
                     {
                       method.condition()
                     } -> std::convertible_to<AnyConditionMethod>;
                   }) {
        return method.condition();
      } else if constexpr(requires {
                            {
                              method.first_condition()
                            } -> std::convertible_to<AnyConditionMethod>;
                          }) {
        return method.first_condition();
      } else if constexpr(requires {
                            {
                              method.conditions()
                            } -> std::convertible_to<
                             std::vector<AnyConditionMethod>>;
                          }) {
        const auto conditions = method.conditions();
        if(!conditions.empty()) {
          return conditions.front();
        } else {
          return method;
        }
      } else if constexpr(requires {
                            {
                              method.other_condition()
                            } -> std::convertible_to<AnyConditionMethod>;
                          }) {
        return method.other_condition();
      } else {
        return method;
      }
    };

    const auto get_second_condition_param =
     []<typename TMethod>(const TMethod& method) -> AnyConditionMethod {
      if constexpr(requires {
                     {
                       method.other_condition()
                     } -> std::convertible_to<AnyConditionMethod>;
                   }) {
        return method.other_condition();
      } else if constexpr(requires {
                            {
                              method.second_condition()
                            } -> std::convertible_to<AnyConditionMethod>;
                          }) {
        return method.second_condition();
      } else {
        return method;
      }
    };

    if(condition_id == "NOT") {
      return NotMethod{get_first_condition_param(other_condition)};
    }

    if(condition_id == "AND") {
      return AndMethod{get_first_condition_param(other_condition),
                       get_second_condition_param(other_condition)};
    }

    if(condition_id == "OR") {
      return OrMethod{get_first_condition_param(other_condition),
                      get_second_condition_param(other_condition)};
    }

    if(condition_id == "XOR") {
      return XorMethod{get_first_condition_param(other_condition),
                       get_second_condition_param(other_condition)};
    }

    if(condition_id == "ALWAYS") {
      return AlwaysMethod{};
    }

    if(condition_id == "NEVER") {
      return NeverMethod{};
    }

    return NeverMethod{};
  }

  auto render_condition_method(this auto& self,
                               const AnyConditionMethod& any_condition,
                               WindowContext& context) -> AnyConditionMethod
  {
    if(auto* condition_ptr =
        condition_method_cast<AllOfMethod>(any_condition)) {
      return self.render_condition_method(*condition_ptr, context);
    } else if(auto* condition_ptr =
               condition_method_cast<AnyOfMethod>(any_condition)) {
      return self.render_condition_method(*condition_ptr, context);
    } else if(auto* condition_ptr =
               condition_method_cast<AlwaysMethod>(any_condition)) {
      return self.render_condition_method(*condition_ptr, context);
    } else if(auto* condition_ptr =
               condition_method_cast<NeverMethod>(any_condition)) {
      return self.render_condition_method(*condition_ptr, context);
    } else if(auto* condition_ptr =
               condition_method_cast<LessThanMethod>(any_condition)) {
      return self.render_condition_method(*condition_ptr, context);
    } else if(auto* condition_ptr =
               condition_method_cast<GreaterThanMethod>(any_condition)) {
      return self.render_condition_method(*condition_ptr, context);
    } else if(auto* condition_ptr =
               condition_method_cast<LessEqualMethod>(any_condition)) {
      return self.render_condition_method(*condition_ptr, context);
    } else if(auto* condition_ptr =
               condition_method_cast<GreaterEqualMethod>(any_condition)) {
      return self.render_condition_method(*condition_ptr, context);
    } else if(auto* condition_ptr =
               condition_method_cast<EqualMethod>(any_condition)) {
      return self.render_condition_method(*condition_ptr, context);
    } else if(auto* condition_ptr =
               condition_method_cast<NotEqualMethod>(any_condition)) {
      return self.render_condition_method(*condition_ptr, context);
    } else if(auto* condition_ptr =
               condition_method_cast<CrossoverMethod>(any_condition)) {
      return self.render_condition_method(*condition_ptr, context);
    } else if(auto* condition_ptr =
               condition_method_cast<CrossunderMethod>(any_condition)) {
      return self.render_condition_method(*condition_ptr, context);
    } else if(auto* condition_ptr =
               condition_method_cast<NotMethod>(any_condition)) {
      return self.render_condition_method(*condition_ptr, context);
    } else if(auto* condition_ptr =
               condition_method_cast<AndMethod>(any_condition)) {
      return self.render_condition_method(*condition_ptr, context);
    } else if(auto* condition_ptr =
               condition_method_cast<OrMethod>(any_condition)) {
      return self.render_condition_method(*condition_ptr, context);
    } else if(auto* condition_ptr =
               condition_method_cast<XorMethod>(any_condition)) {
      return self.render_condition_method(*condition_ptr, context);
    } else {
      return self.render_condition_method(*condition_ptr, context);
    }
  }

  template<typename TCondition>
    requires std::same_as<TCondition, EqualMethod> ||
             std::same_as<TCondition, NotEqualMethod> ||
             std::same_as<TCondition, GreaterThanMethod> ||
             std::same_as<TCondition, LessThanMethod> ||
             std::same_as<TCondition, GreaterEqualMethod> ||
             std::same_as<TCondition, LessEqualMethod>
  auto render_condition_method(this auto& self,
                               const TCondition& condition,
                               WindowContext& context) -> AnyConditionMethod
  {
    auto new_condition = condition;

    ImGui::PushID("left_param");
    auto target = new_condition.target();
    self.render_series_method(target, context);
    ImGui::PopID();

    const auto updated_condition_id =
     self.draw_condition_method_combo(new_condition);

    ImGui::PushID("right_param");
    auto threshold = new_condition.threshold();
    self.render_series_method(threshold, context);
    ImGui::PopID();

    new_condition.target(std::move(target));
    new_condition.threshold(std::move(threshold));

    auto changed_method = self.make_condition_method_from_other(
     updated_condition_id, std::move(new_condition));

    return changed_method;
  }

  template<typename TCondition>
    requires std::same_as<TCondition, CrossoverMethod> ||
             std::same_as<TCondition, CrossunderMethod>
  auto render_condition_method(this auto& self,
                               const TCondition& condition,
                               WindowContext& context) -> AnyConditionMethod
  {
    auto new_condition = condition;

    ImGui::PushID("left_param");
    auto signal = new_condition.signal();
    self.render_series_method(signal, context);
    ImGui::PopID();

    const auto updated_condition_id =
     self.draw_condition_method_combo(new_condition);

    ImGui::PushID("right_param");
    auto reference = new_condition.reference();
    self.render_series_method(reference, context);
    ImGui::PopID();

    new_condition.signal(std::move(signal));
    new_condition.reference(std::move(reference));

    auto changed_method = self.make_condition_method_from_other(
     updated_condition_id, std::move(new_condition));

    return changed_method;
  }

  template<typename TCondition>
    requires std::same_as<TCondition, AlwaysMethod> ||
             std::same_as<TCondition, NeverMethod>
  auto render_condition_method(this auto& self,
                               const TCondition& condition,
                               WindowContext& context) -> AnyConditionMethod
  {
    auto new_condition = condition;
    const auto updated_condition_id =
     self.draw_condition_method_combo(new_condition);

    auto changed_method = self.make_condition_method_from_other(
     updated_condition_id, std::move(new_condition));

    return changed_method;
  }

  template<typename TCondition>
    requires std::same_as<TCondition, AllOfMethod> ||
             std::same_as<TCondition, AnyOfMethod>
  auto render_condition_method(this auto& self,
                               const TCondition& condition,
                               WindowContext& context) -> AnyConditionMethod
  {
    auto new_condition = condition;
    const auto updated_condition_id =
     self.draw_condition_method_combo(new_condition);
    ImGui::Indent();

    auto conditions = new_condition.conditions();
    for(auto counter_id = 0; auto& sub_condition : conditions) {
      ImGui::PushID(counter_id++);
      sub_condition = self.render_condition_method(sub_condition, context);

      if(ImGui::Button("Remove Condition")) {
        conditions.erase(conditions.begin() + (counter_id - 1));
        --counter_id;
      }
      ImGui::PopID();

      ImGui::Separator();
    }

    if(ImGui::Button("Add Condition")) {
      conditions.emplace_back(NeverMethod{});
    }

    new_condition = TCondition(std::move(conditions));

    ImGui::Unindent();

    auto changed_method = self.make_condition_method_from_other(
     updated_condition_id, std::move(new_condition));

    return changed_method;
  }

  template<typename TCondition>
    requires std::same_as<TCondition, NotMethod>
  auto render_condition_method(this auto& self,
                               const TCondition& condition,
                               WindowContext& context) -> AnyConditionMethod
  {
    auto new_condition = condition;
    const auto updated_condition_id =
     self.draw_condition_method_combo(new_condition);
    ImGui::Indent();
    auto sub_condition = new_condition.other_condition();
    sub_condition = self.render_condition_method(sub_condition, context);
    new_condition.other_condition(std::move(sub_condition));
    ImGui::Unindent();
    auto changed_method = self.make_condition_method_from_other(
     updated_condition_id, std::move(new_condition));
    return changed_method;
  }

  template<typename TCondition>
    requires std::same_as<TCondition, AndMethod> ||
             std::same_as<TCondition, OrMethod> ||
             std::same_as<TCondition, XorMethod>
  auto render_condition_method(this auto& self,
                               const TCondition& condition,
                               WindowContext& context) -> AnyConditionMethod
  {
    auto new_condition = condition;
    auto first_condition = new_condition.first_condition();
    auto second_condition = new_condition.second_condition();

    {
      ImGui::PushID("first_condition");
      first_condition = self.render_condition_method(first_condition, context);
      ImGui::PopID();
    }

    ImGui::Separator();
    const auto updated_condition_id =
     self.draw_condition_method_combo(new_condition);
    ImGui::Separator();

    {
      ImGui::PushID("second_condition");
      second_condition =
       self.render_condition_method(second_condition, context);
      ImGui::PopID();
    }

    new_condition.first_condition(std::move(first_condition));
    new_condition.second_condition(std::move(second_condition));

    auto changed_method = self.make_condition_method_from_other(
     updated_condition_id, std::move(new_condition));
    return changed_method;
  }

  void render_plot_method(this auto& self,
                          AnyPlotMethod& plot_method,
                          WindowContext& context)
  {
    const auto plot_method_id = get_plot_method_id(plot_method);
    const auto combo_preview_value = get_plot_method_title(plot_method_id);
    if(ImGui::BeginCombo("##plot_method", combo_preview_value.c_str())) {
      static auto filter = ImGuiTextFilter{};

      if(ImGui::IsWindowAppearing()) {
        ImGui::SetKeyboardFocusHere();
        filter.Clear();
      }

      filter.Draw("##Filter", -FLT_MIN);

      for(const auto& plot_method_option : {"LINE", "HLINE", "HISTOGRAM"}) {
        const auto plot_method_title =
         get_plot_method_title(plot_method_option);
        const bool is_selected = plot_method_option == plot_method_id;

        if(filter.PassFilter(plot_method_title.c_str())) {
          if(ImGui::Selectable(plot_method_title.c_str(), is_selected)) {
            plot_method = get_default_plot_method(plot_method_option);
          }
        }
      }
      ImGui::EndCombo();
    }

    {
      ImGui::Indent();
      if(auto* method_ptr = plot_method_cast<LinePlotMethod>(plot_method)) {
        self.render_plot_method(*method_ptr, context);
      } else if(auto* method_ptr =
                 plot_method_cast<HLinePlotMethod>(plot_method)) {
        self.render_plot_method(*method_ptr, context);
      } else if(auto* method_ptr =
                 plot_method_cast<HistogramPlotMethod>(plot_method)) {
        self.render_plot_method(*method_ptr, context);
      }
      ImGui::Unindent();
    }
  }

  void render_plot_method(this auto& self,
                          HLinePlotMethod& plot_method,
                          WindowContext& context)
  {
    ImGui::Text("Level:");
    ImGui::SameLine();
    auto level = plot_method.level();
    if(ImGui::InputDouble("##hline_level", &level, 0.1, 1.0, "%.2f")) {
      plot_method.level(level);
    }

    ImGui::Text("Color:");
    ImGui::SameLine();
    const auto color_vec = ImGui::ColorConvertU32ToFloat4(plot_method.color());
    auto color = std::array{color_vec.x, color_vec.y, color_vec.z, color_vec.w};
    if(ImGui::ColorEdit4("##hline_color", color.data())) {
      plot_method.color(ImGui::ColorConvertFloat4ToU32(
       {color[0], color[1], color[2], color[3]}));
    }
  }

  template<typename TPlotMethod>
    requires std::same_as<TPlotMethod, LinePlotMethod> ||
             std::same_as<TPlotMethod, HistogramPlotMethod>
  void render_plot_method(this auto& self,
                          TPlotMethod& plot_method,
                          WindowContext& context)
  {
    static const auto source_ids =
     std::vector<std::string>{"SERIES", "CONSTANT"};

    ImGui::Text("Source:");
    ImGui::SameLine();
    {
      auto plot_source_id = get_plot_source_method_id(plot_method.source());
      const auto combo_preview_value =
       get_plot_source_method_title(plot_source_id);
      if(ImGui::BeginCombo("##Sources", combo_preview_value.c_str())) {
        static auto filter = ImGuiTextFilter{};

        if(ImGui::IsWindowAppearing()) {
          ImGui::SetKeyboardFocusHere();
          filter.Clear();
        }

        filter.Draw("##Filter", -FLT_MIN);

        for(const auto& source_id : source_ids) {
          const auto source_title = get_plot_source_method_title(source_id);
          const bool is_selected = source_id == plot_source_id;

          if(filter.PassFilter(source_title.c_str())) {
            if(ImGui::Selectable(source_title.c_str(), is_selected)) {
              if(source_id == "SERIES" &&
                 self.available_series_names_.empty()) {
                const auto source_method_title =
                 get_plot_source_method_title(source_id);
                const auto error_message =
                 std::format("Cannot select '{}' when there are no available "
                             "series other than the current one.",
                             source_method_title);
                context.push_action(
                 [error_message](ApplicationState& app_state) {
                   app_state.alert(error_message);
                 });
              } else {
                auto source = get_default_plot_source_method(source_id);

                if(source_id == "SERIES") {
                  if(auto* series_source_ptr =
                      plot_source_method_cast<SeriesPlotSourceMethod>(source)) {
                    const auto first_available_series_name =
                     self.available_series_names_.front();
                    series_source_ptr->series_name(first_available_series_name);
                  }
                }

                plot_method.source(std::move(source));
              }
            }
          }
        }
        ImGui::EndCombo();
      }
    }

    ImGui::Indent();
    auto source = plot_method.source();
    self.render_plot_source_method(source, context);
    plot_method.source(std::move(source));
    ImGui::Unindent();

    ImGui::Text("Color:");
    ImGui::SameLine();
    const auto color_vec = ImGui::ColorConvertU32ToFloat4(plot_method.color());
    auto color = std::array{color_vec.x, color_vec.y, color_vec.z, color_vec.w};
    if(ImGui::ColorEdit4("##line_color", color.data())) {
      plot_method.color(ImGui::ColorConvertFloat4ToU32(
       {color[0], color[1], color[2], color[3]}));
    }
  }

  void render_plot_source_method(this auto& self,
                                 SeriesPlotSourceMethod& plot_source_method,
                                 WindowContext& context)
  {
    ImGui::Text("Series:");
    ImGui::SameLine();
    auto series_name = plot_source_method.series_name();
    if(ImGui::BeginCombo("##series_names", series_name.c_str())) {
      for(const auto& available_series_name : self.available_series_names_) {
        const bool is_selected = series_name == available_series_name;
        if(ImGui::Selectable(available_series_name.c_str(), is_selected)) {
          series_name = available_series_name;
        }
      }
      ImGui::EndCombo();
    }

    plot_source_method.series_name(series_name);
  }

  void render_plot_source_method(this auto& self,
                                 ConstantPlotSourceMethod& plot_source_method,
                                 WindowContext& context)
  {
    ImGui::Text("Value:");
    ImGui::SameLine();
    auto value = plot_source_method.value();
    if(ImGui::InputDouble("##constant_value", &value, 0.1, 1.0, "%.2f")) {
      plot_source_method.value(value);
    }
  }

  void render_plot_source_method(this auto& self,
                                 AnyPlotSourceMethod& plot_source_method,
                                 WindowContext& context)
  {
    if(auto* method_ptr =
        plot_source_method_cast<SeriesPlotSourceMethod>(plot_source_method)) {
      self.render_plot_source_method(*method_ptr, context);
    } else if(auto* method_ptr =
               plot_source_method_cast<ConstantPlotSourceMethod>(
                plot_source_method)) {
      self.render_plot_source_method(*method_ptr, context);
    } else {
      self.render_plot_source_method(*method_ptr, context);
    }
  }

  void submit_strategy_changes(this auto& self, WindowContext& context)
  {
    context.push_action([strategy_ptr = self.selected_strategy_ptr_,
                         edit_strategy_ptr = self.editing_strategy_ptr_](
                         ApplicationState& app_state) {
      if(edit_strategy_ptr->name().empty()) {
        edit_strategy_ptr->name("Unnamed");
      }

      if(strategy_ptr == nullptr) {
        app_state.add_strategy(
         std::make_shared<backtest::Strategy>(*edit_strategy_ptr));
        return;
      }

      const auto reset_backtests =
       !strategy_ptr->equal_rules(*edit_strategy_ptr);
      if(reset_backtests) {
        for(auto& backtest : app_state.backtests()) {
          if(backtest->strategy_ptr() == strategy_ptr) {
            backtest->reset();
          }
        }
      }

      *strategy_ptr = *edit_strategy_ptr;
    });
  }

  void reset(this auto& self)
  {
    self.current_page_ = Page::List;
    self.selected_strategy_ptr_ = nullptr;
    self.editing_strategy_ptr_ = nullptr;
    self.available_series_names_.clear();
  }
};

} // namespace pludux::apps
