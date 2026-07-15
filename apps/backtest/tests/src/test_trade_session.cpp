#include <gtest/gtest.h>

#include <cmath>

import pludux.backtest;

using namespace pludux;
using namespace pludux::backtest;

TEST(TradeSessionTest, DefaultConstructor)
{
  auto session = TradeSession{};

  EXPECT_FALSE(session.open_position().has_value());
  EXPECT_TRUE(session.trade_events().empty());
  EXPECT_TRUE(session.closed_trades().empty());
  EXPECT_TRUE(session.realized_exits().empty());
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

  ASSERT_FALSE(session.trade_events().empty());

  session.begin_market_bar(static_cast<std::time_t>(25), 120.0, 2);

  EXPECT_EQ(session.market_timestamp(), std::time_t{25});
  EXPECT_DOUBLE_EQ(session.market_price(), 120.0);
  EXPECT_EQ(session.market_lookback(), std::size_t{2});
  EXPECT_TRUE(session.trade_events().empty());
  EXPECT_TRUE(session.open_position().has_value());
}

TEST(TradeSessionTest, EntryOpensLongPositionAndEmitsEntryEvent)
{
  auto session = TradeSession{static_cast<std::time_t>(20), 100.0, 1};

  session.entry_position(TradeEntry{2.0, 100.0});

  ASSERT_TRUE(session.open_position().has_value());
  ASSERT_EQ(session.trade_events().size(), 1);
  EXPECT_TRUE(session.trade_events().back().is_entry());
  EXPECT_EQ(session.trade_events().back().trade_id(), std::size_t{1});
  EXPECT_EQ(session.trade_events().back().event_id(), std::size_t{1});
  EXPECT_EQ(session.trade_events().back().trade_event_index(), std::size_t{1});
  EXPECT_DOUBLE_EQ(session.unrealized_pnl(), 0.0);
  EXPECT_DOUBLE_EQ(session.unrealized_investment(), 200.0);
  EXPECT_EQ(session.unrealized_duration(), 0);

  session.begin_market_bar(static_cast<std::time_t>(25), 105.0, 5);

  EXPECT_DOUBLE_EQ(session.unrealized_pnl(), 10.0);
  EXPECT_DOUBLE_EQ(session.unrealized_investment(), 200.0);
  EXPECT_EQ(session.unrealized_duration(), 5);
}

TEST(TradeSessionTest, EntryOpensShortPositionAndEmitsEntryEvent)
{
  auto session = TradeSession{static_cast<std::time_t>(20), 100.0, 1};

  session.entry_position(TradeEntry{-2.0, 100.0});

  ASSERT_TRUE(session.open_position().has_value());
  ASSERT_EQ(session.trade_events().size(), 1);
  EXPECT_TRUE(session.trade_events().back().is_entry());
  EXPECT_EQ(session.trade_events().back().trade_id(), std::size_t{1});
  EXPECT_EQ(session.trade_events().back().event_id(), std::size_t{1});
  EXPECT_DOUBLE_EQ(session.unrealized_investment(), -200.0);

  session.begin_market_bar(static_cast<std::time_t>(25), 95.0, 2);

  EXPECT_DOUBLE_EQ(session.unrealized_pnl(), 10.0);
  EXPECT_EQ(session.unrealized_duration(), 5);
}

TEST(TradeSessionTest, ScaleInEmitsScaleInEvent)
{
  auto session = TradeSession{static_cast<std::time_t>(20), 100.0, 1};

  session.entry_position(TradeEntry{2.0, 100.0});
  session.begin_market_bar(static_cast<std::time_t>(25), 130.0, 5);
  session.entry_position(TradeEntry{1.0, 130.0});

  ASSERT_TRUE(session.open_position().has_value());
  ASSERT_EQ(session.trade_events().size(), 1);

  const auto& event = session.trade_events().back();
  EXPECT_TRUE(event.is_scale_in());
  EXPECT_EQ(event.trade_id(), std::size_t{1});
  EXPECT_EQ(event.event_id(), std::size_t{2});
  EXPECT_EQ(event.trade_event_index(), std::size_t{2});
  EXPECT_DOUBLE_EQ(event.position_size(), 1.0);
  EXPECT_DOUBLE_EQ(event.position_size_before(), 2.0);
  EXPECT_DOUBLE_EQ(event.investment_before(), 200.0);
  EXPECT_EQ(event.timestamp(), std::time_t{25});
  EXPECT_DOUBLE_EQ(event.price(), 130.0);
  EXPECT_DOUBLE_EQ(event.position_size_after(), 3.0);
  EXPECT_DOUBLE_EQ(event.investment_after(), 330.0);

  EXPECT_DOUBLE_EQ(session.unrealized_pnl(), 60.0);
  EXPECT_DOUBLE_EQ(session.unrealized_investment(), 330.0);
  EXPECT_EQ(session.unrealized_duration(), 5);
}

TEST(TradeSessionTest, RejectInsufficientCashEmitsRejectedEventOnly)
{
  auto session = TradeSession{static_cast<std::time_t>(20), 100.0, 1};

  session.reject_insufficient_cash(TradeEntry{2.0, 100.0});

  ASSERT_FALSE(session.open_position().has_value());
  ASSERT_EQ(session.trade_events().size(), 1);
  EXPECT_TRUE(session.closed_trades().empty());

  const auto& rejected_event = session.trade_events().back();
  EXPECT_TRUE(rejected_event.is_rejected());
  EXPECT_FALSE(rejected_event.is_entry());
  EXPECT_FALSE(rejected_event.is_scale_in());
  EXPECT_FALSE(rejected_event.is_scale_out());
  EXPECT_FALSE(rejected_event.is_exit());
  EXPECT_EQ(rejected_event.trade_id(), std::size_t{0});
  EXPECT_EQ(rejected_event.event_id(), std::size_t{1});
  EXPECT_DOUBLE_EQ(rejected_event.position_size(), 2.0);
  EXPECT_DOUBLE_EQ(rejected_event.price(), 100.0);
  EXPECT_DOUBLE_EQ(rejected_event.fees(), 0.0);
  EXPECT_DOUBLE_EQ(rejected_event.position_size_before(), 0.0);
  EXPECT_DOUBLE_EQ(rejected_event.position_size_after(), 0.0);
  EXPECT_TRUE(rejected_event.stop_loss_levels().empty());

  session.entry_position(TradeEntry{1.0, 100.0});

  ASSERT_TRUE(session.open_position().has_value());
  ASSERT_EQ(session.trade_events().size(), 2);
  EXPECT_TRUE(session.trade_events().back().is_entry());
  EXPECT_EQ(session.trade_events().back().trade_id(), std::size_t{1});
  EXPECT_EQ(session.trade_events().back().event_id(), std::size_t{2});
}

TEST(TradeSessionTest, PartialScaleOutEmitsRecordAndKeepsPositionOpen)
{
  auto session = TradeSession{static_cast<std::time_t>(20), 100.0, 1};

  session.entry_position(TradeEntry{3.0, 100.0});
  session.begin_market_bar(static_cast<std::time_t>(25), 120.0, 5);
  session.exit_position(TradeExit{1.0, 130.0, TradeExit::Reason::signal});

  ASSERT_TRUE(session.open_position().has_value());
  ASSERT_EQ(session.trade_events().size(), 1);
  EXPECT_TRUE(session.closed_trades().empty());
  ASSERT_EQ(session.realized_exits().size(), 1);
  EXPECT_DOUBLE_EQ(session.realized_exits().front().pnl(), 30.0);

  const auto& event = session.trade_events().back();
  EXPECT_TRUE(event.is_exit());
  EXPECT_TRUE(event.is_scale_out());
  EXPECT_EQ(event.type(), TradeEvent::Type::exit_signal);
  EXPECT_DOUBLE_EQ(event.position_size(), 1.0);
  EXPECT_DOUBLE_EQ(event.position_size_after(), 2.0);
  EXPECT_DOUBLE_EQ(event.investment_before(), 300.0);

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
  ASSERT_EQ(session.trade_events().size(), 1);
  ASSERT_EQ(session.closed_trades().size(), 1);

  const auto& event = session.trade_events().back();
  EXPECT_TRUE(event.is_exit());
  EXPECT_FALSE(event.is_scale_out());
  EXPECT_EQ(event.type(), TradeEvent::Type::exit_signal);
  EXPECT_DOUBLE_EQ(event.position_size(), 2.0);
  EXPECT_DOUBLE_EQ(event.position_size_after(), 0.0);

  const auto& trade = session.closed_trades().back();
  EXPECT_EQ(trade.trade_id(), std::size_t{1});
  EXPECT_DOUBLE_EQ(trade.position_size(), 2.0);
  EXPECT_DOUBLE_EQ(trade.investment(), 200.0);
  EXPECT_DOUBLE_EQ(trade.pnl(), 10.0);
  EXPECT_EQ(trade.duration(), std::time_t{10});

  EXPECT_DOUBLE_EQ(session.unrealized_pnl(), 0.0);
  EXPECT_DOUBLE_EQ(session.unrealized_investment(), 0.0);
  EXPECT_EQ(session.unrealized_duration(), 0);
}

TEST(TradeSessionTest,
     IndexedSignalExitStatePropagatesOnlyAfterSuccessfulExecution)
{
  auto session = TradeSession{static_cast<std::time_t>(20), 100.0, 1};

  session.entry_position(TradeEntry{3.0, 100.0});
  session.open_position()->signal_exit_states(
   {SignalExitState{false}, SignalExitState{true}});
  session.sync_latest_event_with_open_position();
  session.begin_market_bar(static_cast<std::time_t>(25), 120.0, 5);

  EXPECT_THROW(
   session.exit_position(TradeExit{
    1.0, 130.0, TradeExit::Reason::signal, std::nullopt, std::nullopt, 2}),
   std::runtime_error);
  ASSERT_TRUE(session.open_position());
  EXPECT_DOUBLE_EQ(session.open_position()->position_size(), 3.0);
  EXPECT_FALSE(session.open_position()->signal_exit_states()[1].consumed());
  EXPECT_TRUE(session.trade_events().empty());

  session.exit_position(TradeExit{
   1.0, 130.0, TradeExit::Reason::signal, std::nullopt, std::nullopt, 1});

  ASSERT_TRUE(session.open_position());
  EXPECT_TRUE(session.open_position()->signal_exit_states()[1].consumed());
  ASSERT_EQ(session.trade_events().size(), 1);
  EXPECT_TRUE(
   session.trade_events().front().signal_exit_states()[1].consumed());
  const auto snapshot = session.open_position_snapshot();
  ASSERT_TRUE(snapshot);
  EXPECT_TRUE(snapshot->signal_exit_states()[1].consumed());

  session.begin_market_bar(static_cast<std::time_t>(30), 140.0, 6);
  session.exit_position(TradeExit{2.0, 140.0, TradeExit::Reason::signal});

  ASSERT_EQ(session.closed_trades().size(), 1);
  ASSERT_EQ(session.closed_trades().front().signal_exit_states().size(), 2);
  EXPECT_TRUE(
   session.closed_trades().front().signal_exit_states()[1].consumed());
}

TEST(TradePositionTest, UpdatesTrailingStopAndChecksTriggers)
{
  auto position = TradePosition{1,
                                2.0,
                                static_cast<std::time_t>(20),
                                100.0,
                                0.0,
                                {StopLossLevel{90.0, 90.0, true, true},
                                 StopLossLevel{80.0, 80.0, true, false},
                                 StopLossLevel{85.0, 85.0, true, true, true}}};
  position.take_profit_levels({TakeProfitLevel{120.0, true}});

  position.update_trailing_stops(115.0);

  ASSERT_EQ(position.stop_loss_levels().size(), 3);
  EXPECT_DOUBLE_EQ(position.stop_loss_levels()[0].evaluated_price(), 90.0);
  EXPECT_DOUBLE_EQ(position.stop_loss_levels()[0].effective_price(), 105.0);
  EXPECT_DOUBLE_EQ(position.stop_loss_levels()[1].effective_price(), 80.0);
  EXPECT_DOUBLE_EQ(position.stop_loss_levels()[2].effective_price(), 85.0);
  EXPECT_TRUE(position.is_stop_loss_triggered(0, 110.0, 104.0));
  EXPECT_FALSE(position.is_stop_loss_triggered(1, 110.0, 104.0));
  EXPECT_FALSE(position.is_stop_loss_triggered(2, 110.0, 80.0));
  EXPECT_TRUE(position.is_take_profit_triggered(0, 121.0, 110.0));
}

TEST(TradeSessionTest, IndexedStopLossStatePropagatesAfterSuccessfulExecution)
{
  auto session = TradeSession{static_cast<std::time_t>(20), 100.0, 1};

  session.entry_position(TradeEntry{3.0, 100.0});
  session.open_position()->stop_loss_levels(
   {StopLossLevel{90.0, 90.0, true, false},
    StopLossLevel{80.0, 80.0, true, false}});
  session.sync_latest_event_with_open_position();
  session.begin_market_bar(static_cast<std::time_t>(25), 85.0, 5);

  EXPECT_THROW(
   session.exit_position(TradeExit{1.0, 85.0, TradeExit::Reason::stop_loss, 2}),
   std::runtime_error);
  ASSERT_TRUE(session.open_position());
  EXPECT_FALSE(session.open_position()->stop_loss_levels()[1].consumed());
  EXPECT_TRUE(session.trade_events().empty());

  session.exit_position(TradeExit{1.0, 85.0, TradeExit::Reason::stop_loss, 1});

  ASSERT_TRUE(session.open_position());
  EXPECT_FALSE(session.open_position()->stop_loss_levels()[0].consumed());
  EXPECT_TRUE(session.open_position()->stop_loss_levels()[1].consumed());
  ASSERT_EQ(session.trade_events().size(), 1);
  EXPECT_TRUE(session.trade_events().front().stop_loss_levels()[1].consumed());
  const auto snapshot = session.open_position_snapshot();
  ASSERT_TRUE(snapshot);
  EXPECT_TRUE(snapshot->stop_loss_levels()[1].consumed());

  session.begin_market_bar(static_cast<std::time_t>(30), 100.0, 6);
  session.exit_position(TradeExit{2.0, 100.0, TradeExit::Reason::signal});

  ASSERT_EQ(session.closed_trades().size(), 1);
  EXPECT_TRUE(session.closed_trades().front().stop_loss_levels()[1].consumed());
}
