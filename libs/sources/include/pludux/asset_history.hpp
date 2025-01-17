#ifndef PLUDUX_PLUDUX_ASSET_HISTOTY_HPP
#define PLUDUX_PLUDUX_ASSET_HISTOTY_HPP

#include <ctime>
#include <iterator>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <pludux/asset_snapshot.hpp>
#include <pludux/series.hpp>

namespace pludux {

class AssetHistory {
public:
  class ConstIterator {
  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = const AssetSnapshot;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    ConstIterator(const AssetHistory& asset_history, std::size_t index);

    auto operator++() -> ConstIterator&;

    auto operator++(int) -> ConstIterator;

    auto operator--() -> ConstIterator&;

    auto operator--(int) -> ConstIterator;

    auto operator*() const -> value_type;

    auto operator==(const ConstIterator& other) const -> bool;

    auto operator!=(const ConstIterator& other) const -> bool;

  private:
    const AssetHistory& asset_history_;
    std::size_t index_;
  };

  using Iterator = ConstIterator;
  using ReverseIterator = std::reverse_iterator<Iterator>;
  using ConstReverseIterator = std::reverse_iterator<ConstIterator>;

  AssetHistory(
   std::initializer_list<std::pair<std::string, std::vector<double>>> data);

  template<typename TBegin, typename TEnd>
  AssetHistory(TBegin begin_it, TEnd end_it)
  : size_{}
  , history_data_{}
  {
    if(begin_it == end_it) {
      return;
    }

    size_ = begin_it->second.size();
    for(auto it = begin_it; it != end_it; ++it) {
      auto data = it->second;
      data.resize(size_, std::numeric_limits<double>::quiet_NaN());
      std::reverse(data.begin(), data.end());

      history_data_.emplace(it->first, DataSeries<double>{std::move(data)});
    }
  }

  operator AssetSnapshot() const noexcept;

  auto operator[](std::size_t index) const noexcept -> AssetSnapshot;

  auto operator[](const std::string& key) const
   -> RefSeries<const DataSeries<double>>;

  auto size() const noexcept -> std::size_t;

  auto contains(const std::string& key) const noexcept -> bool;

  auto begin() const -> Iterator;

  auto end() const -> Iterator;

  auto rbegin() const -> ReverseIterator;

  auto rend() const -> ReverseIterator;

private:
  std::size_t size_;
  typename AssetSnapshot::HistoryData history_data_;
};

} // namespace pludux

#endif
