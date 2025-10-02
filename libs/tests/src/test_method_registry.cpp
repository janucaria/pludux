#include <gtest/gtest.h>

import pludux;

using namespace pludux;
using namespace pludux::screener;

TEST(MethodRegistryTest, Constructors)
{
  auto registry1 = MethodRegistry{};
  EXPECT_TRUE(registry1.empty());
}

TEST(MethodRegistryTest, SetGetHasRemove)
{
  auto registry = MethodRegistry{};

  EXPECT_FALSE(registry.has("close"));
  EXPECT_FALSE(registry.has("open"));

  registry.set("close", CloseMethod{});
  EXPECT_TRUE(registry.has("close"));
  EXPECT_FALSE(registry.has("open"));

  registry.set("open", OpenMethod{});
  EXPECT_TRUE(registry.has("close"));
  EXPECT_TRUE(registry.has("open"));

  auto close_method_opt = registry.get("close");
  ASSERT_TRUE(close_method_opt.has_value());
  EXPECT_EQ(*close_method_opt, CloseMethod{});

  auto open_method_opt = registry.get("open");
  ASSERT_TRUE(open_method_opt.has_value());
  EXPECT_EQ(*open_method_opt, OpenMethod{});

  auto not_found_opt = registry.get("NON_EXIST");
  EXPECT_FALSE(not_found_opt.has_value());

  auto removed_close_opt = registry.remove("close");
  ASSERT_TRUE(removed_close_opt.has_value());
  EXPECT_EQ(*removed_close_opt, CloseMethod{});
  EXPECT_FALSE(registry.has("close"));

  auto removed_open_opt = registry.remove("open");
  ASSERT_TRUE(removed_open_opt.has_value());
  EXPECT_EQ(*removed_open_opt, OpenMethod{});
  EXPECT_FALSE(registry.has("open"));

  auto removed_not_found_opt = registry.remove("NON_EXIST");
  EXPECT_FALSE(removed_not_found_opt.has_value());
}

TEST(MethodRegistryTest, Rename)
{
  auto registry = MethodRegistry{};

  registry.set("close", CloseMethod{});
  registry.set("open", OpenMethod{});

  EXPECT_TRUE(registry.has("close"));
  EXPECT_TRUE(registry.has("open"));

  auto rename_result = registry.rename("close", "closing_price");
  EXPECT_TRUE(rename_result);
  EXPECT_FALSE(registry.has("close"));
  EXPECT_TRUE(registry.has("closing_price"));

  auto closing_price_opt = registry.get("closing_price");
  ASSERT_TRUE(closing_price_opt.has_value());
  EXPECT_EQ(*closing_price_opt, CloseMethod{});

  auto rename_non_exist = registry.rename("NON_EXIST", "new_name");
  EXPECT_FALSE(rename_non_exist);

  auto rename_to_existing = registry.rename("open", "closing_price");
  EXPECT_TRUE(rename_to_existing);
  EXPECT_FALSE(registry.has("open"));
  EXPECT_TRUE(registry.has("closing_price"));
  EXPECT_EQ(registry.size(), 1);
  EXPECT_EQ(*registry.get("closing_price"), OpenMethod{});
}

TEST(MethodRegistryTest, IterateOrderedByInsertion)
{
  auto registry = MethodRegistry{};

  registry.set("close", CloseMethod{});
  registry.set("open", OpenMethod{});
  registry.set("high", HighMethod{});
  registry.set("low", LowMethod{});
  registry.set("volume", VolumeMethod{});
  registry.set("value", ValueMethod{42.0});

  std::vector<std::string> expected_order = {
   "close", "open", "high", "low", "volume", "value"};

  std::vector<std::string> actual_order;
  for(auto& [name, method] : registry) {
    actual_order.push_back(name);
  }

  EXPECT_EQ(actual_order, expected_order);

  const auto const_registry = registry;
  actual_order.clear();
  for(const auto& [name, method] : const_registry) {
    actual_order.push_back(name);
  }
  EXPECT_EQ(actual_order, expected_order);
}