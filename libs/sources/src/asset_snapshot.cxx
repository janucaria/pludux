module;

#include <cstddef>
#include <ctime>
#include <string>
#include <string_view>
#include <unordered_map>

export module pludux:asset_snapshot;

import :asset_series;
import :asset_history;
import :asset_quote_field_resolver;

export namespace pludux {

auto get_default_asset_field_resolver() noexcept
 -> const AssetQuoteFieldResolver&
{
  static const auto default_resolver = AssetQuoteFieldResolver{};
  return default_resolver;
}

class AssetSnapshot {
public:
  AssetSnapshot(const AssetHistory& asset_history) noexcept
  : AssetSnapshot{asset_history, get_default_asset_field_resolver()}
  {
  }

  AssetSnapshot(const AssetHistory& asset_history,
                const AssetQuoteFieldResolver& field_resolver) noexcept
  : AssetSnapshot{0, asset_history, field_resolver}
  {
  }

  AssetSnapshot(std::size_t lookback,
                const AssetHistory& asset_history,
                const AssetQuoteFieldResolver& field_resolver) noexcept
  : lookback_{lookback}
  , asset_history_{asset_history}
  , field_resolver_{field_resolver}
  {
  }

  auto operator[](this AssetSnapshot self, const std::string& field) noexcept
   -> double
  {
    return self.data(field);
  }

  auto operator[](this AssetSnapshot self, std::size_t index) noexcept
   -> AssetSnapshot
  {
    return AssetSnapshot{
     self.lookback_ + index, self.asset_history_, self.field_resolver_};
  }

  auto lookback(this AssetSnapshot self) noexcept -> std::size_t
  {
    return self.lookback_;
  }

  auto size(this AssetSnapshot self) noexcept -> std::size_t
  {
    if(self.lookback_ >= self.asset_history_.size()) {
      return 0;
    }

    return self.asset_history_.size() - self.lookback_;
  }

  auto contains(this AssetSnapshot self, const std::string& field) noexcept
   -> bool
  {
    return self.asset_history_.contains(field);
  }

  auto datetime(this AssetSnapshot self) noexcept -> double
  {
    return self.field_resolver_.get_datetimes(
     self.asset_history_)[self.lookback_];
  }

  auto open(this AssetSnapshot self) noexcept -> double
  {
    return self.field_resolver_.get_opens(self.asset_history_)[self.lookback_];
  }

  auto high(this AssetSnapshot self) noexcept -> double
  {
    return self.field_resolver_.get_highs(self.asset_history_)[self.lookback_];
  }

  auto low(this AssetSnapshot self) noexcept -> double
  {
    return self.field_resolver_.get_lows(self.asset_history_)[self.lookback_];
  }

  auto close(this AssetSnapshot self) noexcept -> double
  {
    return self.field_resolver_.get_closes(self.asset_history_)[self.lookback_];
  }

  auto volume(this AssetSnapshot self) noexcept -> double
  {
    return self.field_resolver_.get_volumes(
     self.asset_history_)[self.lookback_];
  }

  auto data(this AssetSnapshot self, const std::string& field) noexcept
   -> double
  {
    return self.asset_history_[field][self.lookback_];
  }

private:
  std::size_t lookback_;
  const AssetHistory& asset_history_;
  const AssetQuoteFieldResolver& field_resolver_;
};

} // namespace pludux
