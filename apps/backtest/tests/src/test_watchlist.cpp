#include <gtest/gtest.h>

#include <stdexcept>
#include <vector>

import pludux.backtest;

using namespace pludux::backtest;

TEST(WatchlistTest, StoresOrderedAssetsAndName)
{
  const auto first = AssetStoreHandle{1, 1};
  const auto second = AssetStoreHandle{2, 1};
  const auto watchlist = Watchlist{"Crypto", {first, second}};

  EXPECT_EQ(watchlist.name(), "Crypto");
  EXPECT_EQ(watchlist.asset_handles(),
            (std::vector<AssetStoreHandle>{first, second}));
}

TEST(WatchlistTest, AllowsEmptyConfiguration)
{
  const auto watchlist = Watchlist{"Empty"};
  EXPECT_TRUE(watchlist.asset_handles().empty());
}

TEST(WatchlistTest, RejectsDuplicateAssets)
{
  const auto asset = AssetStoreHandle{1, 1};
  EXPECT_THROW((Watchlist{"Duplicate", {asset, asset}}), std::invalid_argument);
}

TEST(WatchlistTest, NameDoesNotChangeEquivalentRules)
{
  const auto assets = std::vector<AssetStoreHandle>{{1, 1}, {2, 1}};
  EXPECT_TRUE(
   (Watchlist{"First", assets}.equivalent_rules(Watchlist{"Second", assets})));
}
