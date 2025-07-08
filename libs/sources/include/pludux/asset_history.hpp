#ifndef PLUDUX_PLUDUX_ASSET_HISTOTY_HPP
#define PLUDUX_PLUDUX_ASSET_HISTOTY_HPP
import pludux.series;

#include <ctime>
#include <initializer_list>
#include <iterator>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>



namespace pludux {

class AssetHistory {
public:
  AssetHistory(
   std::initializer_list<std::pair<std::string, PolySeries<double>>> data);

  template<typename TInputIt>
  AssetHistory(TInputIt begin_it, TInputIt end_it)
  : history_data_(begin_it, end_it)
  , size_(0)
  , datetime_key_{"Datetime"}
  , open_key_{"Open"}
  , high_key_{"High"}
  , low_key_{"Low"}
  , close_key_{"Close"}
  , volume_key_{"Volume"}
  {
    recalculate_size_();
  }

  auto operator[](std::string_view key) const
   -> RefSeries<const PolySeries<double>>;

  auto datetime_key() const noexcept -> const std::string&;

  void datetime_key(std::string key) noexcept;

  auto open_key() const noexcept -> const std::string&;

  void open_key(std::string key) noexcept;

  auto high_key() const noexcept -> const std::string&;

  void high_key(std::string key) noexcept;

  auto low_key() const noexcept -> const std::string&;

  void low_key(std::string key) noexcept;

  auto close_key() const noexcept -> const std::string&;

  void close_key(std::string key) noexcept;

  auto volume_key() const noexcept -> const std::string&;

  void volume_key(std::string key) noexcept;

  auto get_datetimes() const noexcept -> RefSeries<const PolySeries<double>>;

  auto get_opens() const noexcept -> RefSeries<const PolySeries<double>>;

  auto get_highs() const noexcept -> RefSeries<const PolySeries<double>>;

  auto get_lows() const noexcept -> RefSeries<const PolySeries<double>>;

  auto get_closes() const noexcept -> RefSeries<const PolySeries<double>>;

  auto get_volumes() const noexcept -> RefSeries<const PolySeries<double>>;

  auto size() const noexcept -> std::size_t;

  auto history_data() const noexcept
   -> const std::unordered_map<std::string, PolySeries<double>>&;

  auto contains(std::string_view key) const noexcept -> bool;

  void insert(std::string key, PolySeries<double> series);

private:
  std::unordered_map<std::string, PolySeries<double>> history_data_;
  std::size_t size_;
  std::string datetime_key_;
  std::string open_key_;
  std::string high_key_;
  std::string low_key_;
  std::string close_key_;
  std::string volume_key_;

  void recalculate_size_() noexcept;
};

} // namespace pludux

#endif
