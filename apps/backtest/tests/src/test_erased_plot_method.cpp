#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

import pludux.backtest;

namespace {

class ConcretePlotContext {
public:
  explicit ConcretePlotContext(
   std::shared_ptr<std::vector<double>> rendered_values)
  : rendered_values_{std::move(rendered_values)}
  {
  }

  void render_plot_line(const std::vector<double>& data, std::uint32_t)
  {
    *rendered_values_ = data;
  }

  void render_plot_histogram(const std::vector<double>& data, std::uint32_t)
  {
    *rendered_values_ = data;
  }

  void render_plot_momentum_histogram(const std::vector<double>& data,
                                      const std::vector<std::uint32_t>&)
  {
    *rendered_values_ = data;
  }

  auto series_results(const std::string&) const
   -> std::optional<std::reference_wrapper<const std::vector<double>>>
  {
    return std::nullopt;
  }

  auto results_size() const -> std::size_t
  {
    return 3;
  }

private:
  std::shared_ptr<std::vector<double>> rendered_values_;
};

} // namespace

TEST(ErasedPlotMethodTest, UsesConcreteContextType)
{
  using namespace pludux::backtest;

  auto rendered_values = std::make_shared<std::vector<double>>();
  const auto method =
   ErasedPlotMethod<ConcretePlotContext>{HLinePlotMethod{42.0}};

  method(ConcretePlotContext{rendered_values});

  EXPECT_EQ(*rendered_values, (std::vector<double>{42.0, 42.0, 42.0}));
}

TEST(ErasedPlotSourceMethodTest, UsesConcreteContextType)
{
  using namespace pludux::backtest;

  auto rendered_values = std::make_shared<std::vector<double>>();
  const auto source =
   ErasedPlotSourceMethod<ConcretePlotContext>{ConstantPlotSourceMethod{7.0}};

  EXPECT_EQ(source(ConcretePlotContext{rendered_values}),
            (std::vector<double>{7.0, 7.0, 7.0}));
}
