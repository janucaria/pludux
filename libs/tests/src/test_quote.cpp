#include <gtest/gtest.h>
#include <pludux/quote.hpp>

namespace pludux {

TEST(QuoteTest, ConstructorInitializesMembers)
{
  const auto timestamp = std::time_t{1627849200};
  const auto open = double{100};
  const auto high = double{150};
  const auto low = double{90};
  const auto close = double{120};
  const auto volume = std::uint32_t{1000};

  const auto quote = Quote{timestamp, open, high, low, close, volume};

  EXPECT_EQ(quote.timestamp(), timestamp);
  EXPECT_EQ(quote.open(), open);
  EXPECT_EQ(quote.high(), high);
  EXPECT_EQ(quote.low(), low);
  EXPECT_EQ(quote.close(), close);
  EXPECT_EQ(quote.volume(), volume);
}

TEST(QuoteTest, TimestampGetter)
{
  const auto timestamp = std::time_t{1627849200};
  const auto quote = Quote{timestamp, 100, 150, 90, 120, 1000};
  EXPECT_EQ(quote.timestamp(), timestamp);
}

TEST(QuoteTest, OpenGetter)
{
  const auto open = double{100};
  const auto quote = Quote{1627849200, open, 150, 90, 120, 1000};
  EXPECT_EQ(quote.open(), open);
}

TEST(QuoteTest, HighGetter)
{
  const auto high = double{150};
  const auto quote = Quote{1627849200, 100, high, 90, 120, 1000};
  EXPECT_EQ(quote.high(), high);
}

TEST(QuoteTest, LowGetter)
{
  const auto low = double{90};
  const auto quote = Quote{1627849200, 100, 150, low, 120, 1000};
  EXPECT_EQ(quote.low(), low);
}

TEST(QuoteTest, CloseGetter)
{
  const auto close = double{120};
  const auto quote = Quote{1627849200, 100, 150, 90, close, 1000};
  EXPECT_EQ(quote.close(), close);
}

TEST(QuoteTest, VolumeGetter)
{
  const auto volume = std::uint32_t{1000};
  const auto quote = Quote{1627849200, 100, 150, 90, 120, volume};
  EXPECT_EQ(quote.volume(), volume);
}

TEST(QuoteTest, AllGetters)
{
  const auto timestamp = std::time_t{1627849200};
  const auto open = double{100};
  const auto high = double{150};
  const auto low = double{90};
  const auto close = double{120};
  const auto volume = std::uint32_t{1000};

  const auto quote = Quote{timestamp, open, high, low, close, volume};

  EXPECT_EQ(quote.timestamp(), timestamp);
  EXPECT_EQ(quote.open(), open);
  EXPECT_EQ(quote.high(), high);
  EXPECT_EQ(quote.low(), low);
  EXPECT_EQ(quote.close(), close);
  EXPECT_EQ(quote.volume(), volume);
}

} // namespace pludux