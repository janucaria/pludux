module;

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <ctime>
#include <format>
#include <optional>
#include <string>
#include <vector>

#include "../ui/pludux_icons.hpp"

#include <imgui.h>

export module pludux.apps.backtest:windows.trade_list_window;

import pludux.backtest;

import :ui.summary_widgets;
import :ui.widgets;
import :window_context;

export namespace pludux::apps {

class TradeListWindow {
public:
  void render(this TradeListWindow& self, WindowContext& context)
  {
    ImGui::Begin("Trades", nullptr);

    const auto& app_state = context.app_state();
    const auto portfolio_handle = app_state.selected_portfolio_handle();
    const auto* portfolio =
     app_state.get_portfolio_if_present(portfolio_handle);
    if(!portfolio) {
      ui::summary_status(PLUDUX_ICON_TRADES,
                         "No portfolio selected",
                         "Select a portfolio to inspect its open and closed "
                         "trades.");
      ImGui::End();
      return;
    }

    const auto selected_run = app_state.selected_portfolio_backtest();
    if(!selected_run) {
      ui::summary_status(PLUDUX_ICON_TRADES,
                         "No Backtest selected",
                         "Select a Backtest from the active Portfolio to "
                         "inspect its trades.");
      ImGui::End();
      return;
    }

    if(!app_state.is_portfolio_ready(*portfolio)) {
      ui::summary_status(PLUDUX_ICON_WARNING,
                         "Portfolio setup is incomplete",
                         "Complete the portfolio configuration before trade "
                         "results can be shown.");
      ImGui::End();
      return;
    }

    const auto& results = app_state.get_portfolio_results(portfolio_handle);
    const auto* backtest_results = results.backtest(*selected_run);
    if(!backtest_results || backtest_results->timeline().empty()) {
      ui::summary_status(PLUDUX_ICON_TRADES,
                         "No trade results yet",
                         "Trades will appear here as soon as the selected "
                         "Backtest produces timeline results.");
      ImGui::End();
      return;
    }

    const auto backtest_iterator = std::ranges::find(
     results.backtests(), *selected_run, &backtest::BacktestResults::key);
    const auto backtest_index = static_cast<std::size_t>(
     std::ranges::distance(results.backtests().begin(), backtest_iterator));
    const auto* backtest_config =
     app_state.get_backtest_if_present(selected_run->backtest_handle);
    const auto* asset =
     app_state.get_asset_if_present(selected_run->asset_handle);
    const auto label = backtest_config && asset
                        ? backtest_config->name() + " — " + asset->name()
                        : std::string{"Missing Backtest Asset"};
    auto trades = collect_trades(
     backtest_results->timeline(), self.trade_source_, backtest_index, label);
    std::ranges::sort(trades, {}, &TradeView::entry_timestamp);
    std::ranges::reverse(trades);
    self.synchronize_selection(trades);

    const auto visible_trades = self.filter_trades(trades);
    self.render_filter_bar(visible_trades.size(), trades.size());
    ImGui::Spacing();

    if(trades.empty()) {
      ui::summary_status(PLUDUX_ICON_TRADES,
                         "No trades were generated",
                         "The selected Backtest completed without opening a "
                         "position. Review its strategy signals or tested "
                         "date range.");
      ImGui::End();
      return;
    }

    const auto* selected_trade = self.find_selected_trade(trades);
    const auto available = ImGui::GetContentRegionAvail();
    constexpr auto split_layout_width = 920.0f;
    if(available.x >= split_layout_width) {
      const auto list_width = std::max(560.0f, available.x * 0.64f);
      self.render_trade_list(visible_trades, ImVec2{list_width, 0.0f});
      ImGui::SameLine();
      render_trade_details(selected_trade, ImVec2{0.0f, 0.0f});
    } else {
      const auto list_height = std::clamp(available.y * 0.56f, 240.0f, 440.0f);
      self.render_trade_list(visible_trades, ImVec2{0.0f, list_height});
      ImGui::Spacing();
      render_trade_details(selected_trade, ImVec2{0.0f, 0.0f});
    }

    ImGui::End();
  }

private:
  enum class TradeFilter { All, Open, Winners, Losers };
  enum class TradeSource { Actual, Hypothetical };

  struct TradeKey {
    std::size_t id{};
    bool open{};
    std::size_t backtest_index{};

    auto operator==(const TradeKey&) const noexcept -> bool = default;
  };

  struct TradeView {
    TradeKey key;
    std::string backtest_name;
    bool long_position{};
    std::string status;
    std::time_t entry_timestamp{};
    std::optional<std::time_t> exit_timestamp;
    double entry_price{};
    double last_price{};
    double average_price{};
    double quantity{};
    double investment{};
    double pnl{};
    double fees{};
    std::time_t duration{};
    double risk_distance{};
    double risk_reference_price{};
    double risk_boundary_price{};
    std::string signal_exits;
    std::string take_profits;
    std::string stop_losses;
  };

  TradeFilter filter_{TradeFilter::All};
  TradeSource trade_source_{TradeSource::Actual};
  ImGuiTextFilter search_filter_{};
  std::optional<TradeKey> selected_trade_{};

  static auto collect_trades(const backtest::BacktestTimeline& timeline,
                             TradeSource source,
                             std::size_t backtest_index,
                             std::string backtest_name)
   -> std::vector<TradeView>
  {
    auto trades = std::vector<TradeView>{};
    if(timeline.empty()) {
      return trades;
    }

    const auto last_index = timeline.size() - 1;
    if(source == TradeSource::Hypothetical) {
      if(const auto& open_position =
          timeline.strategy_open_position(last_index)) {
        trades.push_back(make_trade_view(
         *open_position, timeline.market_timestamp(last_index)));
      }
      for(auto index = timeline.size(); index-- > 0;) {
        const auto& closed = timeline.strategy_closed_positions(index);
        for(auto position = closed.rbegin(); position != closed.rend();
            ++position) {
          trades.push_back(make_trade_view(*position));
        }
      }
      for(auto& trade : trades) {
        trade.key.backtest_index = backtest_index;
        trade.backtest_name = backtest_name;
      }
      return trades;
    }

    if(const auto& open_position = timeline.open_position(last_index)) {
      trades.push_back(make_trade_view(*open_position));
    }

    for(auto index = timeline.size(); index-- > 0;) {
      const auto& closed_trades = timeline.closed_trades(index);
      for(auto trade = closed_trades.rbegin(); trade != closed_trades.rend();
          ++trade) {
        trades.push_back(make_trade_view(*trade));
      }
    }
    for(auto& trade : trades) {
      trade.key.backtest_index = backtest_index;
      trade.backtest_name = backtest_name;
    }
    return trades;
  }

  static auto make_trade_view(const backtest::StrategyClosedPosition& position)
   -> TradeView
  {
    const auto average_entry = position.normalized_entry_quantity() > 0.0
                                ? position.normalized_entry_notional() /
                                   position.normalized_entry_quantity()
                                : 0.0;
    const auto exit_price = position.intents().empty()
                             ? average_entry
                             : position.intents().back().price();
    return TradeView{.key = TradeKey{position.strategy_trade_id(), false},
                     .long_position =
                      position.direction() == backtest::StrategyDirection::Long,
                     .status = "Hypothetical",
                     .entry_timestamp = position.entry_timestamp(),
                     .exit_timestamp = position.exit_timestamp(),
                     .entry_price = average_entry,
                     .last_price = exit_price,
                     .average_price = average_entry,
                     .quantity = position.normalized_entry_quantity(),
                     .investment = position.normalized_entry_notional(),
                     .pnl = position.directional_price_pnl(),
                     .fees = 0.0,
                     .duration = position.duration(),
                     .risk_distance = NAN,
                     .risk_reference_price = NAN,
                     .risk_boundary_price = NAN,
                     .signal_exits = "See ordered strategy intents",
                     .take_profits = "See ordered strategy intents",
                     .stop_losses = "See ordered strategy intents"};
  }

  static auto
  make_trade_view(const backtest::StrategyOpenPositionSnapshot& position,
                  std::time_t market_timestamp) -> TradeView
  {
    return TradeView{.key = TradeKey{position.strategy_trade_id(), true},
                     .long_position =
                      position.direction() == backtest::StrategyDirection::Long,
                     .status = "Hypothetical open",
                     .entry_timestamp = position.entry_timestamp(),
                     .exit_timestamp = std::nullopt,
                     .entry_price = position.average_price(),
                     .last_price = position.market_price(),
                     .average_price = position.average_price(),
                     .quantity = position.normalized_quantity(),
                     .investment = position.normalized_investment(),
                     .pnl = position.unrealized_price_pnl(),
                     .fees = 0.0,
                     .duration = position.duration(market_timestamp),
                     .risk_distance = NAN,
                     .risk_reference_price = NAN,
                     .risk_boundary_price = NAN,
                     .signal_exits = "Unrealized shadow position",
                     .take_profits = "Unrealized shadow position",
                     .stop_losses = "Unrealized shadow position"};
  }

  static auto make_trade_view(const backtest::ClosedTrade& trade) -> TradeView
  {
    return TradeView{
     .key = TradeKey{trade.trade_id(), false},
     .long_position = trade.position_size() > 0.0,
     .status = exit_status_label(trade.exit_type()),
     .entry_timestamp = trade.entry_timestamp(),
     .exit_timestamp = trade.exit_timestamp(),
     .entry_price = trade.entry_price(),
     .last_price = trade.exit_price(),
     .average_price = trade.average_price(),
     .quantity = std::abs(trade.position_size()),
     .investment = std::abs(trade.investment()),
     .pnl = trade.pnl(),
     .fees = trade.total_entry_fees() + trade.total_exit_fees(),
     .duration = trade.duration(),
     .risk_distance = trade.risk_distance(),
     .risk_reference_price = trade.risk_reference_price(),
     .risk_boundary_price = trade.risk_boundary_price(),
     .signal_exits = format_signal_exits(trade.signal_exit_states()),
     .take_profits = format_take_profits(trade.take_profit_levels()),
     .stop_losses = format_stop_losses(trade.stop_loss_levels())};
  }

  static auto make_trade_view(const backtest::OpenPositionSnapshot& trade)
   -> TradeView
  {
    return TradeView{
     .key = TradeKey{trade.trade_id(), true},
     .long_position = trade.position_size() > 0.0,
     .status = "Open",
     .entry_timestamp = trade.entry_timestamp(),
     .exit_timestamp = std::nullopt,
     .entry_price = trade.entry_price(),
     .last_price = trade.market_price(),
     .average_price = trade.average_price(),
     .quantity = std::abs(trade.position_size()),
     .investment = std::abs(trade.investment()),
     .pnl = trade.unrealized_pnl(),
     .fees = trade.total_entry_fees(),
     .duration = trade.duration(),
     .risk_distance = trade.risk_distance(),
     .risk_reference_price = trade.risk_reference_price(),
     .risk_boundary_price = trade.risk_boundary_price(),
     .signal_exits = format_signal_exits(trade.signal_exit_states()),
     .take_profits = format_take_profits(trade.take_profit_levels()),
     .stop_losses = format_stop_losses(trade.stop_loss_levels())};
  }

  void synchronize_selection(this TradeListWindow& self,
                             const std::vector<TradeView>& trades)
  {
    if(trades.empty()) {
      self.selected_trade_.reset();
      return;
    }

    const auto selected_exists =
     self.selected_trade_ &&
     std::ranges::any_of(trades, [&self](const auto& trade) {
       return trade.key == *self.selected_trade_;
     });
    if(!selected_exists) {
      self.selected_trade_ = trades.front().key;
    }
  }

  auto filter_trades(this const TradeListWindow& self,
                     const std::vector<TradeView>& trades)
   -> std::vector<const TradeView*>
  {
    auto visible = std::vector<const TradeView*>{};
    visible.reserve(trades.size());
    for(const auto& trade : trades) {
      if(!self.matches_filter(trade)) {
        continue;
      }

      const auto searchable_text =
       std::format("#{} {} {} {}",
                   trade.key.id,
                   trade.backtest_name,
                   trade.long_position ? "Long" : "Short",
                   trade.status);
      if(self.search_filter_.PassFilter(searchable_text.c_str())) {
        visible.push_back(&trade);
      }
    }
    return visible;
  }

  auto matches_filter(this const TradeListWindow& self, const TradeView& trade)
   -> bool
  {
    switch(self.filter_) {
    case TradeFilter::Open:
      return trade.key.open;
    case TradeFilter::Winners:
      return !trade.key.open && trade.pnl > 0.0;
    case TradeFilter::Losers:
      return !trade.key.open && trade.pnl < 0.0;
    case TradeFilter::All:
      return true;
    }
    return true;
  }

  void render_filter_bar(this TradeListWindow& self,
                         std::size_t visible_count,
                         std::size_t total_count)
  {
    const auto flags = ImGuiTableFlags_SizingStretchProp;
    if(ImGui::BeginTable("##trade_filters", 4, flags)) {
      ImGui::TableSetupColumn("Search", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableSetupColumn(
       "Source", ImGuiTableColumnFlags_WidthFixed, 130.0f);
      ImGui::TableSetupColumn(
       "Filter", ImGuiTableColumnFlags_WidthFixed, 150.0f);
      ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_WidthFixed, 90.0f);
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ui::search_filter(self.search_filter_, "##trade_search");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(-1.0f);
      if(ImGui::BeginCombo("##trade_source",
                           self.trade_source_ == TradeSource::Actual
                            ? "Actual"
                            : "Hypothetical")) {
        for(const auto option :
            {TradeSource::Actual, TradeSource::Hypothetical}) {
          const auto selected = self.trade_source_ == option;
          if(ImGui::Selectable(option == TradeSource::Actual ? "Actual"
                                                             : "Hypothetical",
                               selected)) {
            self.trade_source_ = option;
            self.selected_trade_.reset();
          }
          if(selected) {
            ImGui::SetItemDefaultFocus();
          }
        }
        ImGui::EndCombo();
      }
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(-1.0f);
      if(ImGui::BeginCombo("##trade_filter", filter_label(self.filter_))) {
        for(const auto option : {TradeFilter::All,
                                 TradeFilter::Open,
                                 TradeFilter::Winners,
                                 TradeFilter::Losers}) {
          const auto selected = self.filter_ == option;
          if(ImGui::Selectable(filter_label(option), selected)) {
            self.filter_ = option;
          }
          if(selected) {
            ImGui::SetItemDefaultFocus();
          }
        }
        ImGui::EndCombo();
      }
      ImGui::TableNextColumn();
      ImGui::AlignTextToFramePadding();
      ImGui::TextDisabled("%zu / %zu", visible_count, total_count);
      ImGui::EndTable();
    }
  }

  void render_trade_list(this TradeListWindow& self,
                         const std::vector<const TradeView*>& trades,
                         ImVec2 size)
  {
    ImGui::BeginChild("##trade_list", size, ImGuiChildFlags_Borders);
    if(trades.empty()) {
      const auto message = self.search_filter_.IsActive()
                            ? "No trades match this search."
                            : "No trades match this filter.";
      ImGui::TextDisabled("%s", message);
      ImGui::EndChild();
      return;
    }

    const auto flags = ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg |
                       ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX |
                       ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit;
    if(ImGui::BeginTable("##trade_table", 10, flags)) {
      ImGui::TableSetupScrollFreeze(2, 1);
      ImGui::TableSetupColumn("Trade", ImGuiTableColumnFlags_WidthFixed, 64.0f);
      ImGui::TableSetupColumn(
       "Backtest", ImGuiTableColumnFlags_WidthFixed, 130.0f);
      ImGui::TableSetupColumn("Side", ImGuiTableColumnFlags_WidthFixed, 58.0f);
      ImGui::TableSetupColumn(
       "Status", ImGuiTableColumnFlags_WidthFixed, 92.0f);
      ImGui::TableSetupColumn(
       "Opened", ImGuiTableColumnFlags_WidthFixed, 142.0f);
      ImGui::TableSetupColumn(
       "Exit / Mark", ImGuiTableColumnFlags_WidthFixed, 98.0f);
      ImGui::TableSetupColumn(
       "Avg entry", ImGuiTableColumnFlags_WidthFixed, 98.0f);
      ImGui::TableSetupColumn("P&L", ImGuiTableColumnFlags_WidthFixed, 104.0f);
      ImGui::TableSetupColumn(
       "Return", ImGuiTableColumnFlags_WidthFixed, 72.0f);
      ImGui::TableSetupColumn(
       "Duration", ImGuiTableColumnFlags_WidthFixed, 88.0f);
      ImGui::TableHeadersRow();

      auto clipper = ImGuiListClipper{};
      clipper.Begin(static_cast<int>(trades.size()));
      while(clipper.Step()) {
        for(auto index = clipper.DisplayStart; index < clipper.DisplayEnd;
            ++index) {
          self.render_trade_row(*trades[static_cast<std::size_t>(index)]);
        }
      }
      ImGui::EndTable();
    }
    ImGui::EndChild();
  }

  void render_trade_row(this TradeListWindow& self, const TradeView& trade)
  {
    ImGui::PushID(static_cast<int>(trade.key.backtest_index));
    ImGui::PushID(static_cast<int>(trade.key.id));
    ImGui::PushID(trade.key.open ? 1 : 0);
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    const auto selected =
     self.selected_trade_ && *self.selected_trade_ == trade.key;
    const auto label = std::format("#{}", trade.key.id);
    if(ImGui::Selectable(label.c_str(),
                         selected,
                         ImGuiSelectableFlags_SpanAllColumns |
                          ImGuiSelectableFlags_AllowOverlap)) {
      self.selected_trade_ = trade.key;
    }

    ImGui::TableNextColumn();
    ImGui::TextUnformatted(trade.backtest_name.c_str());
    ImGui::TableNextColumn();
    ImGui::TextColored(direction_color(trade.long_position),
                       "%s",
                       trade.long_position ? "Long" : "Short");
    ImGui::TableNextColumn();
    ImGui::TextColored(status_color(trade), "%s", trade.status.c_str());
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(format_datetime(trade.entry_timestamp).c_str());
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(format_optional_currency(trade.last_price).c_str());
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(
     format_optional_currency(trade.average_price).c_str());
    ImGui::TableNextColumn();
    ImGui::TextColored(
     outcome_color(trade.pnl), "%s", format_currency(trade.pnl).c_str());
    ImGui::TableNextColumn();
    ImGui::TextColored(
     outcome_color(trade.pnl), "%s", format_return(trade).c_str());
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(format_duration(trade.duration).c_str());
    ImGui::PopID();
    ImGui::PopID();
    ImGui::PopID();
  }

  auto find_selected_trade(this const TradeListWindow& self,
                           const std::vector<TradeView>& trades)
   -> const TradeView*
  {
    if(!self.selected_trade_) {
      return nullptr;
    }
    const auto selected =
     std::ranges::find_if(trades, [&self](const auto& trade) {
       return trade.key == *self.selected_trade_;
     });
    return selected == trades.end() ? nullptr : &*selected;
  }

  static void render_trade_details(const TradeView* trade, ImVec2 size)
  {
    ImGui::BeginChild("##trade_details",
                      size,
                      ImGuiChildFlags_Borders |
                       ImGuiChildFlags_AlwaysUseWindowPadding);
    if(!trade) {
      ImGui::TextDisabled("Select a trade to inspect its details.");
      ImGui::EndChild();
      return;
    }

    ImGui::SetWindowFontScale(1.12f);
    ImGui::Text("Trade #%zu", trade->key.id);
    ImGui::SetWindowFontScale(1.0f);
    ImGui::SameLine();
    ImGui::TextColored(status_color(*trade), "%s", trade->status.c_str());
    ImGui::TextDisabled("%s position | %s P&L",
                        trade->long_position ? "Long" : "Short",
                        trade->key.open ? "unrealized" : "realized");
    ImGui::Spacing();

    ImGui::SeparatorText("Execution");
    if(ImGui::BeginTable("##execution_details",
                         2,
                         ImGuiTableFlags_SizingStretchProp |
                          ImGuiTableFlags_RowBg)) {
      detail_row("Opened", format_datetime(trade->entry_timestamp));
      detail_row("Closed",
                 trade->exit_timestamp ? format_datetime(*trade->exit_timestamp)
                                       : "Open");
      detail_row("Entry signal", format_optional_currency(trade->entry_price));
      detail_row(trade->key.open ? "Market price" : "Exit price",
                 format_optional_currency(trade->last_price));
      detail_row("Average entry",
                 format_optional_currency(trade->average_price));
      detail_row("Quantity", std::format("{:.6g}", trade->quantity));
      detail_row("Investment", format_currency(trade->investment));
      detail_row("Fees", format_currency(trade->fees));
      detail_row("Duration", format_duration(trade->duration));
      detail_row("P&L", format_currency(trade->pnl), outcome_color(trade->pnl));
      detail_row("Return", format_return(*trade), outcome_color(trade->pnl));
      ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::SeparatorText("Risk");
    if(ImGui::BeginTable("##risk_details",
                         2,
                         ImGuiTableFlags_SizingStretchProp |
                          ImGuiTableFlags_RowBg)) {
      detail_row("Risk distance",
                 format_optional_currency(trade->risk_distance));
      detail_row("Risk reference",
                 format_optional_currency(trade->risk_reference_price));
      detail_row("1R boundary",
                 format_optional_currency(trade->risk_boundary_price));
      ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::SeparatorText("Exit plan");
    detail_block("Signal exits", trade->signal_exits);
    detail_block("Take profits", trade->take_profits);
    detail_block("Stop losses", trade->stop_losses);
    ImGui::EndChild();
  }

  static void detail_row(const char* label,
                         const std::string& value,
                         std::optional<ImVec4> color = std::nullopt)
  {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::TextDisabled("%s", label);
    ImGui::TableNextColumn();
    if(color) {
      ImGui::TextColored(*color, "%s", value.c_str());
    } else {
      ImGui::TextWrapped("%s", value.c_str());
    }
  }

  static void detail_block(const char* label, const std::string& value)
  {
    ImGui::TextDisabled("%s", label);
    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() +
                           ImGui::GetContentRegionAvail().x);
    ImGui::TextUnformatted(value.c_str());
    ImGui::PopTextWrapPos();
    ImGui::Spacing();
  }

  static auto format_signal_exits(const auto& states) -> std::string
  {
    auto result = std::string{};
    for(auto index = std::size_t{0}; index < states.size(); ++index) {
      append_separator(result);
      result += std::format(
       "Exit {} ({})",
       index + 1,
       state_label(states[index].enabled(), states[index].consumed()));
    }
    return result.empty() ? "None configured" : result;
  }

  static auto format_take_profits(const auto& levels) -> std::string
  {
    auto result = std::string{};
    for(const auto& level : levels) {
      append_separator(result);
      result += std::format("{} ({})",
                            format_optional_currency(level.price()),
                            state_label(level.enabled(), level.consumed()));
    }
    return result.empty() ? "None configured" : result;
  }

  static auto format_stop_losses(const auto& levels) -> std::string
  {
    auto result = std::string{};
    for(const auto& level : levels) {
      append_separator(result);
      result += std::format("{} -> {}{} ({})",
                            format_optional_currency(level.evaluated_price()),
                            format_optional_currency(level.effective_price()),
                            level.trailing() ? " trailing" : "",
                            state_label(level.enabled(), level.consumed()));
    }
    return result.empty() ? "None configured" : result;
  }

  static void append_separator(std::string& text)
  {
    if(!text.empty()) {
      text += "; ";
    }
  }

  static auto state_label(bool enabled, bool consumed) -> const char*
  {
    if(!enabled) {
      return "disabled";
    }
    return consumed ? "used" : "active";
  }

  static auto exit_status_label(backtest::TradeEvent::Type exit_type)
   -> std::string
  {
    switch(exit_type) {
    case backtest::TradeEvent::Type::exit_signal:
      return "Signal exit";
    case backtest::TradeEvent::Type::take_profit:
      return "Take profit";
    case backtest::TradeEvent::Type::stop_loss:
      return "Stop loss";
    default:
      return "Closed";
    }
  }

  static auto filter_label(TradeFilter filter) -> const char*
  {
    switch(filter) {
    case TradeFilter::All:
      return "All trades";
    case TradeFilter::Open:
      return "Open";
    case TradeFilter::Winners:
      return "Winners";
    case TradeFilter::Losers:
      return "Losers";
    }
    return "All trades";
  }

  static auto format_optional_currency(double value) -> std::string
  {
    return std::isfinite(value) ? format_currency(value) : "\u2014";
  }

  static auto format_return(const TradeView& trade) -> std::string
  {
    if(trade.investment <= 0.0) {
      return "\u2014";
    }
    return std::format("{:.2f}%", (trade.pnl / trade.investment) * 100.0);
  }

  static auto outcome_color(double value) -> ImVec4
  {
    if(value > 0.0) {
      return ImVec4{0.38f, 0.78f, 0.50f, 1.0f};
    }
    if(value < 0.0) {
      return ImVec4{0.91f, 0.40f, 0.40f, 1.0f};
    }
    return ImGui::GetStyleColorVec4(ImGuiCol_Text);
  }

  static auto direction_color(bool long_position) -> ImVec4
  {
    return long_position ? ImVec4{0.38f, 0.68f, 0.95f, 1.0f}
                         : ImVec4{0.93f, 0.67f, 0.28f, 1.0f};
  }

  static auto status_color(const TradeView& trade) -> ImVec4
  {
    if(trade.key.open) {
      return ImVec4{0.38f, 0.68f, 0.95f, 1.0f};
    }
    return outcome_color(trade.pnl);
  }
};

} // namespace pludux::apps
