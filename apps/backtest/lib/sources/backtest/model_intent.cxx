module;

#include <cstddef>
#include <ctime>
#include <optional>

export module pludux.backtest:model_intent;

export namespace pludux::backtest {

enum class ModelDirection { Long, Short };

enum class ModelIntentType {
  InitialEntry,
  PyramidingEntry,
  SignalExit,
  StopLoss,
  TakeProfit
};

class ModelIntent {
public:
  ModelIntent() = default;

  ModelIntent(std::size_t intent_id,
                  std::size_t model_trade_id,
                  ModelIntentType type,
                  ModelDirection direction,
                 std::time_t timestamp,
                 double price,
                 double reduce = 1.0,
                 std::optional<std::size_t> rule_index = std::nullopt,
                  std::size_t strategy_index = 0) noexcept
  : intent_id_{intent_id}
  , model_trade_id_{model_trade_id}
  , type_{type}
  , direction_{direction}
  , timestamp_{timestamp}
  , price_{price}
  , reduce_{reduce}
  , rule_index_{rule_index}
   , strategy_index_{strategy_index}
  {
  }

  auto operator==(const ModelIntent&) const noexcept -> bool = default;

  auto intent_id(this const ModelIntent& self) noexcept -> std::size_t
  {
    return self.intent_id_;
  }

  auto model_trade_id(this const ModelIntent& self) noexcept
   -> std::size_t
  {
    return self.model_trade_id_;
  }

  auto type(this const ModelIntent& self) noexcept -> ModelIntentType
  {
    return self.type_;
  }

  auto direction(this const ModelIntent& self) noexcept -> ModelDirection
  {
    return self.direction_;
  }

  auto timestamp(this const ModelIntent& self) noexcept -> std::time_t
  {
    return self.timestamp_;
  }

  auto price(this const ModelIntent& self) noexcept -> double
  {
    return self.price_;
  }

  auto reduce(this const ModelIntent& self) noexcept -> double
  {
    return self.reduce_;
  }

  auto rule_index(this const ModelIntent& self) noexcept
   -> std::optional<std::size_t>
  {
    return self.rule_index_;
  }

  auto is_initial_entry(this const ModelIntent& self) noexcept -> bool
  {
    return self.type_ == ModelIntentType::InitialEntry;
  }

  auto strategy_index(this const ModelIntent& self) noexcept -> std::size_t
  {
    return self.strategy_index_;
  }

  auto is_entry(this const ModelIntent& self) noexcept -> bool
  {
    return self.type_ == ModelIntentType::InitialEntry ||
            self.type_ == ModelIntentType::PyramidingEntry;
  }

private:
  std::size_t intent_id_{};
  std::size_t model_trade_id_{};
  ModelIntentType type_{ModelIntentType::InitialEntry};
  ModelDirection direction_{ModelDirection::Long};
  std::time_t timestamp_{};
  double price_{};
  double reduce_{1.0};
  std::optional<std::size_t> rule_index_{};
  std::size_t strategy_index_{};
};

class EntryFilterDecision {
public:
  EntryFilterDecision() = default;

  EntryFilterDecision(std::size_t intent_id,
                      bool allowed,
                       std::size_t strategy_index = 0) noexcept
  : intent_id_{intent_id}
  , allowed_{allowed}
   , strategy_index_{strategy_index}
  {
  }

  auto operator==(const EntryFilterDecision&) const noexcept -> bool = default;

  auto intent_id(this const EntryFilterDecision& self) noexcept -> std::size_t
  {
    return self.intent_id_;
  }

  auto allowed(this const EntryFilterDecision& self) noexcept -> bool
  {
    return self.allowed_;
  }

  auto strategy_index(this const EntryFilterDecision& self) noexcept
   -> std::size_t
  {
    return self.strategy_index_;
  }

private:
  std::size_t intent_id_{};
  bool allowed_{true};
  std::size_t strategy_index_{};
};

} // namespace pludux::backtest
