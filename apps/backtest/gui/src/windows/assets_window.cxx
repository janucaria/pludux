module;

#include <string>

#include <imgui.h>

export module pludux.apps.backtest:windows.assets_window;

import :window_context;

export namespace pludux::apps {

class AssetsWindow {
public:
  void render(this const auto, WindowContext& context)
  {
    auto& assets = context.assets();

    ImGui::Begin("Assets", nullptr);

    if(!assets.empty()) {
      for(auto i = 0; i < assets.size(); ++i) {
        const auto& asset = *assets[i];
        const auto& asset_name = asset.name();

        ImGui::PushID(i);

        ImGui::SetNextItemAllowOverlap();

        ImGui::Text("%s", asset_name.c_str());

        ImGui::SameLine();

        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 50);
        if(ImGui::Button("Delete")) {
          context.push_action([i](ApplicationState& app_state) {
            app_state.remove_asset_at_index(i);
          });
        }

        ImGui::PopID();
      }
    }

    ImGui::End();
  }
};

} // namespace pludux::apps
