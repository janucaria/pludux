#include <gtest/gtest.h>

#include <cmath>

import pludux.backtest;

using namespace pludux;
using namespace pludux::backtest;

TEST(TradeSessionTest, DefaultConstructor)
{
  auto session = TradeSession{};

  EXPECT_FALSE(session.open_position().has_value());
  EXPECT_TRUE(session.trade_records().empty());
  EXPECT_EQ(session.market_timestamp(), std::time_t{0});
  EXPECT_TRUE(std::isnan(session.market_price()));
  EXPECT_EQ(session.market_lookback(), std::size_t{0});

  EXPECT_DOUBLE_EQ(session.unrealized_pnl(), 0.0);
  EXPECT_DOUBLE_EQ(session.unrealized_investment(), 0.0);
  EXPECT_EQ(session.unrealized_duration(), 0);

  EXPECT_TRUE(session.is_flat());
  EXPECT_FALSE(session.is_open());
}

TEST(TradeSessionTest, BeginMarketBarUpdatesMarketAndClearsRecords)
{
  auto session = TradeSession{static_cast<std::time_t>(20), 100.0, 1};

  session.entry_position(TradeEntry{3.0, 100.0});
  session.exit_position(TradeExit{1.0, 130.0, TradeExit::Reason::signal});

  ASSERT_FALSE(session.trade_records().empty());

  session.begin_market_bar(static_cast<std::time_t>(25), 120.0, 2);

  EXPECT_EQ(session.market_timestamp(), std::time_t{25});
  EXPECT_DOUBLE_EQ(session.market_price(), 120.0);
  EXPECT_EQ(session.market_lookback(), std::size_t{2});
  EXPECT_TRUE(session.trade_records().empty());
  EXPECT_TRUE(session.open_position().has_value());
}

TEST(TradeSessionTest, EntryOpensLongPositionWithoutEmittingEvent)
{
  auto session = TradeSession{static_cast<std::time_t>(20), 100.0, 1};

  session.entry_position(TradeEntry{2.0, 100.0});

  ASSERT_TRUE(session.open_position().has_value());
  EXPECT_TRUE(session.trade_records().empty());
  EXPECT_DOUBLE_EQ(session.unrealized_pnl(), 0.0);
  EXPECT_DOUBLE_EQ(session.unrealized_investment(), 200.0);
  EXPECT_EQ(session.unrealized_duration(), 0);

  session.begin_market_bar(static_cast<std::time_t>(25), 105.0, 5);

  EXPECT_DOUBLE_EQ(session.unrealized_pnl(), 10.0);
  EXPECT_DOUBLE_EQ(session.unrealized_investment(), 200.0);
  EXPECT_EQ(session.unrealized_duration(), 5);
}

TEST(TradeSessionTest, EntryOpensShortPositionWithoutEmittingEvent)
{
  auto session = TradeSession{static_cast<std::time_t>(20), 100.0, 1};

  session.entry_position(TradeEntry{-2.0, 100.0});

  ASSERT_TRUE(session.open_position().has_value());
  EXPECT_TRUE(session.trade_records().empty());
  EXPECT_DOUBLE_EQ(session.unrealized_investment(), -200.0);

  session.begin_market_bar(static_cast<std::time_t>(25), 95.0, 2);

  EXPECT_DOUBLE_EQ(session.unrealized_pnl(), 10.0);
  EXPECT_EQ(session.unrealized_duration(), 5);
}

TEST(TradeSessionTest, ScaleInEmitsScaledInRecord)
{
  auto session = TradeSession{static_cast<std::time_t>(20), 100.0, 1};

  session.entry_position(
   TradeEntry{2.0, 100.0, 90.0, 120.0, 90.0, false, 120.0});
  session.begin_market_bar(static_cast<std::time_t>(25), 130.0, 5);
  session.entry_position(
   TradeEntry{1.0, 130.0, 95.0, 150.0, 95.0, false, 150.0});

  ASSERT_TRUE(session.open_position().has_value());
  ASSERT_EQ(session.trade_records().size(), 1);

  const auto& record = session.trade_records().back();
  EXPECT_TRUE(record.is_scaled_in());
  EXPECT_DOUBLE_EQ(record.position_size(), 2.0);
  EXPECT_DOUBLE_EQ(record.investment(), 200.0);
  EXPECT_EQ(record.entry_timestamp(), std::time_t{20});
  EXPECT_EQ(record.exit_timestamp(), std::time_t{25});
  EXPECT_DOUBLE_EQ(record.stop_price(), 90.0);
  EXPECT_DOUBLE_EQ(record.target_price(), 120.0);
  EXPECT_DOUBLE_EQ(record.stop_loss_price(), 90.0);
  EXPECT_DOUBLE_EQ(record.take_profit_price(), 120.0);

  EXPECT_DOUBLE_EQ(session.open_position()->stop_price(), 95.0);
  EXPECT_DOUBLE_EQ(session.open_position()->target_price(), 150.0);

  EXPECT_DOUBLE_EQ(session.unrealized_pnl(), 60.0);
  EXPECT_DOUBLE_EQ(session.unrealized_investment(), 330.0);
  EXPECT_EQ(session.unrealized_duration(), 5);
}

TEST(TradeSessionTest, PartialScaleOutEmitsRecordAndKeepsPositionOpen)
{
  auto session = TradeSession{static_cast<std::time_t>(20), 100.0, 1};

  session.entry_position(TradeEntry{3.0, 100.0});
  session.begin_market_bar(static_cast<std::time_t>(25), 120.0, 5);
  session.exit_position(TradeExit{1.0, 130.0, TradeExit::Reason::signal});

  ASSERT_TRUE(session.open_position().has_value());
  ASSERT_EQ(session.trade_records().size(), 1);

  const auto& record = session.trade_records().back();
  EXPECT_TRUE(record.is_closed_exit_signal());
  EXPECT_DOUBLE_EQ(record.position_size(), 1.0);
  EXPECT_DOUBLE_EQ(record.investment(), 100.0);
  EXPECT_DOUBLE_EQ(record.pnl(), 30.0);

  EXPECT_DOUBLE_EQ(session.unrealized_pnl(), 40.0);
  EXPECT_DOUBLE_EQ(session.unrealized_investment(), 200.0);
}

TEST(TradeSessionTest, FullExitEmitsRecordAndClearsOpenPosition)
{
  auto session = TradeSession{static_cast<std::time_t>(20), 100.0, 1};

  session.entry_position(TradeEntry{2.0, 100.0});
  session.begin_market_bar(static_cast<std::time_t>(30), 105.0, 3);
  session.exit_position(TradeExit{2.0, 105.0, TradeExit::Reason::signal});

  EXPECT_FALSE(session.open_position().has_value());
  ASSERT_EQ(session.trade_records().size(), 1);

  const auto& record = session.trade_records().back();
  EXPECT_TRUE(record.is_closed_exit_signal());
  EXPECT_DOUBLE_EQ(record.position_size(), 2.0);
  EXPECT_DOUBLE_EQ(record.investment(), 200.0);
  EXPECT_DOUBLE_EQ(record.pnl(), 10.0);
  EXPECT_EQ(record.duration(), std::time_t{10});

  EXPECT_DOUBLE_EQ(session.unrealized_pnl(), 0.0);
  EXPECT_DOUBLE_EQ(session.unrealized_investment(), 0.0);
  EXPECT_EQ(session.unrealized_duration(), 0);
}

TEST(TradePositionTest, UpdatesTrailingStopAndChecksTriggers)
{
  auto position = TradePosition{2.0,
                                static_cast<std::time_t>(20),
                                100.0,
                                0.0,
                                90.0,
                                120.0,
                                90.0,
                                true,
                                120.0};

  position.update_trailing_stop(115.0);

  EXPECT_DOUBLE_EQ(position.stop_price(), 90.0);
  EXPECT_DOUBLE_EQ(position.stop_loss_price(), 105.0);
  EXPECT_TRUE(position.is_stop_loss_triggered(110.0, 104.0));
  EXPECT_TRUE(position.is_take_profit_triggered(121.0, 110.0));
}
