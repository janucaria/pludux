#include <gtest/gtest.h>

import pludux.backtest;

using namespace pludux;
using namespace pludux::backtest;

TEST(TradeSessionTest, DefaultConstructor)
{
  auto session = TradeSession{};

  EXPECT_FALSE(session.open_position().has_value());
  EXPECT_FALSE(session.closed_position().has_value());
  EXPECT_EQ(session.market_timestamp(), std::time_t{0});
  EXPECT_TRUE(std::isnan(session.market_price()));
  EXPECT_EQ(session.market_index(), std::size_t{0});

  EXPECT_DOUBLE_EQ(session.unrealized_pnl(), 0.0);
  EXPECT_DOUBLE_EQ(session.partial_realized_pnl(), 0.0);
  EXPECT_DOUBLE_EQ(session.unrealized_investment(), 0.0);
  EXPECT_EQ(session.unrealized_duration(), 0);

  EXPECT_DOUBLE_EQ(session.realized_pnl(), 0.0);
  EXPECT_DOUBLE_EQ(session.realized_investment(), 0.0);
  EXPECT_EQ(session.realized_duration(), 0);

  EXPECT_TRUE(session.is_flat());
  EXPECT_FALSE(session.is_open());
  EXPECT_FALSE(session.is_closed());
  EXPECT_FALSE(session.is_reopen());
}

TEST(TradeSessionTest, ConstructorWithMarketParameters)
{
  auto session = TradeSession{static_cast<std::time_t>(1), 100.0, 0};

  EXPECT_FALSE(session.open_position().has_value());
  EXPECT_FALSE(session.closed_position().has_value());
  EXPECT_EQ(session.market_timestamp(), std::time_t{1});
  EXPECT_DOUBLE_EQ(session.market_price(), 100.0);
  EXPECT_EQ(session.market_index(), std::size_t{0});

  EXPECT_DOUBLE_EQ(session.unrealized_pnl(), 0.0);
  EXPECT_DOUBLE_EQ(session.partial_realized_pnl(), 0.0);
  EXPECT_DOUBLE_EQ(session.unrealized_investment(), 0.0);
  EXPECT_EQ(session.unrealized_duration(), 0);

  EXPECT_DOUBLE_EQ(session.realized_pnl(), 0.0);
  EXPECT_DOUBLE_EQ(session.realized_investment(), 0.0);
  EXPECT_EQ(session.realized_duration(), 0);

  EXPECT_TRUE(session.is_flat());
  EXPECT_FALSE(session.is_open());
  EXPECT_FALSE(session.is_closed());
  EXPECT_FALSE(session.is_reopen());
}

TEST(TradeSessionTest, OpenLongPosition)
{
  auto session = TradeSession{static_cast<std::time_t>(20), 100.0, 1};

  auto trade_entry = TradeEntry{2.0, 100.0};
  session.entry_position(trade_entry);

  EXPECT_TRUE(session.open_position().has_value());
  EXPECT_FALSE(session.closed_position().has_value());

  EXPECT_DOUBLE_EQ(session.unrealized_pnl(), 0.0);
  EXPECT_DOUBLE_EQ(session.partial_realized_pnl(), 0.0);
  EXPECT_DOUBLE_EQ(session.unrealized_investment(), 200.0);
  EXPECT_EQ(session.unrealized_duration(), 0);

  EXPECT_DOUBLE_EQ(session.realized_pnl(), 0.0);
  EXPECT_DOUBLE_EQ(session.realized_investment(), 0.0);
  EXPECT_EQ(session.realized_duration(), 0);

  EXPECT_FALSE(session.is_flat());
  EXPECT_TRUE(session.is_open());
  EXPECT_FALSE(session.is_closed());
  EXPECT_FALSE(session.is_reopen());

  session.market_update(static_cast<std::time_t>(25), 105.0, 5);

  EXPECT_DOUBLE_EQ(session.unrealized_pnl(), 10.0);
  EXPECT_DOUBLE_EQ(session.partial_realized_pnl(), 0.0);
  EXPECT_DOUBLE_EQ(session.unrealized_investment(), 200.0);
  EXPECT_EQ(session.unrealized_duration(), 5);
}

TEST(TradeSessionTest, CloseLongPositionWithExit)
{
  auto session = TradeSession{static_cast<std::time_t>(20), 100.0, 1};

  auto trade_entry = TradeEntry{2.0, 100.0};
  session.entry_position(trade_entry);

  session.market_update(static_cast<std::time_t>(30), 105.0, 3);

  auto trade_exit = TradeExit{2.0, 105.0, TradeExit::Reason::signal};
  session.exit_position(trade_exit);

  EXPECT_FALSE(session.open_position().has_value());
  EXPECT_TRUE(session.closed_position().has_value());

  EXPECT_DOUBLE_EQ(session.unrealized_pnl(), 0.0);
  EXPECT_DOUBLE_EQ(session.partial_realized_pnl(), 0.0);
  EXPECT_DOUBLE_EQ(session.unrealized_investment(), 0.0);
  EXPECT_EQ(session.unrealized_duration(), 0);

  EXPECT_DOUBLE_EQ(session.realized_pnl(), 10.0);
  EXPECT_DOUBLE_EQ(session.realized_investment(), 200.0);
  EXPECT_EQ(session.realized_duration(), 10);

  EXPECT_FALSE(session.is_flat());
  EXPECT_FALSE(session.is_open());
  EXPECT_TRUE(session.is_closed());
  EXPECT_FALSE(session.is_reopen());
}

TEST(TradeSessionTest, LongPositionScaleIn)
{
  auto session = TradeSession{static_cast<std::time_t>(20), 100.0, 1};

  auto first_entry = TradeEntry{2.0, 100.0};
  session.entry_position(first_entry);

  session.market_update(static_cast<std::time_t>(25), 130.0, 5);

  auto second_entry = TradeEntry{1.0, 130.0};
  session.entry_position(second_entry);

  EXPECT_TRUE(session.open_position().has_value());
  EXPECT_FALSE(session.closed_position().has_value());

  EXPECT_DOUBLE_EQ(session.unrealized_pnl(), 60.0);
  EXPECT_DOUBLE_EQ(session.partial_realized_pnl(), 0.0);
  EXPECT_DOUBLE_EQ(session.unrealized_investment(), 330.0);
  EXPECT_EQ(session.unrealized_duration(), 5);

  EXPECT_DOUBLE_EQ(session.realized_pnl(), 0.0);
  EXPECT_DOUBLE_EQ(session.realized_investment(), 0.0);
  EXPECT_EQ(session.realized_duration(), 0);

  EXPECT_FALSE(session.is_flat());
  EXPECT_TRUE(session.is_open());
  EXPECT_FALSE(session.is_closed());
  EXPECT_FALSE(session.is_reopen());

  session.market_update(static_cast<std::time_t>(30), 105.0, 7);
  EXPECT_DOUBLE_EQ(session.unrealized_pnl(), -15.0);
  EXPECT_DOUBLE_EQ(session.partial_realized_pnl(), 0.0);
  EXPECT_DOUBLE_EQ(session.unrealized_investment(), 330.0);
  EXPECT_EQ(session.unrealized_duration(), 10);
}

TEST(TradeSessionTest, LongPositionScaleOut)
{
  auto session = TradeSession{static_cast<std::time_t>(20), 100.0, 1};

  auto trade_entry = TradeEntry{3.0, 100.0};
  session.entry_position(trade_entry);

  session.market_update(static_cast<std::time_t>(25), 120.0, 5);

  auto trade_exit = TradeExit{1.0, 130.0, TradeExit::Reason::signal};
  session.exit_position(trade_exit);

  EXPECT_TRUE(session.open_position().has_value());
  EXPECT_FALSE(session.closed_position().has_value());

  EXPECT_DOUBLE_EQ(session.unrealized_pnl(), 40.0);
  EXPECT_DOUBLE_EQ(session.partial_realized_pnl(), 30.0);
  EXPECT_DOUBLE_EQ(session.unrealized_investment(), 200.0);
  EXPECT_EQ(session.unrealized_duration(), 5);

  EXPECT_DOUBLE_EQ(session.realized_pnl(), 0.0);
  EXPECT_DOUBLE_EQ(session.realized_investment(), 0.0);
  EXPECT_EQ(session.realized_duration(), 0);

  EXPECT_FALSE(session.is_flat());
  EXPECT_TRUE(session.is_open());
  EXPECT_FALSE(session.is_closed());
  EXPECT_FALSE(session.is_reopen());
}

TEST(TradeSessionTest, OpenShortPosition)
{
  auto session = TradeSession{static_cast<std::time_t>(20), 100.0, 1};

  auto trade_entry = TradeEntry{-2.0, 100.0};
  session.entry_position(trade_entry);

  EXPECT_TRUE(session.open_position().has_value());
  EXPECT_FALSE(session.closed_position().has_value());

  EXPECT_DOUBLE_EQ(session.unrealized_pnl(), 0.0);
  EXPECT_DOUBLE_EQ(session.partial_realized_pnl(), 0.0);
  EXPECT_DOUBLE_EQ(session.unrealized_investment(), -200.0);
  EXPECT_EQ(session.unrealized_duration(), 0);

  EXPECT_DOUBLE_EQ(session.realized_pnl(), 0.0);
  EXPECT_DOUBLE_EQ(session.realized_investment(), 0.0);
  EXPECT_EQ(session.realized_duration(), 0);

  EXPECT_FALSE(session.is_flat());
  EXPECT_TRUE(session.is_open());
  EXPECT_FALSE(session.is_closed());
  EXPECT_FALSE(session.is_reopen());

  session.market_update(static_cast<std::time_t>(25), 95.0, 2);

  EXPECT_DOUBLE_EQ(session.unrealized_pnl(), 10.0);
  EXPECT_DOUBLE_EQ(session.partial_realized_pnl(), 0.0);
  EXPECT_DOUBLE_EQ(session.unrealized_investment(), -200.0);
  EXPECT_EQ(session.unrealized_duration(), 5);
}

TEST(TradeSessionTest, CloseShortPositionWithExit)
{
  auto session = TradeSession{static_cast<std::time_t>(20), 100.0, 1};

  auto trade_entry = TradeEntry{-2.0, 100.0};
  session.entry_position(trade_entry);

  session.market_update(static_cast<std::time_t>(30), 95.0, 3);
  auto trade_exit = TradeExit{-2.0, 95.0, TradeExit::Reason::signal};
  session.exit_position(trade_exit);

  EXPECT_FALSE(session.open_position().has_value());
  EXPECT_TRUE(session.closed_position().has_value());

  EXPECT_DOUBLE_EQ(session.unrealized_pnl(), 0.0);
  EXPECT_DOUBLE_EQ(session.partial_realized_pnl(), 0.0);
  EXPECT_DOUBLE_EQ(session.unrealized_investment(), 0.0);
  EXPECT_EQ(session.unrealized_duration(), 0);

  EXPECT_DOUBLE_EQ(session.realized_pnl(), 10.0);
  EXPECT_DOUBLE_EQ(session.realized_investment(), -200.0);
  EXPECT_EQ(session.realized_duration(), 10);

  EXPECT_FALSE(session.is_flat());
  EXPECT_FALSE(session.is_open());
  EXPECT_TRUE(session.is_closed());
  EXPECT_FALSE(session.is_reopen());
}

TEST(TradeSessionTest, ShortPositionScaleIn)
{
  auto session = TradeSession{};

  session.market_update(static_cast<std::time_t>(20), 130.0, 1);
  auto first_entry = TradeEntry{-1.0, 130.0};
  session.entry_position(first_entry);

  session.market_update(static_cast<std::time_t>(25), 100.0, 5);
  auto second_entry = TradeEntry{-2.0, 100.0};
  session.entry_position(second_entry);

  EXPECT_TRUE(session.open_position().has_value());
  EXPECT_FALSE(session.closed_position().has_value());

  EXPECT_DOUBLE_EQ(session.unrealized_pnl(), 30.0);
  EXPECT_DOUBLE_EQ(session.partial_realized_pnl(), 0.0);
  EXPECT_DOUBLE_EQ(session.unrealized_investment(), -330.0);
  EXPECT_EQ(session.unrealized_duration(), 5);

  EXPECT_DOUBLE_EQ(session.realized_pnl(), 0.0);
  EXPECT_DOUBLE_EQ(session.realized_investment(), 0.0);
  EXPECT_EQ(session.realized_duration(), 0);

  EXPECT_FALSE(session.is_flat());
  EXPECT_TRUE(session.is_open());
  EXPECT_FALSE(session.is_closed());
  EXPECT_FALSE(session.is_reopen());

  session.market_update(static_cast<std::time_t>(30), 105.0, 7);
  EXPECT_DOUBLE_EQ(session.unrealized_pnl(), 15.0);
  EXPECT_DOUBLE_EQ(session.partial_realized_pnl(), 0.0);
  EXPECT_DOUBLE_EQ(session.unrealized_investment(), -330.0);
  EXPECT_EQ(session.unrealized_duration(), 10);
}

TEST(TradeSessionTest, ShortPositionScaleOut)
{
  auto session = TradeSession{};

  session.market_update(static_cast<std::time_t>(20), 100.0, 1);
  auto trade_entry = TradeEntry{-3.0, 100.0};
  session.entry_position(trade_entry);

  session.market_update(static_cast<std::time_t>(25), 120.0, 5);
  auto trade_exit = TradeExit{-1.0, 130.0, TradeExit::Reason::signal};
  session.exit_position(trade_exit);

  EXPECT_TRUE(session.open_position().has_value());
  EXPECT_FALSE(session.closed_position().has_value());

  EXPECT_DOUBLE_EQ(session.unrealized_pnl(), -40.0);
  EXPECT_DOUBLE_EQ(session.partial_realized_pnl(), -30.0);
  EXPECT_DOUBLE_EQ(session.unrealized_investment(), -200.0);
  EXPECT_EQ(session.unrealized_duration(), 5);

  EXPECT_DOUBLE_EQ(session.realized_pnl(), 0.0);
  EXPECT_DOUBLE_EQ(session.realized_investment(), 0.0);
  EXPECT_EQ(session.realized_duration(), 0);

  EXPECT_FALSE(session.is_flat());
  EXPECT_TRUE(session.is_open());
  EXPECT_FALSE(session.is_closed());
  EXPECT_FALSE(session.is_reopen());
}
