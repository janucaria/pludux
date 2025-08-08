#include <gtest/gtest.h>

#include <pludux/backtest/trade_session.hpp>

using namespace pludux::backtest;

TEST(TradeSessionTest, DefaultConstructor)
{
  auto session = TradeSession{};

  EXPECT_TRUE(std::isnan(session.take_profit_price()));
  EXPECT_TRUE(std::isnan(session.stop_loss_price()));
  EXPECT_TRUE(std::isnan(session.trailing_stop_price()));

  EXPECT_TRUE(std::isnan(session.position_size()));
  EXPECT_TRUE(std::isnan(session.investment()));
  EXPECT_TRUE(std::isnan(session.average_price()));
  EXPECT_TRUE(std::isnan(session.exit_price()));
  EXPECT_EQ(session.entry_index(), std::size_t{0});
  EXPECT_EQ(session.exit_index(), std::size_t{0});
  EXPECT_TRUE(std::isnan(session.pnl()));
  EXPECT_EQ(session.duration(), std::time_t{0});
  EXPECT_TRUE(session.is_flat());
  EXPECT_FALSE(session.is_open());
  EXPECT_FALSE(session.is_closed());
  EXPECT_FALSE(session.is_closed_exit_signal());
  EXPECT_FALSE(session.is_closed_take_profit());
  EXPECT_FALSE(session.is_closed_stop_loss());
}

TEST(TradeSessionTest, ConstructorWithParameters)
{
  auto take_profit_price = 110.0;
  auto stop_loss_price = 90.0;
  auto trailing_stop_price = 95.0;
  auto trade_records = std::vector<TradeRecord>{};

  auto session = TradeSession{
   take_profit_price, stop_loss_price, trailing_stop_price, trade_records};

  EXPECT_DOUBLE_EQ(session.take_profit_price(), take_profit_price);
  EXPECT_DOUBLE_EQ(session.stop_loss_price(), stop_loss_price);
  EXPECT_DOUBLE_EQ(session.trailing_stop_price(), trailing_stop_price);
  EXPECT_TRUE(session.trade_records().empty());

  auto timestamp = std::time(nullptr);
  auto price = 100.0;
  auto index = std::size_t{1};
  session.asset_update(timestamp, price, index);

  EXPECT_TRUE(std::isnan(session.position_size()));
  EXPECT_TRUE(std::isnan(session.investment()));
  EXPECT_TRUE(std::isnan(session.average_price()));
  EXPECT_TRUE(std::isnan(session.exit_price()));
  EXPECT_EQ(session.entry_index(), std::size_t{0});
  EXPECT_EQ(session.exit_index(), std::size_t{0});
  EXPECT_EQ(session.entry_timestamp(), std::time_t{0});
  EXPECT_EQ(session.exit_timestamp(), std::time_t{0});
  EXPECT_TRUE(std::isnan(session.pnl()));
  EXPECT_EQ(session.duration(), std::time_t{0});
  EXPECT_TRUE(session.is_flat());
  EXPECT_FALSE(session.is_open());
  EXPECT_FALSE(session.is_closed());
  EXPECT_FALSE(session.is_closed_exit_signal());
  EXPECT_FALSE(session.is_closed_take_profit());
  EXPECT_FALSE(session.is_closed_stop_loss());
}

TEST(TradeSessionTest, OpenPosition)
{
  auto session = TradeSession{};
  session.emplace_action(
   TradeAction::ActionType::buy, 2.0, static_cast<std::time_t>(20), 100.0, 1);

  EXPECT_DOUBLE_EQ(session.position_size(), 2.0);
  EXPECT_DOUBLE_EQ(session.investment(), 200.0);
  EXPECT_DOUBLE_EQ(session.average_price(), 100.0);
  EXPECT_DOUBLE_EQ(session.exit_price(), 100.0);
  EXPECT_EQ(session.entry_index(), 1);
  EXPECT_EQ(session.exit_index(), 1);
  EXPECT_EQ(session.entry_timestamp(), static_cast<std::time_t>(20));
  EXPECT_EQ(session.exit_timestamp(), std::time_t{20});
  EXPECT_DOUBLE_EQ(session.pnl(), 0.0);
  EXPECT_EQ(session.duration(), std::time_t{0});
  EXPECT_FALSE(session.is_flat());
  EXPECT_TRUE(session.is_open());
  EXPECT_FALSE(session.is_closed());
  EXPECT_FALSE(session.is_closed_exit_signal());
  EXPECT_FALSE(session.is_closed_take_profit());
  EXPECT_FALSE(session.is_closed_stop_loss());

  session.asset_update(static_cast<std::time_t>(25), 105.0, 2);
  EXPECT_DOUBLE_EQ(session.exit_price(), 105.0);
  EXPECT_EQ(session.exit_index(), 2);
  EXPECT_EQ(session.exit_timestamp(), static_cast<std::time_t>(25));
  EXPECT_DOUBLE_EQ(session.pnl(), 10.0);
  EXPECT_EQ(session.duration(), static_cast<std::time_t>(5));
  EXPECT_FALSE(session.is_flat());
  EXPECT_TRUE(session.is_open());
  EXPECT_FALSE(session.is_closed());
  EXPECT_FALSE(session.is_closed_exit_signal());
  EXPECT_FALSE(session.is_closed_take_profit());
  EXPECT_FALSE(session.is_closed_stop_loss());
}

TEST(TradeSessionTest, ClosePositionWithExit)
{
  auto session = TradeSession{};

  session.emplace_action(
   TradeAction::ActionType::buy, 2.0, static_cast<std::time_t>(20), 100.0, 1);
  session.emplace_action(
   TradeAction::ActionType::sell, 2.0, static_cast<std::time_t>(30), 105.0, 3);

  EXPECT_DOUBLE_EQ(session.position_size(), 2.0);
  EXPECT_DOUBLE_EQ(session.investment(), 200.0);
  EXPECT_DOUBLE_EQ(session.average_price(), 100.0);
  EXPECT_EQ(session.exit_price(), 105.0);
  EXPECT_EQ(session.entry_index(), 1);
  EXPECT_EQ(session.exit_index(), 3);
  EXPECT_EQ(session.entry_timestamp(), static_cast<std::time_t>(20));
  EXPECT_EQ(session.exit_timestamp(), static_cast<std::time_t>(30));
  EXPECT_DOUBLE_EQ(session.pnl(), 10.0);
  EXPECT_EQ(session.duration(), static_cast<std::time_t>(10));
  EXPECT_FALSE(session.is_flat());
  EXPECT_FALSE(session.is_open());
  EXPECT_TRUE(session.is_closed());
  EXPECT_TRUE(session.is_closed_exit_signal());
  EXPECT_FALSE(session.is_closed_take_profit());
  EXPECT_FALSE(session.is_closed_stop_loss());

  const auto& trade_records = session.trade_records();
  ASSERT_EQ(trade_records.size(), 1);

  auto& closed_record = trade_records[0];
  EXPECT_EQ(closed_record.status(), TradeRecord::Status::closed_exit_signal);
  EXPECT_DOUBLE_EQ(closed_record.position_size(), 2.0);
  EXPECT_DOUBLE_EQ(closed_record.investment(), 200.0);
  EXPECT_DOUBLE_EQ(closed_record.average_price(), 100.0);
  EXPECT_DOUBLE_EQ(closed_record.entry_price(), 100.0);
  EXPECT_DOUBLE_EQ(closed_record.exit_price(), 105.0);
  EXPECT_EQ(closed_record.entry_index(), 1);
  EXPECT_EQ(closed_record.exit_index(), 3);
  EXPECT_EQ(closed_record.entry_timestamp(), static_cast<std::time_t>(20));
  EXPECT_EQ(closed_record.exit_timestamp(), static_cast<std::time_t>(30));
  EXPECT_DOUBLE_EQ(closed_record.pnl(), 10.0);
  EXPECT_EQ(closed_record.duration(), static_cast<std::time_t>(10));
  EXPECT_FALSE(closed_record.is_open());
  EXPECT_TRUE(closed_record.is_closed());
  EXPECT_TRUE(closed_record.is_closed_exit_signal());
  EXPECT_FALSE(closed_record.is_closed_take_profit());
  EXPECT_FALSE(closed_record.is_closed_stop_loss());
  EXPECT_FALSE(closed_record.is_scaled());
  EXPECT_FALSE(closed_record.is_scaled_in());
  EXPECT_FALSE(closed_record.is_scaled_out());
}

TEST(TradeSessionTest, PositionScaleIn)
{
  auto session = TradeSession{};

  session.emplace_action(
   TradeAction::ActionType::buy, 2.0, static_cast<std::time_t>(20), 100.0, 1);
  session.emplace_action(
   TradeAction::ActionType::buy, 1.0, static_cast<std::time_t>(25), 130.0, 5);

  EXPECT_DOUBLE_EQ(session.position_size(), 3.0);
  EXPECT_DOUBLE_EQ(session.investment(), 330.0);
  EXPECT_DOUBLE_EQ(session.average_price(), 110.0);
  EXPECT_DOUBLE_EQ(session.exit_price(), 130.0);
  EXPECT_EQ(session.entry_index(), 1);
  EXPECT_EQ(session.exit_index(), 5);
  EXPECT_EQ(session.entry_timestamp(), static_cast<std::time_t>(20));
  EXPECT_EQ(session.exit_timestamp(), static_cast<std::time_t>(25));
  EXPECT_DOUBLE_EQ(session.pnl(), 60.0);
  EXPECT_EQ(session.duration(), static_cast<std::time_t>(5));
  EXPECT_FALSE(session.is_flat());
  EXPECT_TRUE(session.is_open());
  EXPECT_FALSE(session.is_closed());
  EXPECT_FALSE(session.is_closed_exit_signal());
  EXPECT_FALSE(session.is_closed_take_profit());
  EXPECT_FALSE(session.is_closed_stop_loss());

  const auto& trade_records = session.trade_records();
  ASSERT_EQ(trade_records.size(), 2);

  auto& scaled_record = trade_records[0];
  EXPECT_EQ(scaled_record.status(), TradeRecord::Status::scaled_in);
  EXPECT_DOUBLE_EQ(scaled_record.position_size(), 2.0);
  EXPECT_DOUBLE_EQ(scaled_record.investment(), 200.0);
  EXPECT_DOUBLE_EQ(scaled_record.average_price(), 100.0);
  EXPECT_DOUBLE_EQ(scaled_record.entry_price(), 100.0);
  EXPECT_DOUBLE_EQ(scaled_record.exit_price(), scaled_record.average_price());
  EXPECT_EQ(scaled_record.entry_index(), 1);
  EXPECT_EQ(scaled_record.exit_index(), 5);
  EXPECT_EQ(scaled_record.entry_timestamp(), static_cast<std::time_t>(20));
  EXPECT_EQ(scaled_record.exit_timestamp(), static_cast<std::time_t>(25));
  EXPECT_DOUBLE_EQ(scaled_record.pnl(), 0.0);
  EXPECT_EQ(scaled_record.duration(), static_cast<std::time_t>(5));
  EXPECT_FALSE(scaled_record.is_open());
  EXPECT_FALSE(scaled_record.is_closed());
  EXPECT_FALSE(scaled_record.is_closed_exit_signal());
  EXPECT_FALSE(scaled_record.is_closed_take_profit());
  EXPECT_FALSE(scaled_record.is_closed_stop_loss());
  EXPECT_TRUE(scaled_record.is_scaled());
  EXPECT_TRUE(scaled_record.is_scaled_in());
  EXPECT_FALSE(scaled_record.is_scaled_out());

  auto& open_record = trade_records[1];
  EXPECT_EQ(open_record.status(), TradeRecord::Status::open);
  EXPECT_DOUBLE_EQ(open_record.position_size(), 3.0);
  EXPECT_DOUBLE_EQ(open_record.investment(), 330.0);
  EXPECT_DOUBLE_EQ(open_record.average_price(), 110.0);
  EXPECT_DOUBLE_EQ(open_record.entry_price(), 130.0);
  EXPECT_DOUBLE_EQ(open_record.exit_price(), 130.0);
  EXPECT_EQ(open_record.entry_index(), 5);
  EXPECT_EQ(open_record.exit_index(), 5);
  EXPECT_EQ(open_record.entry_timestamp(), static_cast<std::time_t>(25));
  EXPECT_EQ(open_record.exit_timestamp(), static_cast<std::time_t>(25));
  EXPECT_DOUBLE_EQ(open_record.pnl(), 60.0);
  EXPECT_EQ(open_record.duration(), static_cast<std::time_t>(0));
  EXPECT_TRUE(open_record.is_open());
  EXPECT_FALSE(open_record.is_closed());
  EXPECT_FALSE(open_record.is_closed_exit_signal());
  EXPECT_FALSE(open_record.is_closed_take_profit());
  EXPECT_FALSE(open_record.is_closed_stop_loss());
  EXPECT_FALSE(open_record.is_scaled());
  EXPECT_FALSE(open_record.is_scaled_in());
  EXPECT_FALSE(open_record.is_scaled_out());

  session.asset_update(static_cast<std::time_t>(30), 105.0, 7);
  EXPECT_EQ(open_record.status(), TradeRecord::Status::open);
  EXPECT_DOUBLE_EQ(open_record.position_size(), 3.0);
  EXPECT_DOUBLE_EQ(open_record.investment(), 330.0);
  EXPECT_DOUBLE_EQ(open_record.average_price(), 110.0);
  EXPECT_DOUBLE_EQ(open_record.entry_price(), 130.0);
  EXPECT_DOUBLE_EQ(open_record.exit_price(), 105.0);
  EXPECT_EQ(open_record.entry_index(), 5);
  EXPECT_EQ(open_record.exit_index(), 7);
  EXPECT_EQ(open_record.entry_timestamp(), static_cast<std::time_t>(25));
  EXPECT_EQ(open_record.exit_timestamp(), static_cast<std::time_t>(30));
  EXPECT_DOUBLE_EQ(open_record.pnl(), -15.0);
  EXPECT_EQ(open_record.duration(), static_cast<std::time_t>(5));
  EXPECT_TRUE(open_record.is_open());
  EXPECT_FALSE(open_record.is_closed());
  EXPECT_FALSE(open_record.is_closed_exit_signal());
  EXPECT_FALSE(open_record.is_closed_take_profit());
  EXPECT_FALSE(open_record.is_closed_stop_loss());
  EXPECT_FALSE(open_record.is_scaled());
  EXPECT_FALSE(open_record.is_scaled_in());
  EXPECT_FALSE(open_record.is_scaled_out());
}

TEST(TradeSessionTest, PositionScaleOut)
{
  auto session = TradeSession{};

  session.emplace_action(
   TradeAction::ActionType::buy, 3.0, static_cast<std::time_t>(20), 100.0, 1);
  session.emplace_action(
   TradeAction::ActionType::sell, 1.0, static_cast<std::time_t>(25), 130.0, 5);

  session.asset_update(static_cast<std::time_t>(30), 120.0, 6);

  EXPECT_DOUBLE_EQ(session.position_size(), 2.0);
  EXPECT_DOUBLE_EQ(session.investment(), 300.0);
  EXPECT_DOUBLE_EQ(session.average_price(), 100.0);
  EXPECT_DOUBLE_EQ(session.exit_price(), 120.0);
  EXPECT_EQ(session.entry_index(), 1);
  EXPECT_EQ(session.exit_index(), 6);
  EXPECT_EQ(session.entry_timestamp(), static_cast<std::time_t>(20));
  EXPECT_EQ(session.exit_timestamp(), static_cast<std::time_t>(30));
  EXPECT_DOUBLE_EQ(session.pnl(), 70.0);
  EXPECT_EQ(session.duration(), static_cast<std::time_t>(10));
  EXPECT_FALSE(session.is_flat());
  EXPECT_TRUE(session.is_open());
  EXPECT_FALSE(session.is_closed());
  EXPECT_FALSE(session.is_closed_exit_signal());
  EXPECT_FALSE(session.is_closed_take_profit());
  EXPECT_FALSE(session.is_closed_stop_loss());

  const auto& trade_records = session.trade_records();
  ASSERT_EQ(trade_records.size(), 2);

  auto& scaled_record = trade_records[0];
  EXPECT_EQ(scaled_record.status(), TradeRecord::Status::scaled_out);
  EXPECT_DOUBLE_EQ(scaled_record.position_size(), 1.0);
  EXPECT_DOUBLE_EQ(scaled_record.investment(), 100.0);
  EXPECT_DOUBLE_EQ(scaled_record.average_price(), 100.0);
  EXPECT_DOUBLE_EQ(scaled_record.exit_price(), 130.0);
  EXPECT_EQ(scaled_record.entry_index(), 1);
  EXPECT_EQ(scaled_record.exit_index(), 5);
  EXPECT_EQ(scaled_record.entry_timestamp(), static_cast<std::time_t>(20));
  EXPECT_EQ(scaled_record.exit_timestamp(), static_cast<std::time_t>(25));
  EXPECT_DOUBLE_EQ(scaled_record.pnl(), 30.0);
  EXPECT_EQ(scaled_record.duration(), static_cast<std::time_t>(5));
  EXPECT_FALSE(scaled_record.is_open());
  EXPECT_FALSE(scaled_record.is_closed());
  EXPECT_FALSE(scaled_record.is_closed_exit_signal());
  EXPECT_FALSE(scaled_record.is_closed_take_profit());
  EXPECT_FALSE(scaled_record.is_closed_stop_loss());
  EXPECT_TRUE(scaled_record.is_scaled());
  EXPECT_FALSE(scaled_record.is_scaled_in());
  EXPECT_TRUE(scaled_record.is_scaled_out());

  auto& open_record = trade_records[1];
  EXPECT_EQ(open_record.status(), TradeRecord::Status::open);
  EXPECT_DOUBLE_EQ(open_record.position_size(), 2.0);
  EXPECT_DOUBLE_EQ(open_record.investment(), 200.0);
  EXPECT_DOUBLE_EQ(open_record.average_price(), 100.0);
  EXPECT_DOUBLE_EQ(open_record.entry_price(), 100.0);
  EXPECT_DOUBLE_EQ(open_record.exit_price(), 120.0);
  EXPECT_EQ(open_record.entry_index(), 5);
  EXPECT_EQ(open_record.exit_index(), 6);
  EXPECT_EQ(open_record.entry_timestamp(), static_cast<std::time_t>(25));
  EXPECT_EQ(open_record.exit_timestamp(), static_cast<std::time_t>(30));
  EXPECT_DOUBLE_EQ(open_record.pnl(), 40.0);
  EXPECT_EQ(open_record.duration(), static_cast<std::time_t>(5));
  EXPECT_TRUE(open_record.is_open());
  EXPECT_FALSE(open_record.is_closed());
  EXPECT_FALSE(open_record.is_closed_exit_signal());
  EXPECT_FALSE(open_record.is_closed_take_profit());
  EXPECT_FALSE(open_record.is_closed_stop_loss());
  EXPECT_FALSE(open_record.is_scaled());
  EXPECT_FALSE(open_record.is_scaled_in());
  EXPECT_FALSE(open_record.is_scaled_out());
}