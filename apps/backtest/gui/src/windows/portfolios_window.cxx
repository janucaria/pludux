module;

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "../ui/pludux_icons.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

export module pludux.apps.backtest:windows.portfolios_window;

import pludux.backtest;
import :series_node_editor;
import :ui.widgets;
import :window_context;

export namespace pludux::apps {

class PortfoliosWindow {
public:
  void render(this PortfoliosWindow& self, WindowContext& context)
  {
    ImGui::Begin("Portfolios");
    if(self.editor_open_) {
      self.render_editor(context);
    } else {
      self.render_list(context);
    }
    ImGui::End();
  }

  void discard_draft(this PortfoliosWindow& self) noexcept
  {
    self.editor_open_ = false;
    self.editing_.reset();
    self.editing_handle_.reset();
  }

private:
  std::optional<backtest::PortfolioStoreHandle> editing_handle_;
  std::optional<backtest::PortfolioStoreHandle> open_requested_;
  std::shared_ptr<backtest::Portfolio> editing_;
  ImGuiTextFilter filter_;
  ImGuiTextFilter backtest_picker_filter_;
  ui::SeriesNodeEditor series_node_editor_{
   ui::SeriesNodeCatalog::PortfolioComparator};
  bool editor_open_{};

  static auto make_editable_portfolio(backtest::Portfolio portfolio)
   -> std::shared_ptr<backtest::Portfolio>
  {
    if(portfolio.maximum_open_trades() == 0) {
      portfolio.maximum_open_trades(1);
    }
    return std::make_shared<backtest::Portfolio>(std::move(portfolio));
  }

  void render_list(this PortfoliosWindow& self, WindowContext& context)
  {
    auto& app_state = context.app_state();
    if(ImGui::Button(PLUDUX_ICON_ADD " New Portfolio")) {
      auto portfolio = backtest::Portfolio{};
      if(!app_state.get_market_handles().empty()) {
        portfolio.market_handle(app_state.get_market_handles().front());
      }
      if(!app_state.get_broker_handles().empty()) {
        portfolio.broker_handle(app_state.get_broker_handles().front());
      }
      self.editing_handle_.reset();
      self.editing_ = self.make_editable_portfolio(std::move(portfolio));
      self.editor_open_ = true;
      return;
    }

    ImGui::Spacing();
    ui::search_filter(self.filter_, "##portfolios_search");
    ImGui::BeginChild("item view", ImVec2(0, 0));

    const auto& portfolio_handles = app_state.get_portfolio_handles();
    auto visible_portfolio_count = std::size_t{};
    for(auto index = std::size_t{}; index < portfolio_handles.size(); ++index) {
      const auto handle = portfolio_handles[index];
      const auto* portfolio = app_state.get_portfolio_if_present(handle);
      if(!portfolio) {
        continue;
      }

      const auto portfolio_matches =
       self.filter_.PassFilter(portfolio->name().c_str());
      auto child_matches = false;
      const auto runs = app_state.expanded_backtest_runs(*portfolio);
      for(const auto run : runs) {
        const auto* backtest =
         app_state.get_backtest_if_present(run.backtest_handle);
        const auto* asset = app_state.get_asset_if_present(run.asset_handle);
        child_matches =
         child_matches ||
         (backtest && self.filter_.PassFilter(backtest->name().c_str())) ||
         (asset && self.filter_.PassFilter(asset->name().c_str()));
      }
      if(!portfolio_matches && !child_matches) {
        continue;
      }
      ++visible_portfolio_count;

      ImGui::PushID(static_cast<int>(handle.slot_index()));
      ImGui::PushID(static_cast<int>(handle.generation()));
      const auto selected = app_state.selected_portfolio_handle() == handle;
      const auto selected_backtest =
       selected ? app_state.selected_portfolio_backtest()
                : std::optional<backtest::BacktestRunKey>{};
      const auto force_open_for_filter =
       self.filter_.IsActive() && child_matches;
      if(force_open_for_filter || self.open_requested_ == handle) {
        ImGui::SetNextItemOpen(true, ImGuiCond_Always);
        if(self.open_requested_ == handle) {
          self.open_requested_.reset();
        }
      }
      const auto row_start = ImGui::GetCursorScreenPos();
      const auto row_width = ImGui::GetContentRegionAvail().x;
      ImGui::SetNextItemAllowOverlap();
      auto tree_flags =
       ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_FramePadding |
       ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
       ImGuiTreeNodeFlags_SpanAvailWidth;
      if(selected) {
        tree_flags |= ImGuiTreeNodeFlags_Selected;
      }
      const auto open = ImGui::TreeNodeEx(
       [&] {
         const auto has_draft =
          self.editing_handle_ == handle && self.editing_ &&
          !self.same_portfolio(*self.editing_, *portfolio);
         if(has_draft) {
           return portfolio->name() + " (Unsaved)";
         }
         return portfolio->name().empty() ? std::string{"Unnamed"}
                                          : portfolio->name();
       }()
        .c_str(),
       tree_flags);
      const auto row_clicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
      const auto arrow_toggled = ImGui::IsItemToggledOpen();
      const auto action =
       ui::resource_row_actions(row_start,
                                row_width,
                                index,
                                portfolio_handles.size(),
                                true,
                                app_state.is_portfolio_ready(*portfolio));
      if(action == ui::ResourceRowAction::Edit) {
        if(self.editing_handle_ != handle || !self.editing_) {
          self.editing_handle_ = handle;
          self.editing_ = self.make_editable_portfolio(*portfolio);
        }
        self.editor_open_ = true;
      } else if(action == ui::ResourceRowAction::Duplicate) {
        context.push_edit("Duplicate Portfolio",
                          [handle](ApplicationState& state) {
                            auto copy = state.get_portfolio(handle);
                            copy.name(copy.name() + " Copy");
                            state.add_portfolio(std::move(copy));
                          });
      } else if(action == ui::ResourceRowAction::Rerun) {
        context.push_edit("Rerun Portfolio", [handle](ApplicationState& state) {
          state.rerun_portfolio(handle);
        });
      } else if(action == ui::ResourceRowAction::MoveUp) {
        context.push_edit("Move Portfolio Up",
                          [index](ApplicationState& state) {
                            state.reorder_list_portfolio(index, index - 1);
                          });
      } else if(action == ui::ResourceRowAction::MoveDown) {
        context.push_edit("Move Portfolio Down",
                          [index](ApplicationState& state) {
                            state.reorder_list_portfolio(index, index + 1);
                          });
      } else if(action == ui::ResourceRowAction::Delete) {
        context.push_edit(
         "Delete Portfolio",
         [handle](ApplicationState& state) { state.remove_portfolio(handle); });
      } else if(row_clicked && !arrow_toggled) {
        self.open_requested_ = handle;
        context.push_view_action(
         [handle](ApplicationState& state) { state.select_portfolio(handle); });
      }
      if(open) {
        for(const auto run : runs) {
          const auto* backtest =
           app_state.get_backtest_if_present(run.backtest_handle);
          const auto* asset = app_state.get_asset_if_present(run.asset_handle);
          const auto name = backtest && asset
                             ? backtest->name() + " — " + asset->name()
                             : std::string{"Missing Backtest Asset"};
          if(!portfolio_matches &&
             !(backtest && self.filter_.PassFilter(backtest->name().c_str())) &&
             !(asset && self.filter_.PassFilter(asset->name().c_str()))) {
            continue;
          }

          ImGui::PushID(static_cast<int>(run.backtest_handle.slot_index()));
          ImGui::PushID(static_cast<int>(run.backtest_handle.generation()));
          ImGui::PushID(static_cast<int>(run.asset_handle.slot_index()));
          ImGui::PushID(static_cast<int>(run.asset_handle.generation()));
          const auto child_selected =
           selected_backtest && *selected_backtest == run;
          if(backtest && asset) {
            if(ImGui::Selectable(name.c_str(), child_selected)) {
              context.push_view_action([handle, run](ApplicationState& state) {
                state.select_portfolio_backtest(handle, run);
              });
            }
          } else {
            ImGui::BeginDisabled();
            ImGui::Selectable("Missing Backtest", false);
            ImGui::EndDisabled();
          }
          ImGui::PopID();
          ImGui::PopID();
          ImGui::PopID();
          ImGui::PopID();
        }
        ImGui::TreePop();
      }
      ImGui::Separator();
      ImGui::PopID();
      ImGui::PopID();
    }

    if(portfolio_handles.empty()) {
      ImGui::TextDisabled("No portfolios yet. Create one to get started.");
    } else if(visible_portfolio_count == 0) {
      ImGui::TextDisabled("No portfolios match this search.");
    }

    ImGui::EndChild();
  }

  void render_editor(this PortfoliosWindow& self, WindowContext& context)
  {
    auto& app_state = context.app_state();
    auto& portfolio = *self.editing_;

    ImGui::BeginGroup();
    ImGui::BeginChild("item view",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
    ImGui::TextUnformatted(self.editing_handle_ ? "Edit Portfolio"
                                                : "Add New Portfolio");
    ImGui::Separator();

    ui::form_section(
     "Portfolio Account",
     "Configure the shared capital, market, broker, and account policies.");
    auto name = portfolio.name();
    ui::field_label("Name");
    ImGui::InputTextWithHint("##portfolio_name", "Unnamed", &name);
    portfolio.name(std::move(name));

    auto capital = portfolio.initial_capital();
    ui::field_label("Initial capital");
    if(ImGui::InputDouble("##portfolio_capital", &capital, 100.0, 1000.0) &&
       std::isfinite(capital) && capital >= 0.0) {
      portfolio.initial_capital(capital);
    }

    self.market_combo(app_state, portfolio);
    self.broker_combo(app_state, portfolio);

    auto maximum_open_trades = portfolio.maximum_open_trades();
    ui::field_label("Maximum open trades");
    constexpr auto size_type = sizeof(std::size_t) == sizeof(std::uint64_t)
                                ? ImGuiDataType_U64
                                : ImGuiDataType_U32;
    constexpr auto step = std::size_t{1};
    if(ImGui::InputScalar("##portfolio_maximum_open_trades",
                          size_type,
                          &maximum_open_trades,
                          &step)) {
      portfolio.maximum_open_trades(
       std::max(maximum_open_trades, std::size_t{1}));
    }

    auto maximum_combined_layers = portfolio.maximum_combined_layers();
    ui::field_label("Maximum combined layers");
    if(ImGui::InputScalar("##portfolio_maximum_combined_layers",
                          size_type,
                          &maximum_combined_layers,
                          &step)) {
      portfolio.maximum_combined_layers(
       std::max(maximum_combined_layers, std::size_t{1}));
    }

    ui::form_section(
     "Entry Comparators",
     "Compare prepared requested orders in sequence using Requested Order "
     "values and asset OHLCV or custom DATA. Current Close sees the completed "
     "current bar. Next Open sees the previous completed bar; use Requested "
     "Order Price for the executable open. The first unequal value determines "
     "priority; complete ties use ordered Backtest and Watchlist order.");
    auto& comparators = portfolio.entry_comparators();
    auto comparator_index = std::size_t{};
    while(comparator_index < comparators.size()) {
      ImGui::PushID(static_cast<int>(comparator_index));
      ImGui::Text("Comparator %zu", comparator_index + 1);
      auto order = comparators[comparator_index].order();
      ui::field_label("Order");
      const auto* order_label =
       order == backtest::PortfolioEntryComparatorOrder::HigherFirst
        ? "Higher first"
        : "Lower first";
      if(ImGui::BeginCombo("##comparator_order", order_label)) {
        if(ImGui::Selectable(
            "Higher first",
            order == backtest::PortfolioEntryComparatorOrder::HigherFirst)) {
          order = backtest::PortfolioEntryComparatorOrder::HigherFirst;
        }
        if(ImGui::Selectable(
            "Lower first",
            order == backtest::PortfolioEntryComparatorOrder::LowerFirst)) {
          order = backtest::PortfolioEntryComparatorOrder::LowerFirst;
        }
        ImGui::EndCombo();
      }
      comparators[comparator_index].order(order);

      ui::field_label("Expression");
      auto expression = comparators[comparator_index].expression();
      self.series_node_editor_.render(expression, context);
      comparators[comparator_index].expression(std::move(expression));

      auto erased = false;
      if(comparator_index > 0 && ImGui::Button("Move up")) {
        std::swap(comparators[comparator_index],
                  comparators[comparator_index - 1]);
      }
      if(comparator_index + 1 < comparators.size()) {
        ImGui::SameLine();
        if(ImGui::Button("Move down")) {
          std::swap(comparators[comparator_index],
                    comparators[comparator_index + 1]);
        }
      }
      ImGui::SameLine();
      if(ImGui::Button(PLUDUX_ICON_DELETE " Remove")) {
        comparators.erase(comparators.begin() +
                          static_cast<std::ptrdiff_t>(comparator_index));
        erased = true;
      }
      ImGui::Separator();
      ImGui::PopID();
      if(!erased) {
        ++comparator_index;
      }
    }
    if(ImGui::Button(PLUDUX_ICON_ADD " Add comparator")) {
      comparators.emplace_back();
    }

    ui::form_section(
     "Ordered Backtests",
     "Order is the final deterministic tie-breaker when entries compare "
     "equally.");
    auto handles = portfolio.backtest_handles();
    const auto visible_rows = std::clamp<std::size_t>(handles.size(), 1, 6);
    const auto list_height =
     static_cast<float>(visible_rows) * ImGui::GetFrameHeightWithSpacing() +
     (2.0f * ImGui::GetStyle().WindowPadding.y);
    ImGui::BeginChild(
     "ordered_backtests", ImVec2{0.0f, list_height}, ImGuiChildFlags_Borders);
    if(handles.empty()) {
      ImGui::TextDisabled("No backtests selected.");
    }
    auto index = std::size_t{};
    while(index < handles.size()) {
      const auto handle = handles[index];
      const auto* backtest = app_state.get_backtest_if_present(handle);
      ImGui::PushID(static_cast<int>(index));
      const auto action = self.ordered_backtest_row(
       backtest ? backtest->name().c_str() : "Missing Backtest",
       !backtest,
       index,
       handles.size());
      if(action == ui::ResourceRowAction::MoveUp) {
        std::swap(handles[index], handles[index - 1]);
      } else if(action == ui::ResourceRowAction::MoveDown) {
        std::swap(handles[index], handles[index + 1]);
      } else if(action == ui::ResourceRowAction::Delete) {
        handles.erase(handles.begin() + static_cast<std::ptrdiff_t>(index));
        ImGui::PopID();
        continue;
      }
      ImGui::PopID();
      ++index;
    }
    ImGui::EndChild();

    self.backtest_picker(app_state, handles);
    portfolio.backtest_handles(std::move(handles));

    if(!app_state.is_portfolio_ready(portfolio)) {
      ImGui::Spacing();
      ImGui::PushTextWrapPos(ImGui::GetCursorPosX() +
                             ImGui::GetContentRegionAvail().x);
      ImGui::TextDisabled(
       PLUDUX_ICON_WARNING
       " This Portfolio can be saved, but it cannot run until a Market, "
       "Broker, and at least one complete Backtest are configured.");
      ImGui::PopTextWrapPos();
    }

    ImGui::EndChild();

    if(self.editing_handle_) {
      const auto* stored =
       app_state.get_portfolio_if_present(*self.editing_handle_);
      const auto changed = stored && !self.same_portfolio(*stored, portfolio);
      if(ImGui::Button("OK")) {
        if(changed) {
          self.submit_changes(context, true);
        } else {
          self.discard_draft();
        }
      }
      ImGui::SameLine();
      if(ImGui::Button("Cancel")) {
        self.editor_open_ = false;
      }
      ImGui::SameLine();
      const auto draft_action = ui::apply_reset_button(changed);
      if(draft_action == ui::DraftAction::Apply) {
        self.submit_changes(context, false);
      } else if(draft_action == ui::DraftAction::Reset && stored) {
        self.editing_ = self.make_editable_portfolio(*stored);
      }
    } else {
      if(ImGui::Button("Create Portfolio")) {
        self.submit_changes(context, true);
      }
      ImGui::SameLine();
      if(ImGui::Button("Cancel")) {
        self.discard_draft();
      }
    }
    ImGui::EndGroup();
  }

  void submit_changes(this PortfoliosWindow& self,
                      WindowContext& context,
                      bool close_editor)
  {
    const auto handle = self.editing_handle_;
    const auto value = *self.editing_;
    context.push_edit(handle ? "Edit Portfolio" : "Add Portfolio",
                      [handle, value](ApplicationState& state) {
                        if(handle) {
                          state.update_portfolio(*handle, value);
                          state.select_portfolio(*handle);
                        } else if(const auto created =
                                   state.add_portfolio(value)) {
                          state.select_portfolio(*created);
                        }
                      });
    if(close_editor) {
      self.discard_draft();
    }
  }

  static auto same_portfolio(const backtest::Portfolio& lhs,
                             const backtest::Portfolio& rhs) noexcept -> bool
  {
    return lhs.name() == rhs.name() && lhs.equivalent_rules(rhs);
  }

  static auto ordered_backtest_row(const char* name,
                                   bool missing,
                                   std::size_t index,
                                   std::size_t count) -> ui::ResourceRowAction
  {
    const auto row_start = ImGui::GetCursorScreenPos();
    const auto row_width = ImGui::GetContentRegionAvail().x;
    const auto row_height = ImGui::GetFrameHeight();
    ImGui::SetNextItemAllowOverlap();
    ImGui::Selectable("##ordered_backtest_row",
                      false,
                      ImGuiSelectableFlags_AllowOverlap,
                      ImVec2{row_width, row_height});

    const auto& style = ImGui::GetStyle();
    const auto button_width =
     ImGui::CalcTextSize(PLUDUX_ICON_MOVE_UP).x + (2.0f * style.FramePadding.x);
    const auto buttons_width =
     (3.0f * button_width) + (2.0f * style.ItemSpacing.x);
    const auto buttons_start_x =
     std::max(row_start.x, row_start.x + row_width - buttons_width);

    ImGui::SetCursorScreenPos(row_start);
    ImGui::AlignTextToFramePadding();
    if(missing) {
      ImGui::TextDisabled("%s", name);
    } else {
      ImGui::TextUnformatted(name);
    }

    auto action = ui::ResourceRowAction::None;
    ImGui::SetCursorScreenPos(ImVec2{buttons_start_x, row_start.y});
    ImGui::BeginDisabled(index == 0);
    if(ui::icon_button(PLUDUX_ICON_MOVE_UP "##move_up", "Move up")) {
      action = ui::ResourceRowAction::MoveUp;
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(index + 1 == count);
    if(ui::icon_button(PLUDUX_ICON_MOVE_DOWN "##move_down", "Move down")) {
      action = ui::ResourceRowAction::MoveDown;
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if(ui::icon_button(PLUDUX_ICON_DELETE "##remove", "Remove")) {
      action = ui::ResourceRowAction::Delete;
    }
    ImGui::Separator();
    return action;
  }

  void backtest_picker(this PortfoliosWindow& self,
                       const ApplicationState& app_state,
                       std::vector<backtest::BacktestStoreHandle>& selected)
  {
    const auto button_width = ImGui::GetContentRegionAvail().x;
    if(ImGui::Button(PLUDUX_ICON_ADD " Add Backtests...",
                     ImVec2{button_width, 0.0f})) {
      self.backtest_picker_filter_.Clear();
      ImGui::OpenPopup("add_backtests");
    }

    if(!ImGui::BeginPopup("add_backtests")) {
      return;
    }

    ImGui::TextUnformatted("Add Backtests");
    ImGui::TextDisabled("Choose any number of backtests in priority order.");
    ui::search_filter(self.backtest_picker_filter_,
                      "##portfolio_backtest_search");

    const auto& available = app_state.get_backtest_handles();
    auto addable_count = std::size_t{};
    for(const auto handle : available) {
      const auto* backtest = app_state.get_backtest_if_present(handle);
      if(backtest &&
         self.backtest_picker_filter_.PassFilter(backtest->name().c_str()) &&
         std::ranges::find(selected, handle) == selected.end()) {
        ++addable_count;
      }
    }

    ImGui::BeginDisabled(addable_count == 0);
    if(ImGui::Button("Add all matches")) {
      for(const auto handle : available) {
        const auto* backtest = app_state.get_backtest_if_present(handle);
        if(backtest &&
           self.backtest_picker_filter_.PassFilter(backtest->name().c_str()) &&
           std::ranges::find(selected, handle) == selected.end()) {
          selected.push_back(handle);
        }
      }
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if(ImGui::Button("Done")) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::Separator();

    if(addable_count == 0) {
      ImGui::TextDisabled("No matching Backtests available.");
    }
    for(const auto handle : available) {
      const auto* backtest = app_state.get_backtest_if_present(handle);
      if(!backtest ||
         !self.backtest_picker_filter_.PassFilter(backtest->name().c_str()) ||
         std::ranges::find(selected, handle) != selected.end()) {
        continue;
      }
      ImGui::PushID(static_cast<int>(handle.slot_index()));
      ImGui::PushID(static_cast<int>(handle.generation()));
      if(ImGui::Selectable(backtest->name().c_str(),
                           false,
                           ImGuiSelectableFlags_NoAutoClosePopups)) {
        selected.push_back(handle);
      }
      ImGui::PopID();
      ImGui::PopID();
    }
    ImGui::EndPopup();
  }

  static void market_combo(const ApplicationState& app_state,
                           backtest::Portfolio& portfolio)
  {
    const auto* current =
     app_state.get_market_if_present(portfolio.market_handle());
    ui::field_label("Market");
    if(ImGui::BeginCombo("##portfolio_market",
                         current ? current->name().c_str() : "Select market")) {
      for(const auto handle : app_state.get_market_handles()) {
        const auto& market = app_state.get_market(handle);
        if(ImGui::Selectable(market.name().c_str(),
                             handle == portfolio.market_handle())) {
          portfolio.market_handle(handle);
        }
      }
      ImGui::EndCombo();
    }
  }

  static void broker_combo(const ApplicationState& app_state,
                           backtest::Portfolio& portfolio)
  {
    const auto* current =
     app_state.get_broker_if_present(portfolio.broker_handle());
    ui::field_label("Broker");
    if(ImGui::BeginCombo("##portfolio_broker",
                         current ? current->name().c_str() : "Select broker")) {
      for(const auto handle : app_state.get_broker_handles()) {
        const auto& broker = app_state.get_broker(handle);
        if(ImGui::Selectable(broker.name().c_str(),
                             handle == portfolio.broker_handle())) {
          portfolio.broker_handle(handle);
        }
      }
      ImGui::EndCombo();
    }
  }
};

} // namespace pludux::apps
