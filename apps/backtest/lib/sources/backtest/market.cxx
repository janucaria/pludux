module;

#include <cstddef>
#include <cstdint>
#include <string>

export module pludux.backtest:market;

export namespace pludux::backtest {

class Market {
public:
  Market()
  : Market{""}
  {
  }

  Market(std::string name)
  : Market{name, 1.0, 1.0}
  {
  }

  Market(std::string name, double min_order_qty)
  : Market{name, min_order_qty, min_order_qty}
  {
  }

  Market(std::string name, double min_order_qty, double qty_step)
  : name_{std::move(name)}
  , min_order_qty_{min_order_qty}
  , qty_step_{qty_step}
  {
  }

  auto name(this const Market& self) noexcept -> const std::string&
  {
    return self.name_;
  }

  void name(this Market& self, std::string new_name) noexcept
  {
    self.name_ = std::move(new_name);
  }

  auto min_order_qty(this const Market& self) noexcept -> double
  {
    return self.min_order_qty_;
  }

  void min_order_qty(this Market& self, double new_min_order_qty) noexcept
  {
    self.min_order_qty_ = new_min_order_qty;
  }

  auto qty_step(this const Market& self) noexcept -> double
  {
    return self.qty_step_;
  }

  void qty_step(this Market& self, double new_qty_step) noexcept
  {
    self.qty_step_ = new_qty_step;
  }

private:
  std::string name_;

  double min_order_qty_;
  double qty_step_;
};

} // namespace pludux::backtest
