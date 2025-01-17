#include <pludux/asset_snapshot.hpp>

namespace pludux {

AssetSnapshot::AssetSnapshot(std::size_t offset,
                             const HistoryData& history_data)
: offset_{offset}
, history_data_{history_data}
{
}

auto AssetSnapshot::operator[](std::string key) const
 -> SubSeries<RefSeries<const DataSeries<double>>>
{
  return SubSeries{RefSeries{history_data_.at(key)}, offset_};
}

auto AssetSnapshot::offset() const noexcept -> std::size_t
{
  return offset_;
}

auto AssetSnapshot::size() const noexcept -> std::size_t
{
  auto it = history_data_.begin();
  if(it == history_data_.end()) {
    return 0;
  }

  return it->second.size();
}

auto AssetSnapshot::contains(const std::string& key) const noexcept -> bool
{
  return history_data_.contains(key);
}

} // namespace pludux
