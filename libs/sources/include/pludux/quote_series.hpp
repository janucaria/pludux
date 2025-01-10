#ifndef PLUDUX_PLUDUX_QUOTE_HISTOTY_HPP
#define PLUDUX_PLUDUX_QUOTE_HISTOTY_HPP

#include <ctime>
#include <string>
#include <vector>
#include <iterator>

#include <pludux/quote.hpp>
#include <pludux/series.hpp>

namespace pludux {

class QuoteSeries {
public:
  class ConstIterator {
  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = const Quote;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    ConstIterator(const QuoteSeries& quote_history, std::size_t index);

    auto operator++() -> ConstIterator&;

    auto operator++(int) -> ConstIterator;

    auto operator--() -> ConstIterator&;

    auto operator--(int) -> ConstIterator;

    auto operator*() const -> value_type;

    auto operator==(const ConstIterator& other) const -> bool;

    auto operator!=(const ConstIterator& other) const -> bool;

  private:
    const QuoteSeries& quote_series_;
    std::size_t index_;
  };

  using Iterator = ConstIterator;
  using ReverseIterator = std::reverse_iterator<Iterator>;
  using ConstReverseIterator = std::reverse_iterator<ConstIterator>;

  QuoteSeries();

  QuoteSeries(std::size_t size,
               std::vector<std::time_t> timestamps,
               std::vector<double> opens,
               std::vector<double> highs,
               std::vector<double> lows,
               std::vector<double> closes,
               std::vector<std::uint32_t> volumes);
  
  auto operator[](std::size_t index) const -> Quote;

  auto timestamps() const noexcept -> const Series&;

  auto opens() const noexcept -> const Series&;

  auto highs() const noexcept -> const Series&;

  auto lows() const noexcept -> const Series&;

  auto closes() const noexcept -> const Series&;

  auto volumes() const noexcept -> const Series&;

  void add_quote(const Quote& quote);

  auto size() const noexcept -> std::size_t;

  auto begin() const -> Iterator;

  auto end() const -> Iterator;

  auto cbegin() const -> ConstIterator;

  auto cend() const -> ConstIterator;

  auto rbegin() const -> ReverseIterator;

  auto rend() const -> ReverseIterator;

  auto crbegin() const -> ConstReverseIterator;

  auto crend() const -> ConstReverseIterator;

private:
  std::size_t size_;
  Series timestamps_;
  Series opens_;
  Series highs_;
  Series lows_;
  Series closes_;
  Series volumes_;
};

} // namespace pludux

#endif
