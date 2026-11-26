#include <gtest/gtest.h>

#include "../src/charts/chart_bar.hpp"

namespace {

class ChartBarFill : public testing::Test {
protected:
  void SetUp() override
  {
    ImGui::CreateContext();
    auto& io = ImGui::GetIO();
    io.DisplaySize = {640, 480};
    io.IniFilename = nullptr;
    io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
    ImGui::NewFrame();
  }

  void TearDown() override
  {
    ImGui::EndFrame();
    ImGui::DestroyContext();
  }
};

TEST_F(ChartBarFill, DrawsFillWithEitherVerticalCornerOrder)
{
  auto& draw_list = *ImGui::GetBackgroundDrawList();

  pludux::apps::fill_chart_bar(draw_list, {10, 10}, {20, 30}, IM_COL32_WHITE);
  const auto filled_index_count = draw_list.IdxBuffer.Size;
  ASSERT_GT(filled_index_count, 0);

  pludux::apps::fill_chart_bar(draw_list, {10, 30}, {20, 10}, IM_COL32_WHITE);
  EXPECT_EQ(draw_list.IdxBuffer.Size, 2 * filled_index_count);
}

} // namespace
