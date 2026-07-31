module;

#include <cstddef>
#include <ctime>
#include <optional>

export module pludux.backtest:strategy_intent;

export namespace pludux::backtest {

enum class StrategyDirection { Long, Short };

enum class StrategyIntentType {
  InitialEntry,
  PyramidingEntry,
  SignalExit,
  StopLoss,
  TakeProfit
};

class StrategyIntent {
public:
  StrategyIntent() = default;

  StrategyIntent(std::size_t intent_id,
                 std::size_t strategy_trade_id,
                 StrategyIntentType type,
                 StrategyDirection direction,
                 std::time_t timestamp,
                 double price,
                 double reduce = 1.0,
                 std::optional<std::size_t> rule_index = std::nullopt) noexcept
  : intent_id_{intent_id}
  , strategy_trade_id_{strategy_trade_id}
  , type_{type}
  , direction_{direction}
  , timestamp_{timestamp}
  , price_{price}
  , reduce_{reduce}
  , rule_index_{rule_index}
  {
  }

  auto operator==(const StrategyIntent&) const noexcept -> bool = default;

  auto intent_id(this const StrategyIntent& self) noexcept -> std::size_t
  {
    return self.intent_id_;
  }

  auto strategy_trade_id(this const StrategyIntent& self) noexcept
   -> std::size_t
  {
    return self.strategy_trade_id_;
  }

  auto type(this const StrategyIntent& self) noexcept -> StrategyIntentType
  {
    return self.type_;
  }

  auto direction(this const StrategyIntent& self) noexcept -> StrategyDirection
  {
    return self.direction_;
  }

  auto timestamp(this const StrategyIntent& self) noexcept -> std::time_t
  {
    return self.timestamp_;
  }

  auto price(this const StrategyIntent& self) noexcept -> double
  {
    return self.price_;
  }

  auto reduce(this const StrategyIntent& self) noexcept -> double
  {
    return self.reduce_;
  }

  auto rule_index(this const StrategyIntent& self) noexcept
   -> std::optional<std::size_t>
  {
    return self.rule_index_;
  }

  auto is_initial_entry(this const StrategyIntent& self) noexcept -> bool
  {
    return self.type_ == StrategyIntentType::InitialEntry;
  }

  auto is_entry(this const StrategyIntent& self) noexcept -> bool
  {
    return self.type_ == StrategyIntentType::InitialEntry ||
           self.type_ == StrategyIntentType::PyramidingEntry;
  }

private:
  std::size_t intent_id_{};
  std::size_t strategy_trade_id_{};
  StrategyIntentType type_{StrategyIntentType::InitialEntry};
  StrategyDirection direction_{StrategyDirection::Long};
  std::time_t timestamp_{};
  double price_{};
  double reduce_{1.0};
  std::optional<std::size_t> rule_index_{};
};

class ExecutionFilterDecision {
public:
  ExecutionFilterDecision() = default;

  ExecutionFilterDecision(std::size_t intent_id, bool allowed) noexcept
  : intent_id_{intent_id}
  , allowed_{allowed}
  {
  }

  auto operator==(const ExecutionFilterDecision&) const noexcept
   -> bool = default;

  auto intent_id(this const ExecutionFilterDecision& self) noexcept
   -> std::size_t
  {
    return self.intent_id_;
  }

  auto allowed(this const ExecutionFilterDecision& self) noexcept -> bool
  {
    return self.allowed_;
  }

private:
  std::size_t intent_id_{};
  bool allowed_{true};
};

} // namespace pludux::backtest
