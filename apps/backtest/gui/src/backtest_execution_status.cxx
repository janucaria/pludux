module;

#include <cstddef>
#include <string>
#include <utility>

export module pludux.apps.backtest:backtest_execution_status;

export namespace pludux::apps {

enum class BacktestExecutionPhase {
  Waiting,
  Running,
  Completed,
  Failed,
};

class BacktestExecutionStatus {
public:
  BacktestExecutionStatus() = default;

  BacktestExecutionStatus(BacktestExecutionPhase phase,
                          std::size_t processed_bars,
                          std::size_t total_bars,
                          std::string error_message = {})
  : phase_{phase}
  , processed_bars_{processed_bars}
  , total_bars_{total_bars}
  , error_message_{std::move(error_message)}
  {
  }

  auto phase(this const BacktestExecutionStatus& self) noexcept
   -> BacktestExecutionPhase
  {
    return self.phase_;
  }

  auto processed_bars(this const BacktestExecutionStatus& self) noexcept
   -> std::size_t
  {
    return self.processed_bars_;
  }

  auto total_bars(this const BacktestExecutionStatus& self) noexcept
   -> std::size_t
  {
    return self.total_bars_;
  }

  auto progress(this const BacktestExecutionStatus& self) noexcept -> float
  {
    if(self.total_bars_ == 0) {
      return self.phase_ == BacktestExecutionPhase::Completed ? 1.0f : 0.0f;
    }

    const auto completed = static_cast<double>(self.processed_bars_);
    const auto total = static_cast<double>(self.total_bars_);
    return static_cast<float>(completed / total);
  }

  auto error_message(this const BacktestExecutionStatus& self) noexcept
   -> const std::string&
  {
    return self.error_message_;
  }

private:
  BacktestExecutionPhase phase_{BacktestExecutionPhase::Waiting};
  std::size_t processed_bars_{};
  std::size_t total_bars_{};
  std::string error_message_{};
};

} // namespace pludux::apps
