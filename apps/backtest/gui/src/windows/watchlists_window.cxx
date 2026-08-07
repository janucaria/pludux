module;

#include <algorithm>
#include <cstddef>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include "../ui/pludux_icons.hpp"

export module pludux.apps.backtest:windows.watchlists_window;

import pludux.backtest;
import :ui.widgets;
import :window_context;

export namespace pludux::apps {

class WatchlistsWindow {
public:
  void render(this WatchlistsWindow& self, WindowContext& context)
  {
    ImGui::Begin("Watchlists");
    if(self.editing_) {
      self.render_editor(context);
    } else {
      self.render_list(context);
    }
    ImGui::End();
  }

  void discard_draft(this WatchlistsWindow& self) noexcept
  {
    self.editing_.reset();
    self.editing_handle_.reset();
  }

private:
  std::optional<backtest::WatchlistStoreHandle> editing_handle_;
  std::shared_ptr<backtest::Watchlist> editing_;
  ImGuiTextFilter filter_;

  void render_list(this WatchlistsWindow& self, WindowContext& context)
  {
    auto& state = context.app_state();
    if(ImGui::Button(PLUDUX_ICON_ADD " New Watchlist")) {
      self.editing_handle_.reset();
      self.editing_ = std::make_shared<backtest::Watchlist>();
      return;
    }

    ImGui::Spacing();
    ui::search_filter(self.filter_, "##watchlists_search");
    ImGui::BeginChild("watchlists_list");
    const auto& handles = state.get_watchlist_handles();
    auto visible = std::size_t{};
    for(auto index = std::size_t{}; index < handles.size(); ++index) {
      const auto handle = handles[index];
      const auto* watchlist = state.get_watchlist_if_present(handle);
      if(!watchlist || !self.filter_.PassFilter(watchlist->name().c_str())) {
        continue;
      }
      ++visible;
      ImGui::PushID(static_cast<int>(handle.slot_index()));
      ImGui::PushID(static_cast<int>(handle.generation()));
      const auto row_start = ImGui::GetCursorScreenPos();
      const auto row_width = ImGui::GetContentRegionAvail().x;
      ImGui::TextUnformatted(
       watchlist->name().empty() ? "Unnamed" : watchlist->name().c_str());
      ImGui::TextDisabled("%zu asset%s",
                          watchlist->asset_handles().size(),
                          watchlist->asset_handles().size() == 1 ? "" : "s");
      const auto action = ui::resource_row_actions(
       row_start, row_width, index, handles.size(), true, true);
      if(action == ui::ResourceRowAction::Edit) {
        self.editing_handle_ = handle;
        self.editing_ = std::make_shared<backtest::Watchlist>(*watchlist);
      } else if(action == ui::ResourceRowAction::Duplicate) {
        context.push_edit("Duplicate Watchlist", [handle](ApplicationState& s) {
          auto copy = s.get_watchlist(handle);
          copy.name(copy.name() + " Copy");
          s.add_watchlist(std::move(copy));
        });
      } else if(action == ui::ResourceRowAction::MoveUp) {
        context.push_edit("Move Watchlist Up", [index](ApplicationState& s) {
          s.reorder_list_watchlist(index, index - 1);
        });
      } else if(action == ui::ResourceRowAction::MoveDown) {
        context.push_edit("Move Watchlist Down", [index](ApplicationState& s) {
          s.reorder_list_watchlist(index, index + 1);
        });
      } else if(action == ui::ResourceRowAction::Delete) {
        context.push_edit("Delete Watchlist", [handle](ApplicationState& s) {
          s.remove_watchlist(handle);
        });
      }
      ImGui::Separator();
      ImGui::PopID();
      ImGui::PopID();
    }
    if(handles.empty()) {
      ImGui::TextDisabled("No watchlists yet. Create one to get started.");
    } else if(visible == 0) {
      ImGui::TextDisabled("No watchlists match this search.");
    }
    ImGui::EndChild();
  }

  void render_editor(this WatchlistsWindow& self, WindowContext& context)
  {
    auto& state = context.app_state();
    ImGui::TextUnformatted(self.editing_handle_ ? "Edit Watchlist"
                                                : "Add New Watchlist");
    ui::field_label("Name");
    auto name = self.editing_->name();
    if(ImGui::InputText("##watchlist_name", &name)) {
      self.editing_->name(std::move(name));
    }
    ImGui::SeparatorText("Assets");

    auto selected = self.editing_->asset_handles();
    for(auto index = std::size_t{}; index < selected.size();) {
      const auto handle = selected[index];
      const auto* asset = state.get_asset_if_present(handle);
      ImGui::PushID(static_cast<int>(index));
      ImGui::TextUnformatted(asset ? asset->name().c_str() : "Missing Asset");
      ImGui::SameLine();
      ImGui::BeginDisabled(index == 0);
      if(ui::icon_button(PLUDUX_ICON_MOVE_UP "##up", "Move up")) {
        std::swap(selected[index], selected[index - 1]);
      }
      ImGui::EndDisabled();
      ImGui::SameLine();
      ImGui::BeginDisabled(index + 1 == selected.size());
      if(ui::icon_button(PLUDUX_ICON_MOVE_DOWN "##down", "Move down")) {
        std::swap(selected[index], selected[index + 1]);
      }
      ImGui::EndDisabled();
      ImGui::SameLine();
      const auto remove =
       ui::icon_button(PLUDUX_ICON_DELETE "##remove", "Remove");
      ImGui::PopID();
      if(remove) {
        selected.erase(selected.begin() + static_cast<std::ptrdiff_t>(index));
      } else {
        ++index;
      }
    }

    if(ImGui::BeginCombo("##add_watchlist_asset", "Add Asset...")) {
      for(const auto handle : state.get_asset_handles()) {
        if(std::ranges::find(selected, handle) != selected.end()) {
          continue;
        }
        const auto& asset = state.get_asset(handle);
        if(ImGui::Selectable(asset.name().c_str())) {
          selected.push_back(handle);
        }
      }
      ImGui::EndCombo();
    }
    self.editing_->asset_handles(std::move(selected));

    ImGui::Spacing();
    if(ImGui::Button(self.editing_handle_ ? "Save Changes"
                                          : "Create Watchlist")) {
      auto draft = *self.editing_;
      if(draft.name().empty()) {
        draft.name("Unnamed");
      }
      if(self.editing_handle_) {
        const auto handle = *self.editing_handle_;
        context.push_edit("Edit Watchlist",
                          [handle, draft](ApplicationState& s) {
                            s.update_watchlist(handle, draft);
                          });
      } else {
        context.push_edit("Add Watchlist", [draft](ApplicationState& s) {
          s.add_watchlist(draft);
        });
      }
      self.discard_draft();
    }
    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.discard_draft();
    }
  }
};

} // namespace pludux::apps
