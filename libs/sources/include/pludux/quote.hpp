#ifndef PLUDUX_PLUDUX_QUOTE_HPP
#define PLUDUX_PLUDUX_QUOTE_HPP

#include <ctime>
#include <cstdint>

namespace pludux {

class Quote {
public:
  Quote();

  Quote(std::time_t timestamp,
        double open,
        double high,
        double low,
        double close,
        std::uint32_t volume);

  auto timestamp() const noexcept -> std::time_t;

  auto open() const noexcept -> double;

  auto high() const noexcept -> double;

  auto low() const noexcept -> double;

  auto close() const noexcept -> double;

  auto volume() const noexcept -> std::uint32_t;

private:
  std::time_t timestamp_;
  double open_;
  double high_;
  double low_;
  double close_;
  std::uint32_t volume_;
};

} // namespace pludux

#endif