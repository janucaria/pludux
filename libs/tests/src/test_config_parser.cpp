#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <pludux/screener.hpp>

#include <pludux/config_parser.hpp>

using namespace pludux;
using namespace pludux::screener;
using json = nlohmann::json;

TEST(ConfigParserTest, ParseScreenerSmaMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "SMA",
      "period": 14,
      "offset": 0,
      "target": {
        "method": "DATA",
        "field": "close",
        "offset": 0
      }
    }
  )");

  const auto method = parse_screener_method(config);
  const auto sma_method = screener_method_cast<SmaMethod>(&method);

  ASSERT_NE(sma_method, nullptr);

  EXPECT_EQ(sma_method->period(), 14);
  EXPECT_EQ(sma_method->offset(), 0);

  const auto target = screener_method_cast<DataMethod>(&sma_method->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "close");
  EXPECT_EQ(target->offset(), 0);
}

TEST(ConfigParserTest, ParseScreenerEmaMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "EMA",
      "period": 10,
      "offset": 1,
      "target": {
        "method": "DATA",
        "field": "open",
        "offset": 1
      }
    }
  )");

  const auto method = parse_screener_method(config);
  const auto ema_method = screener_method_cast<EmaMethod>(&method);

  ASSERT_NE(ema_method, nullptr);

  EXPECT_EQ(ema_method->period(), 10);
  EXPECT_EQ(ema_method->offset(), 1);

  const auto target = screener_method_cast<DataMethod>(&ema_method->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "open");
  EXPECT_EQ(target->offset(), 1);
}

TEST(ConfigParserTest, ParseScreenerWmaMethod)
{
  const auto config = json::parse(R"(
      {
        "method": "WMA",
        "period": 20,
        "offset": 2,
        "target": {
          "method": "DATA",
          "field": "high",
          "offset": 2
        }
      }
    )");

  const auto method = parse_screener_method(config);
  const auto wma_method = screener_method_cast<WmaMethod>(&method);

  ASSERT_NE(wma_method, nullptr);

  EXPECT_EQ(wma_method->period(), 20);
  EXPECT_EQ(wma_method->offset(), 2);

  const auto target = screener_method_cast<DataMethod>(&wma_method->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "high");
  EXPECT_EQ(target->offset(), 2);
}

TEST(ConfigParserTest, ParseScreenerRmaMethod)
{
  const auto config = json::parse(R"(
      {
        "method": "RMA",
        "period": 15,
        "offset": 3,
        "target": {
          "method": "DATA",
          "field": "low",
          "offset": 3
        }
      }
    )");

  const auto method = parse_screener_method(config);
  const auto rma_method = screener_method_cast<RmaMethod>(&method);

  ASSERT_NE(rma_method, nullptr);

  EXPECT_EQ(rma_method->period(), 15);
  EXPECT_EQ(rma_method->offset(), 3);

  const auto target = screener_method_cast<DataMethod>(&rma_method->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "low");
  EXPECT_EQ(target->offset(), 3);
}

TEST(ConfigParserTest, ParseScreenerHmaMethod)
{
  const auto config = json::parse(R"(
      {
        "method": "HMA",
        "period": 25,
        "offset": 4,
        "target": {
          "method": "DATA",
          "field": "volume",
          "offset": 4
        }
      }
    )");

  const auto method = parse_screener_method(config);
  const auto hma_method = screener_method_cast<HmaMethod>(&method);

  ASSERT_NE(hma_method, nullptr);

  EXPECT_EQ(hma_method->period(), 25);
  EXPECT_EQ(hma_method->offset(), 4);

  const auto target = screener_method_cast<DataMethod>(&hma_method->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "volume");
  EXPECT_EQ(target->offset(), 4);
}

TEST(ConfigParserTest, ParseScreenerRsiMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "RSI",
      "period": 14,
      "offset": 0,
      "target": {
        "method": "DATA",
        "field": "close",
        "offset": 0
      }
    }
  )");

  const auto method = parse_screener_method(config);

  const auto rsi_method = screener_method_cast<RsiMethod>(&method);
  ASSERT_NE(rsi_method, nullptr);

  EXPECT_EQ(rsi_method->period(), 14);
  EXPECT_EQ(rsi_method->offset(), 0);

  const auto target = screener_method_cast<DataMethod>(&rsi_method->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "close");
  EXPECT_EQ(target->offset(), 0);
}

TEST(ConfigParserTest, ParseScreenerValueMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "VALUE",
      "value": 100
    }
  )");

  const auto method = parse_screener_method(config);
  const auto value_method = screener_method_cast<ValueMethod>(&method);

  ASSERT_NE(value_method, nullptr);
  EXPECT_EQ(value_method->value(), 100);
}

TEST(ConfigParserTest, ParseScreenerDataMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "DATA",
      "field": "open",
      "offset": 0
    }
  )");

  const auto method = parse_screener_method(config);
  const auto field_method = screener_method_cast<DataMethod>(&method);

  ASSERT_NE(field_method, nullptr);
  EXPECT_EQ(field_method->field(), "open");
  EXPECT_EQ(field_method->offset(), 0);
}

TEST(ConfigParserTest, ParseScreenerInvalidMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "INVALID"
    }
  )");

  EXPECT_THROW(parse_screener_method(config), std::invalid_argument);
}

TEST(ConfigParserTest, ParseScreenerAllOfFilter)
{
  const auto config = json::parse(R"(
    {
      "filter": "ALL_OF",
      "filters": [
        {
          "filter": "GREATER_THAN",
          "threshold": {
            "method": "VALUE",
            "value": 100
          },
          "target": {
            "method": "DATA",
            "field": "close",
            "offset": 0
          }
        },
        {
          "filter": "LESS_THAN",
          "threshold": {
            "method": "VALUE",
            "value": 200
          },
          "target": {
            "method": "DATA",
            "field": "close",
            "offset": 0
          }
        }
      ]
    }
  )");

  const auto filter = parse_screener_filter(config);

  const auto all_of_filter = screener_filter_cast<AllOfFilter>(filter);
  ASSERT_NE(all_of_filter, nullptr);

  const auto filters = all_of_filter->filters();
  ASSERT_EQ(filters.size(), 2);

  const auto greater_than_filter =
   screener_filter_cast<GreaterThanFilter>(filters[0]);
  const auto less_than_filter =
   screener_filter_cast<LessThanFilter>(filters[1]);

  EXPECT_NE(greater_than_filter, nullptr);
  EXPECT_NE(less_than_filter, nullptr);
}

TEST(ConfigParserTest, ParseScreenerAnyOfFilter)
{
  const auto config = json::parse(R"(
    {
      "filter": "ANY_OF",
      "filters": [
        {
          "filter": "GREATER_THAN",
          "threshold": {
            "method": "VALUE",
            "value": 100
          },
          "target": {
            "method": "DATA",
            "field": "close",
            "offset": 0
          }
        },
        {
          "filter": "LESS_THAN",
          "threshold": {
            "method": "VALUE",
            "value": 200
          },
          "target": {
            "method": "DATA",
            "field": "close",
            "offset": 0
          }
        }
      ]
    }
  )");

  const auto filter = parse_screener_filter(config);

  const auto any_of_filter = screener_filter_cast<AnyOfFilter>(filter);
  ASSERT_NE(any_of_filter, nullptr);

  const auto filters = any_of_filter->filters();
  ASSERT_EQ(filters.size(), 2);

  const auto greater_than_filter =
   screener_filter_cast<GreaterThanFilter>(filters[0]);
  const auto less_than_filter =
   screener_filter_cast<LessThanFilter>(filters[1]);

  EXPECT_NE(greater_than_filter, nullptr);
  EXPECT_NE(less_than_filter, nullptr);
}

TEST(ConfigParserTest, ParseScreenerGreaterThanFilter)
{
  const auto config = json::parse(R"(
    {
      "filter": "GREATER_THAN",
      "threshold": {
        "method": "VALUE",
        "value": 100
      },
      "target": {
        "method": "DATA",
        "field": "close",
        "offset": 0
      }
    }
  )");

  const auto filter = parse_screener_filter(config);

  const auto greater_than_filter =
   screener_filter_cast<GreaterThanFilter>(filter);
  ASSERT_NE(greater_than_filter, nullptr);

  const auto target =
   screener_method_cast<DataMethod>(&greater_than_filter->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "close");
  EXPECT_EQ(target->offset(), 0);

  const auto threshold =
   screener_method_cast<ValueMethod>(&greater_than_filter->threshold());
  ASSERT_NE(threshold, nullptr);

  EXPECT_EQ(threshold->value(), 100);
}

TEST(ConfigParserTest, ParseScreenerGreaterEqualFilter)
{
  const auto config = json::parse(R"(
    {
      "filter": "GREATER_EQUAL",
      "threshold": {
        "method": "VALUE",
        "value": 100
      },
      "target": {
        "method": "DATA",
        "field": "close",
        "offset": 0
      }
    }
  )");

  const auto filter = parse_screener_filter(config);

  const auto greater_equal_filter =
   screener_filter_cast<GreaterEqualFilter>(filter);
  ASSERT_NE(greater_equal_filter, nullptr);

  const auto target =
   screener_method_cast<DataMethod>(&greater_equal_filter->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "close");
  EXPECT_EQ(target->offset(), 0);
}

TEST(ConfigParserTest, ParseScreenerLessThanFilter)
{
  const auto config = json::parse(R"(
    {
      "filter": "LESS_THAN",
      "threshold": {
        "method": "VALUE",
        "value": 100
      },
      "target": {
        "method": "DATA",
        "field": "close",
        "offset": 0
      }
    }
  )");

  const auto filter = parse_screener_filter(config);

  const auto less_than_filter = screener_filter_cast<LessThanFilter>(filter);
  ASSERT_NE(less_than_filter, nullptr);

  const auto target =
   screener_method_cast<DataMethod>(&less_than_filter->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "close");
  EXPECT_EQ(target->offset(), 0);
}

TEST(ConfigParserTest, ParseScreenerLessEqualFilter)
{
  const auto config = json::parse(R"(
    {
      "filter": "LESS_EQUAL",
      "threshold": {
        "method": "VALUE",
        "value": 100
      },
      "target": {
        "method": "DATA",
        "field": "close",
        "offset": 0
      }
    }
  )");

  const auto filter = parse_screener_filter(config);

  const auto less_equal_filter = screener_filter_cast<LessEqualFilter>(filter);
  ASSERT_NE(less_equal_filter, nullptr);

  const auto target =
   screener_method_cast<DataMethod>(&less_equal_filter->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "close");
  EXPECT_EQ(target->offset(), 0);
}

TEST(ConfigParserTest, ParseScreenerInvalidFilter)
{
  const auto config = json::parse(R"(
    {
      "filter": "invalid"
    }
  )");

  EXPECT_THROW(parse_screener_filter(config), std::invalid_argument);
}