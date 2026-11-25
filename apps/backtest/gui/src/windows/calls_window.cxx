module;

#include <algorithm>
#include <array>
#include <cstddef>
#include <exception>
#include <fstream>
#include <functional>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#ifdef __EMSCRIPTEN__
#include "../emscripten_js_imports.hpp"
#else
#include <nfd.hpp>
#endif

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

export module pludux.apps.backtest:windows.calls_window;

import pludux.backtest;
import pludux.apps.backtest.application_state;
import :window_context;

export namespace pludux::apps {

class CallsWindow {
public:
  void discard_draft()
  {
    selected_ = {};
    name_handle_ = {};
    pending_.clear();
    error_.clear();
#ifdef __EMSCRIPTEN__
    upload_active_ = false;
#endif
  }

  void render(WindowContext& context)
  {
    for(auto& pending : pending_) {
      context.push_edit(pending.first.valid() ? "Replace Call Table"
                                              : "Import Call Table",
                        [edit = std::move(pending)](ApplicationState& state) {
                          if(edit.first.valid()) {
                            state.update_call(edit.first, edit.second);
                          } else {
                            state.add_call(edit.second);
                          }
                        });
    }
    pending_.clear();
    if(!ImGui::Begin("Calls")) {
      ImGui::End();
      return;
    }
    const auto& state = context.app_state();
    if(!state.get_call_if_present(selected_) &&
       !state.get_call_handles().empty()) {
      selected_ = state.get_call_handles().front();
    }
    const auto* table = state.get_call_if_present(selected_);
    if(ImGui::Button("Import CSV"))
      open_csv({}, "");
    if(table) {
      ImGui::SameLine();
      if(ImGui::Button("Replace from CSV"))
        open_csv(selected_, table->name);
      ImGui::SameLine();
      if(ImGui::Button("Delete Table"))
        ImGui::OpenPopup("Delete Call Table");
      if(ImGui::BeginPopupModal(
          "Delete Call Table", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Delete %s?", table->name.c_str());
        if(ImGui::Button("Delete")) {
          context.push_edit("Delete Call Table",
                            [handle = selected_](ApplicationState& state) {
                              state.remove_call(handle);
                            });
          selected_ = {};
          ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if(ImGui::Button("Cancel"))
          ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
      }
    }
    if(!error_.empty())
      ImGui::TextWrapped("%s", error_.c_str());
    if(ImGui::BeginCombo("Table",
                         table ? table->name.c_str() : "Select a table")) {
      for(const auto handle : state.get_call_handles()) {
        const auto* candidate = state.get_call_if_present(handle);
        if(!candidate)
          continue;
        ImGui::PushID(static_cast<int>(handle.slot_index()));
        if(ImGui::Selectable(candidate->name.c_str(), handle == selected_)) {
          selected_ = handle;
          first_column_ = 0;
          name_ = candidate->name;
        }
        ImGui::PopID();
      }
      ImGui::EndCombo();
    }
    table = state.get_call_if_present(selected_);
    if(table) {
      if(!ImGui::IsAnyItemActive() &&
         (name_handle_ != selected_ || saved_name_ != table->name)) {
        name_handle_ = selected_;
        saved_name_ = table->name;
        name_ = table->name;
      }
      ImGui::InputText("Name", &name_);
      ImGui::SameLine();
      if(ImGui::Button("Rename") && !name_.empty() && name_ != table->name) {
        context.push_edit(
         "Rename Call Table",
         [handle = selected_, name = name_](ApplicationState& state) {
           if(const auto* original = state.get_call_if_present(handle)) {
             auto copy = *original;
             copy.name = name;
             state.update_call(handle, std::move(copy));
           }
         });
      }
      ImGui::Text("%zu rows", table->rows.size());
      if(ImGui::BeginChild("Call table preview", ImVec2{0.0f, 0.0f})) {
        render_table(*table);
      }
      ImGui::EndChild();
    }
    ImGui::End();
  }

private:
  using Pending = std::pair<backtest::CallStoreHandle, backtest::CallTable>;
  backtest::CallStoreHandle selected_;
  backtest::CallStoreHandle name_handle_;
  std::vector<Pending> pending_;
  std::string name_;
  std::string saved_name_;
  std::string error_;

#ifdef __EMSCRIPTEN__
  backtest::CallStoreHandle upload_handle_;
  std::string upload_name_;
  bool upload_active_{};
#endif

  std::size_t first_column_ = 0;

  void render_table(const backtest::CallTable& table)
  {
    // ImGui's table column limit includes the row-number column.
    constexpr std::size_t page_size = 63;
    if(first_column_ >= table.headers.size())
      first_column_ = 0;
    if(table.headers.size() > page_size) {
      ImGui::BeginDisabled(first_column_ == 0);
      if(ImGui::Button("Previous columns"))
        first_column_ -= page_size;
      ImGui::EndDisabled();
      ImGui::SameLine();
      ImGui::BeginDisabled(first_column_ + page_size >= table.headers.size());
      if(ImGui::Button("Next columns"))
        first_column_ += page_size;
      ImGui::EndDisabled();
    }
    const auto count =
     std::min(page_size, table.headers.size() - first_column_);
    if(!ImGui::BeginTable("Call table",
                          static_cast<int>(count + 1),
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                           ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY |
                           ImGuiTableFlags_SizingFixedFit)) {
      return;
    }
    ImGui::TableSetupScrollFreeze(1, 1);
    ImGui::TableSetupColumn("Row");
    for(auto column = std::size_t{}; column < count; ++column) {
      ImGui::TableSetupColumn(table.headers[first_column_ + column].c_str());
    }
    ImGui::TableHeadersRow();
    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(table.rows.size()));
    while(clipper.Step()) {
      for(auto index = clipper.DisplayStart; index < clipper.DisplayEnd;
          ++index) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%d", index + 2);
        const auto& row = table.rows[index];
        for(auto column = std::size_t{}; column < count; ++column) {
          ImGui::TableSetColumnIndex(static_cast<int>(column + 1));
          ImGui::TextUnformatted(row[first_column_ + column].c_str());
        }
      }
    }
    ImGui::EndTable();
  }

  void load_csv(backtest::CallStoreHandle handle,
                std::string name,
                std::istream& input)
  {
    try {
      pending_.emplace_back(handle,
                            backtest::read_call_csv(input, std::move(name)));
      error_.clear();
    } catch(const std::exception& error) {
      error_ = error.what();
    }
  }

  void open_csv(backtest::CallStoreHandle handle, const std::string& name)
  {
#ifdef __EMSCRIPTEN__
    using Callback =
     std::function<void(const std::string&, const std::string&, void*)>;
    static const auto callback = Callback{[](const std::string& filename,
                                             const std::string& contents,
                                             void* target) {
      auto& window = *static_cast<CallsWindow*>(target);
      if(!window.upload_active_)
        return;
      if(window.upload_handle_.valid())
        window.upload_active_ = false;
      auto input = std::istringstream{contents};
      window.load_csv(window.upload_handle_,
                      window.upload_name_.empty() ? filename
                                                  : window.upload_name_,
                      input);
    }};
    upload_handle_ = handle;
    upload_name_ = name;
    upload_active_ = true;
    pludux_js_open_multiple_text_files(".csv", &callback, this);
#else
    const auto guard = NFD::Guard{};
    auto path = NFD::UniquePath{};
    constexpr auto filters =
     std::array<nfdfilteritem_t, 1>{{{"CSV files", "csv"}}};
    const auto result = NFD::OpenDialog(path, filters.data(), filters.size());
    if(result == NFD_ERROR) {
      error_ = NFD::GetError();
    } else if(result == NFD_OKAY) {
      const auto file = std::string{path.get()};
      auto input = std::ifstream{file, std::ios::binary};
      if(!input) {
        error_ = "Could not open the CSV file";
      } else {
        load_csv(handle,
                 name.empty() ? file.substr(file.find_last_of("/\\") + 1)
                              : name,
                 input);
      }
    }
#endif
  }
};

} // namespace pludux::apps
