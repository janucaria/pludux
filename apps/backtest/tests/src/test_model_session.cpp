#include <gtest/gtest.h>

import pludux.backtest;

using namespace pludux::backtest;

TEST(ModelSessionTest, AggregatesPyramidingAndPartialExits)
{
  auto session = ModelSession{};
  session.begin_market_bar(1, 100.0);
  session.enter(ModelDirection::Long, 100.0);

  session.begin_market_bar(2, 110.0);
  session.enter(ModelDirection::Long, 110.0, true);
  session.exit(ModelIntentType::TakeProfit, 120.0, 0.5, 0);

  session.begin_market_bar(3, 90.0);
  session.exit(ModelIntentType::SignalExit, 90.0);

  ASSERT_TRUE(session.is_flat());
  ASSERT_EQ(session.closed_positions().size(), 1);
  const auto& position = session.closed_positions().front();
  EXPECT_DOUBLE_EQ(position.normalized_entry_quantity(), 2.0);
  EXPECT_DOUBLE_EQ(position.normalized_entry_notional(), 210.0);
  EXPECT_DOUBLE_EQ(position.directional_price_pnl(), 0.0);
  EXPECT_DOUBLE_EQ(position.return_ratio(), 0.0);
  EXPECT_EQ(position.intents().size(), 4);
}

TEST(ModelSessionTest, ShortReturnUsesShortDirection)
{
  auto session = ModelSession{};
  session.begin_market_bar(1, 100.0);
  session.enter(ModelDirection::Short, 100.0);
  session.begin_market_bar(2, 90.0);
  session.exit(ModelIntentType::SignalExit, 90.0);

  ASSERT_EQ(session.closed_positions().size(), 1);
  EXPECT_DOUBLE_EQ(session.closed_positions().front().price_pnl(), 10.0);
  EXPECT_DOUBLE_EQ(session.closed_positions().front().return_ratio(), 0.10);
}

TEST(ModelSessionTest, OpenPositionAtDatasetEndStaysUnrealized)
{
  auto session = ModelSession{};
  session.begin_market_bar(1, 100.0);
  session.enter(ModelDirection::Long, 100.0);
  session.begin_market_bar(2, 125.0);

  ASSERT_TRUE(session.position().has_value());
  EXPECT_DOUBLE_EQ(session.position()->unrealized_price_pnl(), 25.0);
  EXPECT_DOUBLE_EQ(session.position()->unrealized_return_ratio(), 0.25);
  EXPECT_TRUE(session.closed_positions().empty());
}
