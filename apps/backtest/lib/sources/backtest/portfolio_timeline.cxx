module;

#include <cstddef>
#include <ctime>
#include <vector>

export module pludux.backtest:portfolio_timeline;

export namespace pludux::backtest {

class PortfolioTimeline {
public:
  struct Row {
    std::time_t timestamp{};
    double capital{};
    double available_capital{};
    double equity{};
    double peak_equity{};
    double drawdown{};
    double max_drawdown{};
    double realized_pnl{};
    double unrealized_pnl{};
    double reserved_notional{};
    double gross_exposure{};
    double net_exposure{};
    std::size_t open_position_count{};
    std::size_t fresh_backtest_count{};
    std::size_t stale_backtest_count{};
    std::size_t unavailable_backtest_count{};

    auto operator==(const Row&) const noexcept -> bool = default;
  };

  auto operator==(const PortfolioTimeline&) const noexcept -> bool = default;

  void append(this PortfolioTimeline& self, Row row)
  {
    self.rows_.push_back(row);
  }

  void clear(this PortfolioTimeline& self) noexcept
  {
    self.rows_.clear();
  }

  void reserve(this PortfolioTimeline& self, std::size_t size)
  {
    self.rows_.reserve(size);
  }

  auto empty(this const PortfolioTimeline& self) noexcept -> bool
  {
    return self.rows_.empty();
  }

  auto size(this const PortfolioTimeline& self) noexcept -> std::size_t
  {
    return self.rows_.size();
  }

  auto rows(this const PortfolioTimeline& self) noexcept
   -> const std::vector<Row>&
  {
    return self.rows_;
  }

  auto row(this const PortfolioTimeline& self, std::size_t index) noexcept
   -> const Row&
  {
    return self.rows_[index];
  }

private:
  std::vector<Row> rows_;
};

} // namespace pludux::backtest
