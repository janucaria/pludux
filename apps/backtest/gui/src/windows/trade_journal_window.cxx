module;

#include <array>
#include <string>

#include <imgui.h>

export module pludux.apps.backtest:windows.trade_journal_window;

import :window_context;

export namespace pludux::apps {

class TradeJournalWindow {
public:
  void render(this auto& self, WindowContext& context)
  {
    const auto& app_state = context.app_state();

    ImGui::Begin("Trades", nullptr);

    const auto backtest_handle = app_state.selected_backtest_handle();
    const auto backtest_ptr =
     app_state.get_backtest_if_present(backtest_handle);
    if(backtest_ptr) {
      const auto table_flags =
       ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
       ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable |
       ImGuiTableFlags_Hideable | ImGuiTableFlags_ScrollY;

      constexpr auto headers = std::array{"Trade #",
                                          "Type",
                                          "Status",
                                          "Entry Date",
                                          "Exit Date",
                                          "Entry Price",
                                          "Exit Price",
                                          "Take Profit",
                                          "Stop Loss",
                                          "Avg Price",
                                          "Investment (Size)",
                                          "P&L",
                                          "Duration"};

      if(ImGui::BeginTable("TradesTable", headers.size(), table_flags)) {
        ImGui::TableSetupScrollFreeze(0, 1);
        for(const auto& header : headers) {
          if(header == std::string{"Trade #"}) {
            const auto text_size = ImGui::CalcTextSize(header);
            ImGui::TableSetupColumn(
             header, ImGuiTableColumnFlags_WidthFixed, text_size.x);
          } else {
            ImGui::TableSetupColumn(header);
          }
        }

        ImGui::TableHeadersRow();

        const auto& backtest_timelines =
         app_state.get_backtest_timelines(backtest_handle);
        if(backtest_timelines.empty()) {
          ImGui::EndTable();
          ImGui::End();
          return;
        }

        const auto last_timeline_index = backtest_timelines.size() - 1;
        const auto last_timeline_index_i =
         static_cast<int>(last_timeline_index);
        for(auto i = last_timeline_index_i; i >= 0; --i) {
          auto id_counter = i * last_timeline_index;
          if(i == last_timeline_index_i) {
            if(const auto& open_position =
                backtest_timelines.open_position(i)) {
              ImGui::PushID(id_counter++);
              draw_open_trade_row(*open_position);
              ImGui::PopID();
            }
          }

          for(const auto& closed_trade : backtest_timelines.closed_trades(i)) {
            ImGui::PushID(id_counter++);
            draw_closed_trade_row(closed_trade);
            ImGui::PopID();
          }
        }

        ImGui::EndTable();
      }
    }

    ImGui::End();
  }

private:
  static void draw_closed_trade_row(const backtest::ClosedTrade& trade)
  {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::Selectable(std::to_string(trade.trade_id()).c_str(),
                      false,
                      ImGuiSelectableFlags_SpanAllColumns |
                       ImGuiSelectableFlags_AllowOverlap);
    ImGui::TableNextColumn();
    ImGui::Text("%s", trade.position_size() > 0.0 ? "Long" : "Short");
    ImGui::TableNextColumn();
    ImGui::Text("%s", format_trade_status(trade.exit_type()).c_str());
    ImGui::TableNextColumn();
    ImGui::Text("%s", format_datetime(trade.entry_timestamp()).c_str());
    ImGui::TableNextColumn();
    ImGui::Text("%s", format_datetime(trade.exit_timestamp()).c_str());
    ImGui::TableNextColumn();
    ImGui::Text("%s", format_currency(trade.entry_price()).c_str());
    ImGui::TableNextColumn();
    ImGui::Text("%s", format_currency(trade.exit_price()).c_str());
    ImGui::TableNextColumn();
    ImGui::Text("%s", format_currency(trade.take_profit_price()).c_str());
    ImGui::TableNextColumn();
    ImGui::Text("%s", format_currency(trade.stop_loss_price()).c_str());
    ImGui::TableNextColumn();
    ImGui::Text("%s", format_currency(trade.average_price()).c_str());
    ImGui::TableNextColumn();
    const auto investment = trade.investment();
    ImGui::Text(
     "%s (%.0f)", format_currency(investment).c_str(), trade.position_size());
    ImGui::TableNextColumn();

    ImGui::Text("%s (%.2f%%)",
                format_currency(trade.pnl()).c_str(),
                trade.pnl() / trade.investment() * 100.0);
    ImGui::TableNextColumn();
    ImGui::Text("%s", format_duration(trade.duration()).c_str());
  }

  static void draw_open_trade_row(const backtest::OpenPositionSnapshot& trade)
  {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::Selectable(std::to_string(trade.trade_id()).c_str(),
                      false,
                      ImGuiSelectableFlags_SpanAllColumns |
                       ImGuiSelectableFlags_AllowOverlap);
    ImGui::TableNextColumn();
    ImGui::Text("%s", trade.position_size() > 0.0 ? "Long" : "Short");
    ImGui::TableNextColumn();
    ImGui::Text("Open");
    ImGui::TableNextColumn();
    ImGui::Text("%s", format_datetime(trade.entry_timestamp()).c_str());
    ImGui::TableNextColumn();
    ImGui::Text("N/A");
    ImGui::TableNextColumn();
    ImGui::Text("%s", format_currency(trade.entry_price()).c_str());
    ImGui::TableNextColumn();
    ImGui::Text("N/A");
    ImGui::TableNextColumn();
    ImGui::Text("%s", format_currency(trade.take_profit_price()).c_str());
    ImGui::TableNextColumn();
    ImGui::Text("%s", format_currency(trade.stop_loss_price()).c_str());
    ImGui::TableNextColumn();
    ImGui::Text("%s", format_currency(trade.average_price()).c_str());
    ImGui::TableNextColumn();
    const auto investment = trade.investment();
    ImGui::Text(
     "%s (%.0f)", format_currency(investment).c_str(), trade.position_size());
    ImGui::TableNextColumn();
    ImGui::Text("%s (%.2f%%)",
                format_currency(trade.unrealized_pnl()).c_str(),
                trade.unrealized_pnl() / trade.investment() * 100.0);
    ImGui::TableNextColumn();
    ImGui::Text("%s", format_duration(trade.duration()).c_str());
  }

  static auto format_trade_status(backtest::TradeEvent::Type status) noexcept
   -> std::string
  {
    switch(status) {
    case backtest::TradeEvent::Type::exit_signal:
      return "Exit Signal";
    case backtest::TradeEvent::Type::take_profit:
      return "Take Profit";
    case backtest::TradeEvent::Type::stop_loss:
      return "Stop Loss";
    default:
      break;
    }

    return "Unknown";
  }
};

} // namespace pludux::apps
