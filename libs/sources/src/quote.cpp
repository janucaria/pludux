#include <pludux/Quote.hpp>

namespace pludux {

Quote::Quote(std::time_t timestamp,
             double open,
             double high,
             double low,
             double close,
             std::uint32_t volume)
: timestamp_{timestamp}
, open_{open}
, high_{high}
, low_{low}
, close_{close}
, volume_{volume}
{
}


auto Quote::timestamp() const noexcept -> std::time_t
{
  return timestamp_;
}

auto Quote::open() const noexcept -> double
{
  return open_;
}

auto Quote::high() const noexcept -> double
{
  return high_;
}

auto Quote::low() const noexcept -> double
{
  return low_;
}

auto Quote::close() const noexcept -> double
{
  return close_;
}

auto Quote::volume() const noexcept -> std::uint32_t
{
  return volume_;
}

} // namespace pludux
