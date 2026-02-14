module;

#include <cstddef>
#include <ctime>
#include <string>
#include <string_view>
#include <unordered_map>

export module pludux:asset_quote_field_resolver;

import :asset_series;
import :asset_history;

export namespace pludux {

class AssetQuoteFieldResolver {
public:
  AssetQuoteFieldResolver()
  : AssetQuoteFieldResolver{
     "Datetime", "Open", "High", "Low", "Close", "Volume"}
  {
  }

  AssetQuoteFieldResolver(std::string datetime_field,
                          std::string open_field,
                          std::string high_field,
                          std::string low_field,
                          std::string close_field,
                          std::string volume_field)
  : datetime_field_{std::move(datetime_field)}
  , open_field_{std::move(open_field)}
  , high_field_{std::move(high_field)}
  , low_field_{std::move(low_field)}
  , close_field_{std::move(close_field)}
  , volume_field_{std::move(volume_field)}
  {
  }

  auto operator==(const AssetQuoteFieldResolver& other) const noexcept -> bool = default;

  auto datetime_field(this const AssetQuoteFieldResolver& self) noexcept
   -> const std::string&
  {
    return self.datetime_field_;
  }

  void datetime_field(this AssetQuoteFieldResolver& self,
                      std::string field) noexcept
  {
    self.datetime_field_ = std::move(field);
  }

  auto open_field(this const AssetQuoteFieldResolver& self) noexcept
   -> const std::string&
  {
    return self.open_field_;
  }

  void open_field(this AssetQuoteFieldResolver& self,
                  std::string field) noexcept
  {
    self.open_field_ = std::move(field);
  }

  auto high_field(this const AssetQuoteFieldResolver& self) noexcept
   -> const std::string&
  {
    return self.high_field_;
  }

  void high_field(this AssetQuoteFieldResolver& self,
                  std::string field) noexcept
  {
    self.high_field_ = std::move(field);
  }

  auto low_field(this const AssetQuoteFieldResolver& self) noexcept
   -> const std::string&
  {
    return self.low_field_;
  }

  void low_field(this AssetQuoteFieldResolver& self, std::string field) noexcept
  {
    self.low_field_ = std::move(field);
  }

  auto close_field(this const AssetQuoteFieldResolver& self) noexcept
   -> const std::string&
  {
    return self.close_field_;
  }

  void close_field(this AssetQuoteFieldResolver& self,
                   std::string field) noexcept
  {
    self.close_field_ = std::move(field);
  }

  auto volume_field(this const AssetQuoteFieldResolver& self) noexcept
   -> const std::string&
  {
    return self.volume_field_;
  }

  void volume_field(this AssetQuoteFieldResolver& self,
                    std::string field) noexcept
  {
    self.volume_field_ = std::move(field);
  }

  auto get_datetimes(this const AssetQuoteFieldResolver& self,
                     const AssetHistory& history) noexcept -> AssetSeries
  {
    return history[self.datetime_field_];
  }

  auto get_opens(this const AssetQuoteFieldResolver& self,
                 const AssetHistory& history) noexcept -> AssetSeries
  {
    return history[self.open_field_];
  }

  auto get_highs(this const AssetQuoteFieldResolver& self,
                 const AssetHistory& history) noexcept -> AssetSeries
  {
    return history[self.high_field_];
  }

  auto get_lows(this const AssetQuoteFieldResolver& self,
                const AssetHistory& history) noexcept -> AssetSeries
  {
    return history[self.low_field_];
  }

  auto get_closes(this const AssetQuoteFieldResolver& self,
                  const AssetHistory& history) noexcept -> AssetSeries
  {
    return history[self.close_field_];
  }

  auto get_volumes(this const AssetQuoteFieldResolver& self,
                   const AssetHistory& history) noexcept -> AssetSeries
  {
    return history[self.volume_field_];
  }

private:
  std::string datetime_field_;
  std::string open_field_;
  std::string high_field_;
  std::string low_field_;
  std::string close_field_;
  std::string volume_field_;
};

} // namespace pludux
