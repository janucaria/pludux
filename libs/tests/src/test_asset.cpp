#include <gtest/gtest.h>
#include <pludux/asset.hpp>
#include <pludux/quote.hpp>

namespace pludux {

TEST(AssetTest, ConstructorInitializesSymbol)
{
  const auto symbol = std::string{"AAPL"};
  const auto asset = Asset{symbol};
  EXPECT_EQ(asset.symbol(), symbol);
}

TEST(AssetTest, SymbolGetter)
{
  const auto symbol = std::string{"AAPL"};
  const auto asset = Asset{symbol};
  EXPECT_EQ(asset.symbol(), symbol);
}

TEST(AssetTest, AddQuote)
{
  const auto symbol = std::string{"AAPL"};
  auto asset = Asset{symbol};

  const auto timestamp = std::time_t{1627849200};
  const auto open = double{100};
  const auto high = double{150};
  const auto low = double{90};
  const auto close = double{120};
  const auto volume = std::uint32_t{1000};

  const auto quote = Quote{timestamp, open, high, low, close, volume};
  asset.add_quote(quote);

  const auto& quotes = asset.quotes();
  ASSERT_EQ(quotes.size(), 1);
  EXPECT_EQ(quotes[0].timestamp(), timestamp);
  EXPECT_EQ(quotes[0].open(), open);
  EXPECT_EQ(quotes[0].high(), high);
  EXPECT_EQ(quotes[0].low(), low);
  EXPECT_EQ(quotes[0].close(), close);
  EXPECT_EQ(quotes[0].volume(), volume);
}

TEST(AssetTest, QuotesGetter)
{
  const auto symbol = std::string{"AAPL"};
  auto asset = Asset{symbol};

  const auto timestamp1 = std::time_t{1627849200};
  const auto open1 = double{100};
  const auto high1 = double{150};
  const auto low1 = double{90};
  const auto close1 = double{120};
  const auto volume1 = std::uint32_t{1000};

  const auto quote1 = Quote{timestamp1, open1, high1, low1, close1, volume1};
  asset.add_quote(quote1);

  const auto timestamp2 = std::time_t{1627935600};
  const auto open2 = double{110};
  const auto high2 = double{160};
  const auto low2 = double{100};
  const auto close2 = double{130};
  const auto volume2 = std::uint32_t{2000};

  const auto quote2 = Quote{timestamp2, open2, high2, low2, close2, volume2};
  asset.add_quote(quote2);

  const auto& quotes = asset.quotes();
  ASSERT_EQ(quotes.size(), 2);
  EXPECT_EQ(quotes[0].timestamp(), timestamp1);
  EXPECT_EQ(quotes[0].open(), open1);
  EXPECT_EQ(quotes[0].high(), high1);
  EXPECT_EQ(quotes[0].low(), low1);
  EXPECT_EQ(quotes[0].close(), close1);
  EXPECT_EQ(quotes[0].volume(), volume1);

  EXPECT_EQ(quotes[1].timestamp(), timestamp2);
  EXPECT_EQ(quotes[1].open(), open2);
  EXPECT_EQ(quotes[1].high(), high2);
  EXPECT_EQ(quotes[1].low(), low2);
  EXPECT_EQ(quotes[1].close(), close2);
  EXPECT_EQ(quotes[1].volume(), volume2);
}

} // namespace pludux