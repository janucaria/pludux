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

  Market(std::string name, double min_order_quantity)
  : Market{name, min_order_quantity, min_order_quantity}
  {
  }

  Market(std::string name, double min_order_quantity, double quantity_step)
  : name_{std::move(name)}
  , min_order_quantity_{min_order_quantity}
  , quantity_step_{quantity_step}
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

  auto min_order_quantity(this const Market& self) noexcept -> double
  {
    return self.min_order_quantity_;
  }

  void min_order_quantity(this Market& self, double new_min_order_qty) noexcept
  {
    self.min_order_quantity_ = new_min_order_qty;
  }

  auto quantity_step(this const Market& self) noexcept -> double
  {
    return self.quantity_step_;
  }

  void quantity_step(this Market& self, double new_qty_step) noexcept
  {
    self.quantity_step_ = new_qty_step;
  }

private:
  std::string name_;

  double min_order_quantity_;
  double quantity_step_;
};

} // namespace pludux::backtest
