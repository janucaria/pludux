module;

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <format>
#include <optional>
#include <string>
#include <vector>

#include <imgui.h>

export module pludux.apps.backtest:windows.events_window;

import pludux.backtest;
import pludux.apps.backtest.application_state;
import :window_context;

export namespace pludux::apps {

class EventsWindow {
  using Type = backtest::TradeEvent::Type;

  struct EventKind {
    Type type;
    const char* name;
  };
  static constexpr std::array kinds = {
   EventKind{Type::entry, "Entry"},
   EventKind{Type::scale_in, "Scale in"},
   EventKind{Type::scale_out, "Scale out"},
   EventKind{Type::exit_signal, "Signal exit"},
   EventKind{Type::stop_loss, "Stop loss"},
   EventKind{Type::take_profit, "Take profit"},
   EventKind{Type::rejected_insufficient_cash, "Insufficient cash"},
   EventKind{Type::rejected_maximum_open_trades, "Maximum open trades"},
   EventKind{Type::rejected_maximum_combined_layers,
             "Maximum combined layers"}};

  backtest::PortfolioStoreHandle portfolio_;
  std::optional<Type> type_;

public:
  void render(WindowContext& context)
  {
    if(ImGui::Begin("Events")) {
      render_contents(context.app_state());
    }
    ImGui::End();
  }

private:
  void render_contents(const ApplicationState& state)
  {
    const auto handle = state.selected_portfolio_handle();
    if(portfolio_ != handle) {
      portfolio_ = handle;
      type_.reset();
    }
    const auto* portfolio = state.get_portfolio_if_present(handle);
    if(!portfolio) {
      ImGui::TextDisabled("Select a Portfolio to view its execution events.");
      return;
    }
    const auto selected = state.selected_portfolio_strategy();
    if(!selected) {
      ImGui::TextDisabled(
       "Select a strategy from the active Portfolio to inspect its events.");
      return;
    }
    if(!state.is_portfolio_ready(*portfolio)) {
      ImGui::TextDisabled("Portfolio setup is incomplete.");
      return;
    }
    const auto* current = state.get_portfolio_results_if_present(handle);
    if(!current || current->events().empty()) {
      ImGui::TextDisabled("No execution events recorded.");
      return;
    }
    const auto& results = *current;
    render_filter();
    std::vector<std::size_t> matches;
    for(std::size_t i = 0; i < results.events().size(); ++i) {
      const auto ref = results.events()[i];
      const auto& run = results.backtests()[ref.backtest_index];
      const auto& event =
       run.timeline().trade_events(ref.bar_index)[ref.event_index];
      if(run.key() == selected->run &&
         event.strategy_index() == selected->strategy_index &&
         (!type_ || event.type() == *type_ ||
          (*type_ == Type::scale_out && event.is_scale_out()))) {
        matches.push_back(i);
      }
    }
    // Stable sorting keeps the recorded execution order within each timestamp.
    std::stable_sort(matches.begin(), matches.end(), [&](auto lhs, auto rhs) {
      const auto timestamp = [&](std::size_t index) {
        const auto ref = results.events()[index];
        return results.backtests()[ref.backtest_index]
         .timeline()
         .trade_events(ref.bar_index)[ref.event_index]
         .timestamp();
      };
      return timestamp(lhs) > timestamp(rhs);
    });
    const auto count = matches.size();
    if(count == 0) {
      ImGui::TextDisabled("No matching events.");
      return;
    }
    if(!ImGui::BeginTable("events",
                          8,
                          ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable |
                           ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY |
                           ImGuiTableFlags_SizingFixedFit)) {
      return;
    }
    for(auto column : {"Time", "System", "Strategy", "Asset"}) {
      ImGui::TableSetupColumn(column);
    }
    const auto event_width =
     ImGui::CalcTextSize("Rejected \xC2\xB7 Maximum combined layers").x;
    ImGui::TableSetupColumn(
     "Event", ImGuiTableColumnFlags_WidthFixed, event_width);
    for(auto column : {"Quantity", "Price", "Cash (required / available)"}) {
      ImGui::TableSetupColumn(column);
    }
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableHeadersRow();
    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(count));
    while(clipper.Step()) {
      for(int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
        const auto ref = results.events()[matches[row]];
        const auto& run = results.backtests()[ref.backtest_index];
        render_row(state,
                   run,
                   run.timeline().trade_events(ref.bar_index)[ref.event_index]);
      }
    }
    ImGui::EndTable();
  }

  void render_filter()
  {
    ImGui::SetNextItemWidth(190);
    if(ImGui::BeginCombo("Event", type_ ? kind_name(*type_) : "All events")) {
      if(ImGui::Selectable("All events", !type_)) {
        type_.reset();
      }
      for(const auto& kind : kinds) {
        if(ImGui::Selectable(kind.name, type_ == kind.type)) {
          type_ = kind.type;
        }
      }
      ImGui::EndCombo();
    }
  }

  static constexpr auto kind_name(Type type) -> const char*
  {
    for(const auto& kind : kinds) {
      if(type == kind.type) {
        return kind.name;
      }
    }
    return "Unknown event";
  }

  static void render_row(const ApplicationState& state,
                         const backtest::BacktestResults& run,
                         const backtest::TradeEvent& event)
  {
    const auto* system = state.get_system_if_present(run.system_handle());
    const auto* strategy =
     system && event.strategy_index() < system->strategy_count()
      ? state.get_strategy_if_present(
         system->strategy_handle(event.strategy_index()))
      : nullptr;
    const auto* asset = state.get_asset_if_present(run.asset_handle());
    const auto side =
     event.is_rejected()                                          ? "Rejected"
     : event.position_size_after() > event.position_size_before() ? "Buy"
                                                                  : "Sell";
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(format_datetime(event.timestamp()).c_str());
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(system ? system->name().c_str() : "Missing System");
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(strategy ? strategy->name().c_str()
                                    : "Missing Strategy");
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(asset ? asset->name().c_str() : "Missing Asset");
    ImGui::TableNextColumn();
    const auto partial =
     event.is_scale_out() && event.type() != Type::scale_out;
    ImGui::Text("%s \xC2\xB7 %s%s",
                side,
                partial ? "Scale out \xC2\xB7 " : "",
                kind_name(event.type()));
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(
     std::format("{}", std::abs(event.position_size())).c_str());
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(std::format("{}", event.price()).c_str());
    ImGui::TableNextColumn();
    if(event.type() == Type::rejected_insufficient_cash) {
      ImGui::Text("%s / %s",
                  format_currency(event.rejection_required_cash()).c_str(),
                  format_currency(event.rejection_available_cash()).c_str());
    }
  }
};

} // namespace pludux::apps
