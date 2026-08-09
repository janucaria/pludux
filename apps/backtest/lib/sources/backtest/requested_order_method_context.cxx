module;

#include <cstddef>
#include <limits>
#include <string>
#include <vector>

export module pludux.backtest:requested_order_method_context;

import pludux;

import :requested_order;

export namespace pludux::backtest {

class RequestedOrderMethodContext {
public:
  explicit RequestedOrderMethodContext(const RequestedOrder& requested_order)
  : requested_order_{requested_order}
  {
  }

  auto requested_order(this const RequestedOrderMethodContext& self) noexcept
   -> const RequestedOrder&
  {
    return self.requested_order_;
  }

  auto call_series_method(this const RequestedOrderMethodContext&,
                          const std::string&,
                          AssetSnapshot) noexcept -> double
  {
    return std::numeric_limits<double>::quiet_NaN();
  }

  auto call_series_method(this const RequestedOrderMethodContext&,
                          const std::string&,
                          AssetSnapshot,
                          MethodOutput) noexcept -> double
  {
    return std::numeric_limits<double>::quiet_NaN();
  }

  auto get_series_result(this const RequestedOrderMethodContext&,
                         const std::string&,
                         std::size_t) noexcept -> double
  {
    return std::numeric_limits<double>::quiet_NaN();
  }

  auto get_series_results(this RequestedOrderMethodContext& self,
                          const auto&) noexcept -> std::vector<double>&
  {
    return self.unused_series_results_;
  }

  auto index(this const RequestedOrderMethodContext&) noexcept -> std::size_t
  {
    return 0;
  }

private:
  const RequestedOrder& requested_order_;
  std::vector<double> unused_series_results_;
};

} // namespace pludux::backtest
