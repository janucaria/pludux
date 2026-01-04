#include <gtest/gtest.h>

#include <jsoncons/json.hpp>

import pludux;

using namespace pludux;
using json = jsoncons::ojson;

class ConfigParserTest : public ::testing::Test {
protected:
  ConfigParser config_parser;

  void SetUp() override
  {
    config_parser = std::move(make_default_registered_config_parser());

    config_parser.parse_method(json::parse(R"(
      {
        "method": "DATA",
        "name": "high",
        "field": "high"
      }
    )"));

    config_parser.parse_method(json::parse(R"(
      {
        "method": "DATA",
        "name": "low",
        "field": "low"
      }
    )"));

    config_parser.parse_method(json::parse(R"(
      {
        "method": "DATA",
        "name": "close",
        "field": "close"
      }
    )"));

    config_parser.parse_method(json::parse(R"(
      {
        "method": "DATA",
        "name": "volume",
        "field": "volume"
      }
    )"));
  }
};

TEST_F(ConfigParserTest, ParseScreenerReferenceMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "REFERENCE",
      "name": "close"
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto reference_method = series_method_cast<ReferenceMethod>(method);
  ASSERT_NE(reference_method, nullptr);

  EXPECT_EQ(reference_method->name(), "close");

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScrennerLookbackMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "LOOKBACK",
      "period": 3,
      "source": {
        "method": "CLOSE"
      }
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto lookback_method =
   series_method_cast<LookbackMethod<AnySeriesMethod>>(method);
  ASSERT_NE(lookback_method, nullptr);

  EXPECT_EQ(lookback_method->period(), 3);
  const auto source_method =
   series_method_cast<CloseMethod>(lookback_method->source());
  ASSERT_NE(source_method, nullptr);

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerSelectOutputMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "SELECT_OUTPUT",
      "name": "upper-band",
      "source": {
        "method": "MACD",
        "fastPeriod": 12,
        "slowPeriod": 26,
        "signalPeriod": 9,
        "input": {
          "method": "CLOSE"
        }
      }
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto select_output_method =
   series_method_cast<SelectOutputMethod<AnySeriesMethod>>(method);
  ASSERT_NE(select_output_method, nullptr);
  EXPECT_EQ(select_output_method->output(), SeriesOutput::UpperBand);

  const auto macd_method = series_method_cast<MacdMethod<AnySeriesMethod>>(
   select_output_method->source());
  ASSERT_NE(macd_method, nullptr);

  const auto source = series_method_cast<CloseMethod>(macd_method->source());
  EXPECT_NE(source, nullptr);
  EXPECT_EQ(macd_method->fast_period(), 12);
  EXPECT_EQ(macd_method->slow_period(), 26);
  EXPECT_EQ(macd_method->signal_period(), 9);

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerOpenMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "OPEN"
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto open_method = series_method_cast<OpenMethod>(method);
  ASSERT_NE(open_method, nullptr);

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerHighMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "HIGH"
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto high_method = series_method_cast<HighMethod>(method);
  ASSERT_NE(high_method, nullptr);

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerLowMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "LOW"
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto low_method = series_method_cast<LowMethod>(method);
  ASSERT_NE(low_method, nullptr);

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerCloseMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "CLOSE"
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto close_method = series_method_cast<CloseMethod>(method);
  ASSERT_NE(close_method, nullptr);

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerVolumeMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "VOLUME"
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto volume_method = series_method_cast<VolumeMethod>(method);
  ASSERT_NE(volume_method, nullptr);

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerSmaMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "SMA",
      "period": 14,
      "source": {
        "method": "DATA",
        "field": "close"
      }
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto sma_method =
   series_method_cast<SmaMethod<AnySeriesMethod>>(method);
  ASSERT_NE(sma_method, nullptr);

  EXPECT_EQ(sma_method->period(), 14);

  const auto source = series_method_cast<DataMethod>(sma_method->source());
  ASSERT_NE(source, nullptr);

  EXPECT_EQ(source->field(), "close");

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerEmaMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "EMA",
      "period": 10,
      "source": {
        "method": "DATA",
        "field": "open"
      }
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto ema_method =
   series_method_cast<EmaMethod<AnySeriesMethod>>(method);
  ASSERT_NE(ema_method, nullptr);

  EXPECT_EQ(ema_method->period(), 10);

  const auto source = series_method_cast<DataMethod>(ema_method->source());
  ASSERT_NE(source, nullptr);

  EXPECT_EQ(source->field(), "open");

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerWmaMethod)
{
  const auto config = json::parse(R"(
      {
        "method": "WMA",
        "period": 20,
        "source": {
          "method": "DATA",
          "field": "high"
        }
      }
    )");

  const auto method = config_parser.parse_method(config);

  const auto wma_method =
   series_method_cast<WmaMethod<AnySeriesMethod>>(method);
  ASSERT_NE(wma_method, nullptr);

  EXPECT_EQ(wma_method->period(), 20);

  const auto source = series_method_cast<DataMethod>(wma_method->source());
  ASSERT_NE(source, nullptr);

  EXPECT_EQ(source->field(), "high");

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerRmaMethod)
{
  const auto config = json::parse(R"(
      {
        "method": "RMA",
        "period": 15,
        "source": {
          "method": "DATA",
          "field": "low"
        }
      }
    )");

  const auto method = config_parser.parse_method(config);

  const auto rma_method =
   series_method_cast<RmaMethod<AnySeriesMethod>>(method);
  ASSERT_NE(rma_method, nullptr);

  EXPECT_EQ(rma_method->period(), 15);

  const auto source = series_method_cast<DataMethod>(rma_method->source());
  ASSERT_NE(source, nullptr);

  EXPECT_EQ(source->field(), "low");

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerHmaMethod)
{
  const auto config = json::parse(R"(
      {
        "method": "HMA",
        "period": 25,
        "source": {
          "method": "DATA",
          "field": "volume"
        }
      }
    )");

  const auto method = config_parser.parse_method(config);

  const auto hma_method =
   series_method_cast<HmaMethod<AnySeriesMethod>>(method);
  ASSERT_NE(hma_method, nullptr);

  EXPECT_EQ(hma_method->period(), 25);

  const auto source = series_method_cast<DataMethod>(hma_method->source());
  ASSERT_NE(source, nullptr);

  EXPECT_EQ(source->field(), "volume");

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerRsiMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "RSI",
      "period": 14,
      "source": {
        "method": "DATA",
        "field": "close"
      }
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto rsi_method =
   series_method_cast<RsiMethod<AnySeriesMethod>>(method);
  ASSERT_NE(rsi_method, nullptr);

  EXPECT_EQ(rsi_method->period(), 14);

  const auto source = series_method_cast<DataMethod>(rsi_method->source());
  ASSERT_NE(source, nullptr);

  EXPECT_EQ(source->field(), "close");

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerValueMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "VALUE",
      "value": 100
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto value_method = series_method_cast<ValueMethod>(method);
  ASSERT_NE(value_method, nullptr);

  EXPECT_EQ(value_method->value(), 100);

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerDataMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "DATA",
      "field": "open"
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto field_method = series_method_cast<DataMethod>(method);
  ASSERT_NE(field_method, nullptr);

  EXPECT_EQ(field_method->field(), "open");

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerAtrMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "ATR",
      "period": 14,
      "maSmoothingType": "RMA"
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto atr_method = series_method_cast<AtrMethod>(method);
  ASSERT_NE(atr_method, nullptr);

  EXPECT_EQ(atr_method->period(), 14);
  EXPECT_EQ(atr_method->ma_smoothing_type(), MaMethodType::Rma);

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerBbMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "BB",
      "maType": "SMA",
      "period": 20,
      "stddev": 2.0,
      "maSource": {
        "method": "DATA",
        "field": "close"
      }
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto bb_method = series_method_cast<BbMethod<AnySeriesMethod>>(method);
  ASSERT_NE(bb_method, nullptr);

  const auto ma_source = series_method_cast<DataMethod>(bb_method->ma_source());
  EXPECT_NE(ma_source, nullptr);
  EXPECT_EQ(bb_method->ma_type(), BbMaType::Sma);
  EXPECT_EQ(bb_method->period(), 20);
  EXPECT_EQ(bb_method->stddev(), 2.0);

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerMacdMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "MACD",
      "fast": 12,
      "slow": 26,
      "signal": 9,
      "source": {
        "method": "DATA",
        "field": "close"
      }
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto macd_method =
   series_method_cast<MacdMethod<AnySeriesMethod>>(method);
  ASSERT_NE(macd_method, nullptr);

  const auto source = series_method_cast<DataMethod>(macd_method->source());
  EXPECT_NE(source, nullptr);
  EXPECT_EQ(macd_method->fast_period(), 12);
  EXPECT_EQ(macd_method->slow_period(), 26);
  EXPECT_EQ(macd_method->signal_period(), 9);

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerStochMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "STOCH",
      "kPeriod": 5,
      "kSmooth": 3,
      "dPeriod": 3
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto stoch_method = series_method_cast<StochMethod>(method);
  ASSERT_NE(stoch_method, nullptr);

  EXPECT_EQ(stoch_method->k_period(), 5);
  EXPECT_EQ(stoch_method->k_smooth(), 3);
  EXPECT_EQ(stoch_method->d_period(), 3);

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerStochRsiMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "STOCH_RSI",
      "rsiPeriod": 14,
      "kPeriod": 5,
      "kSmooth": 3,
      "dPeriod": 3,
      "rsiSource": {
        "method": "DATA",
        "field": "close"
      }
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto stoch_rsi_method =
   series_method_cast<StochRsiMethod<AnySeriesMethod>>(method);
  ASSERT_NE(stoch_rsi_method, nullptr);

  const auto rsi_source =
   series_method_cast<DataMethod>(stoch_rsi_method->rsi_source());
  EXPECT_NE(rsi_source, nullptr);
  EXPECT_EQ(stoch_rsi_method->rsi_period(), 14);
  EXPECT_EQ(stoch_rsi_method->k_period(), 5);
  EXPECT_EQ(stoch_rsi_method->k_smooth(), 3);
  EXPECT_EQ(stoch_rsi_method->d_period(), 3);

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerKcMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "KC",
      "ma": {
        "method": "SMA",
        "period": 5,
        "source": {
          "method": "DATA",
          "field": "close"
        }
      },
      "range": {
        "method": "ATR",
        "period": 14
      },
      "multiplier": 1.0
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto kc_method =
   series_method_cast<KcMethod<AnySeriesMethod, AnySeriesMethod>>(method);
  ASSERT_NE(kc_method, nullptr);

  const auto ma_method =
   series_method_cast<SmaMethod<AnySeriesMethod>>(kc_method->ma());
  const auto range_method = series_method_cast<AtrMethod>(kc_method->range());
  EXPECT_NE(ma_method, nullptr);
  EXPECT_NE(range_method, nullptr);
  EXPECT_EQ(kc_method->multiplier(), 1.0);

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerAddMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "ADD",
      "augend": 50,
      "addend": {
        "method": "VALUE",
        "value": 25
      }
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto add_method =
   series_method_cast<AddMethod<AnySeriesMethod, AnySeriesMethod>>(method);
  ASSERT_NE(add_method, nullptr);

  const auto augend = series_method_cast<ValueMethod>(add_method->augend());
  const auto addend = series_method_cast<ValueMethod>(add_method->addend());
  ASSERT_NE(augend, nullptr);
  ASSERT_NE(addend, nullptr);
  EXPECT_EQ(augend->value(), 50);
  EXPECT_EQ(addend->value(), 25);

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerSubtractMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "SUBTRACT",
      "minuend": {
        "method": "VALUE",
        "value": 100
      },
      "subtrahend": {
        "method": "VALUE",
        "value": 30
      }
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto subtract_method =
   series_method_cast<SubtractMethod<AnySeriesMethod, AnySeriesMethod>>(method);
  ASSERT_NE(subtract_method, nullptr);

  const auto minuend =
   series_method_cast<ValueMethod>(subtract_method->minuend());
  const auto subtrahend =
   series_method_cast<ValueMethod>(subtract_method->subtrahend());
  ASSERT_NE(minuend, nullptr);
  ASSERT_NE(subtrahend, nullptr);
  EXPECT_EQ(minuend->value(), 100);
  EXPECT_EQ(subtrahend->value(), 30);

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerMultiplyMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "MULTIPLY",
      "multiplicand": {
        "method": "VALUE",
        "value": 10
      },
      "multiplier": {
        "method": "VALUE",
        "value": 5
      }
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto multiply_method =
   series_method_cast<MultiplyMethod<AnySeriesMethod, AnySeriesMethod>>(method);
  ASSERT_NE(multiply_method, nullptr);

  const auto multiplicand =
   series_method_cast<ValueMethod>(multiply_method->multiplicand());
  const auto multiplier =
   series_method_cast<ValueMethod>(multiply_method->multiplier());
  ASSERT_NE(multiplicand, nullptr);
  ASSERT_NE(multiplier, nullptr);
  EXPECT_EQ(multiplicand->value(), 10);
  EXPECT_EQ(multiplier->value(), 5);

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerDivideMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "DIVIDE",
      "dividend": {
        "method": "VALUE",
        "value": 100
      },
      "divisor": {
        "method": "VALUE",
        "value": 2
      }
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto divide_method =
   series_method_cast<DivideMethod<AnySeriesMethod, AnySeriesMethod>>(method);
  ASSERT_NE(divide_method, nullptr);

  const auto dividend =
   series_method_cast<ValueMethod>(divide_method->dividend());
  const auto divisor =
   series_method_cast<ValueMethod>(divide_method->divisor());
  ASSERT_NE(dividend, nullptr);
  ASSERT_NE(divisor, nullptr);
  EXPECT_EQ(dividend->value(), 100);
  EXPECT_EQ(divisor->value(), 2);

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerNegateMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "NEGATE",
      "operand": {
        "method": "VALUE",
        "value": 42
      }
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto negate_method =
   series_method_cast<NegateMethod<AnySeriesMethod>>(method);
  ASSERT_NE(negate_method, nullptr);

  const auto operand =
   series_method_cast<ValueMethod>(negate_method->operand());
  ASSERT_NE(operand, nullptr);
  EXPECT_EQ(operand->value(), 42);

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerChangeMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "CHANGE",
      "source": {
        "method": "DATA",
        "field": "close"
      }
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto changes_method =
   series_method_cast<ChangeMethod<AnySeriesMethod>>(method);
  ASSERT_NE(changes_method, nullptr);

  const auto source = series_method_cast<DataMethod>(changes_method->source());
  ASSERT_NE(source, nullptr);
  EXPECT_EQ(source->field(), "close");

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerAbsDiffMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "ABS_DIFF",
      "minuend": {
        "method": "DATA",
        "field": "high"
      },
      "subtrahend": {
        "method": "DATA",
        "field": "low"
      }
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto abs_diff_method =
   series_method_cast<AbsDiffMethod<AnySeriesMethod, AnySeriesMethod>>(method);
  ASSERT_NE(abs_diff_method, nullptr);

  const auto minuend =
   series_method_cast<DataMethod>(abs_diff_method->minuend());
  const auto subtrahend =
   series_method_cast<DataMethod>(abs_diff_method->subtrahend());
  ASSERT_NE(minuend, nullptr);
  ASSERT_NE(subtrahend, nullptr);
  EXPECT_EQ(minuend->field(), "high");
  EXPECT_EQ(subtrahend->field(), "low");

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerPercentageMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "PERCENTAGE",
      "base": 100,
      "percent": 20
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto percentage_method =
   series_method_cast<PercentageMethod<AnySeriesMethod>>(method);
  ASSERT_NE(percentage_method, nullptr);

  const auto base = series_method_cast<ValueMethod>(percentage_method->base());
  ASSERT_NE(base, nullptr);
  EXPECT_EQ(base->value(), 100);

  const auto percent = percentage_method->percent();
  EXPECT_EQ(percent, 20);

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerInvalidMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "INVALID"
    }
  )");

  EXPECT_THROW(config_parser.parse_method(config), std::invalid_argument);
}

TEST_F(ConfigParserTest, ParseScreenerAllOfMethod)
{
  const auto config = json::parse(R"(
    {
      "filter": "ALL_OF",
      "conditions": [
        {
          "filter": "GREATER_THAN",
          "threshold": {
            "method": "VALUE",
            "value": 100
          },
          "target": {
            "method": "DATA",
            "field": "close"
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
            "field": "close"
          }
        }
      ]
    }
  )");

  const auto filter = config_parser.parse_filter(config);

  const auto all_of_filter = condition_method_cast<AllOfMethod>(filter);
  ASSERT_NE(all_of_filter, nullptr);

  const auto filters = all_of_filter->conditions();
  ASSERT_EQ(filters.size(), 2);

  const auto greater_than_filter =
   condition_method_cast<GreaterThanMethod>(filters[0]);
  const auto less_than_filter =
   condition_method_cast<LessThanMethod>(filters[1]);

  EXPECT_NE(greater_than_filter, nullptr);
  EXPECT_NE(less_than_filter, nullptr);

  const auto serialized_config = config_parser.serialize_filter(filter);
  const auto deserialized_filter =
   config_parser.parse_filter(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerAnyOfMethod)
{
  const auto config = json::parse(R"(
    {
      "filter": "ANY_OF",
      "conditions": [
        {
          "filter": "GREATER_THAN",
          "threshold": {
            "method": "VALUE",
            "value": 100
          },
          "target": {
            "method": "DATA",
            "field": "close"
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
            "field": "close"
          }
        }
      ]
    }
  )");

  const auto filter = config_parser.parse_filter(config);

  const auto any_of_filter = condition_method_cast<AnyOfMethod>(filter);
  ASSERT_NE(any_of_filter, nullptr);

  const auto conditions = any_of_filter->conditions();
  ASSERT_EQ(conditions.size(), 2);

  const auto greater_than_filter =
   condition_method_cast<GreaterThanMethod>(conditions[0]);
  const auto less_than_filter =
   condition_method_cast<LessThanMethod>(conditions[1]);

  EXPECT_NE(greater_than_filter, nullptr);
  EXPECT_NE(less_than_filter, nullptr);

  const auto serialized_config = config_parser.serialize_filter(filter);
  const auto deserialized_filter =
   config_parser.parse_filter(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerGreaterThanMethod)
{
  const auto config = json::parse(R"(
    {
      "filter": "GREATER_THAN",
      "threshold": 100,
      "target": {
        "method": "DATA",
        "field": "close"
      }
    }
  )");

  const auto filter = config_parser.parse_filter(config);

  const auto greater_than_filter =
   condition_method_cast<GreaterThanMethod>(filter);
  ASSERT_NE(greater_than_filter, nullptr);

  const auto target =
   series_method_cast<DataMethod>(greater_than_filter->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "close");

  const auto threshold =
   series_method_cast<ValueMethod>(greater_than_filter->threshold());
  ASSERT_NE(threshold, nullptr);

  EXPECT_EQ(threshold->value(), 100);

  const auto serialized_config = config_parser.serialize_filter(filter);
  const auto deserialized_filter =
   config_parser.parse_filter(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerGreaterEqualMethod)
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
        "field": "close"
      }
    }
  )");

  const auto filter = config_parser.parse_filter(config);

  const auto greater_equal_filter =
   condition_method_cast<GreaterEqualMethod>(filter);
  ASSERT_NE(greater_equal_filter, nullptr);

  const auto target =
   series_method_cast<DataMethod>(greater_equal_filter->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "close");

  const auto serialized_config = config_parser.serialize_filter(filter);
  const auto deserialized_filter =
   config_parser.parse_filter(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerLessThanMethod)
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
        "field": "close"
      }
    }
  )");

  const auto filter = config_parser.parse_filter(config);

  const auto less_than_filter = condition_method_cast<LessThanMethod>(filter);
  ASSERT_NE(less_than_filter, nullptr);

  const auto target =
   series_method_cast<DataMethod>(less_than_filter->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "close");

  const auto serialized_config = config_parser.serialize_filter(filter);
  const auto deserialized_filter =
   config_parser.parse_filter(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerLessEqualMethod)
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
        "field": "close"
      }
    }
  )");

  const auto filter = config_parser.parse_filter(config);

  const auto less_equal_filter = condition_method_cast<LessEqualMethod>(filter);
  ASSERT_NE(less_equal_filter, nullptr);

  const auto target =
   series_method_cast<DataMethod>(less_equal_filter->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "close");

  const auto serialized_config = config_parser.serialize_filter(filter);
  const auto deserialized_filter =
   config_parser.parse_filter(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerEqualMethod)
{
  const auto config = json::parse(R"(
    {
      "filter": "EQUAL",
      "threshold": {
        "method": "VALUE",
        "value": 100
      },
      "target": {
        "method": "DATA",
        "field": "close"
      }
    }
  )");

  const auto filter = config_parser.parse_filter(config);

  const auto equal_filter = condition_method_cast<EqualMethod>(filter);
  ASSERT_NE(equal_filter, nullptr);

  const auto target = series_method_cast<DataMethod>(equal_filter->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "close");

  const auto threshold =
   series_method_cast<ValueMethod>(equal_filter->threshold());
  ASSERT_NE(threshold, nullptr);

  EXPECT_EQ(threshold->value(), 100);

  const auto serialized_config = config_parser.serialize_filter(filter);
  const auto deserialized_filter =
   config_parser.parse_filter(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerNotEqualMethod)
{
  const auto config = json::parse(R"(
    {
      "filter": "NOT_EQUAL",
      "threshold": {
        "method": "VALUE",
        "value": 100
      },
      "target": {
        "method": "DATA",
        "field": "close"
      }
    }
  )");

  const auto filter = config_parser.parse_filter(config);

  const auto not_equal_filter = condition_method_cast<NotEqualMethod>(filter);
  ASSERT_NE(not_equal_filter, nullptr);

  const auto target =
   series_method_cast<DataMethod>(not_equal_filter->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "close");

  const auto threshold =
   series_method_cast<ValueMethod>(not_equal_filter->threshold());
  ASSERT_NE(threshold, nullptr);

  EXPECT_EQ(threshold->value(), 100);

  const auto serialized_config = config_parser.serialize_filter(filter);
  const auto deserialized_filter =
   config_parser.parse_filter(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerCrossunderMethod)
{
  const auto config = json::parse(R"(
    {
      "filter": "CROSSUNDER",
      "signal": {
        "method": "DATA",
        "field": "close"
      },
      "reference": 100
    }
  )");

  const auto filter = config_parser.parse_filter(config);

  const auto crossunder_filter =
   condition_method_cast<CrossunderMethod>(filter);
  ASSERT_NE(crossunder_filter, nullptr);

  const auto signal =
   series_method_cast<DataMethod>(crossunder_filter->signal());
  ASSERT_NE(signal, nullptr);

  EXPECT_EQ(signal->field(), "close");

  const auto reference =
   series_method_cast<ValueMethod>(crossunder_filter->reference());
  ASSERT_NE(reference, nullptr);

  EXPECT_EQ(reference->value(), 100);

  const auto serialized_config = config_parser.serialize_filter(filter);
  const auto deserialized_filter =
   config_parser.parse_filter(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerCrossoverMethod)
{
  const auto config = json::parse(R"(
    {
      "filter": "CROSSOVER",
      "signal": {
        "method": "DATA",
        "field": "close"
      },
      "reference": {
        "method": "VALUE",
        "value": 100
      }
    }
  )");

  const auto filter = config_parser.parse_filter(config);

  const auto crossover_filter = condition_method_cast<CrossoverMethod>(filter);
  ASSERT_NE(crossover_filter, nullptr);

  const auto signal =
   series_method_cast<DataMethod>(crossover_filter->signal());
  ASSERT_NE(signal, nullptr);

  EXPECT_EQ(signal->field(), "close");

  const auto reference =
   series_method_cast<ValueMethod>(crossover_filter->reference());
  ASSERT_NE(reference, nullptr);

  EXPECT_EQ(reference->value(), 100);

  const auto serialized_config = config_parser.serialize_filter(filter);
  const auto deserialized_filter =
   config_parser.parse_filter(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerTrueMethod)
{
  const auto config = json::parse(R"(
    {
      "filter": "TRUE"
    }
  )");

  const auto filter = config_parser.parse_filter(config);

  const auto true_filter = condition_method_cast<TrueMethod>(filter);
  ASSERT_NE(true_filter, nullptr);

  const auto serialized_config = config_parser.serialize_filter(filter);
  const auto deserialized_filter =
   config_parser.parse_filter(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerFalseMethod)
{
  const auto config = json::parse(R"(
    {
      "filter": "FALSE"
    }
  )");

  const auto filter = config_parser.parse_filter(config);

  const auto false_filter = condition_method_cast<FalseMethod>(filter);
  ASSERT_NE(false_filter, nullptr);

  const auto serialized_config = config_parser.serialize_filter(filter);
  const auto deserialized_filter =
   config_parser.parse_filter(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerAndMethod)
{
  const auto config = json::parse(R"(
    {
      "filter": "AND",
      "firstCondition":
        {
          "filter": "TRUE"
        },

      "secondCondition":
        {
          "filter": "FALSE"
        }
    }
  )");

  const auto filter = config_parser.parse_filter(config);

  const auto and_filter = condition_method_cast<AndMethod>(filter);
  ASSERT_NE(and_filter, nullptr);

  const auto first_condition = and_filter->first_condition();
  const auto second_condition = and_filter->second_condition();

  const auto true_filter = condition_method_cast<TrueMethod>(first_condition);
  const auto false_filter =
   condition_method_cast<FalseMethod>(second_condition);

  ASSERT_NE(true_filter, nullptr);
  ASSERT_NE(false_filter, nullptr);

  const auto serialized_config = config_parser.serialize_filter(filter);
  const auto deserialized_filter =
   config_parser.parse_filter(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerOrMethod)
{
  const auto config = json::parse(R"(
    {
      "filter": "OR",
      "firstCondition":
        {
          "filter": "TRUE"
        },

      "secondCondition":
        {
          "filter": "FALSE"
        }
    }
  )");

  const auto filter = config_parser.parse_filter(config);

  const auto or_filter = condition_method_cast<OrMethod>(filter);
  ASSERT_NE(or_filter, nullptr);

  const auto first_condition = or_filter->first_condition();
  const auto second_condition = or_filter->second_condition();

  const auto true_filter = condition_method_cast<TrueMethod>(first_condition);
  const auto false_filter =
   condition_method_cast<FalseMethod>(second_condition);

  ASSERT_NE(true_filter, nullptr);
  ASSERT_NE(false_filter, nullptr);

  const auto serialized_config = config_parser.serialize_filter(filter);
  const auto deserialized_filter =
   config_parser.parse_filter(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerNotMethod)
{
  const auto config = json::parse(R"(
    {
      "filter": "NOT",
      "condition": true
    }
  )");

  const auto filter = config_parser.parse_filter(config);

  const auto not_filter = condition_method_cast<NotMethod>(filter);
  ASSERT_NE(not_filter, nullptr);

  const auto other_condition = not_filter->other_condition();
  const auto true_filter = condition_method_cast<TrueMethod>(other_condition);

  ASSERT_NE(true_filter, nullptr);

  const auto serialized_config = config_parser.serialize_filter(filter);
  const auto deserialized_filter =
   config_parser.parse_filter(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerXorMethod)
{
  const auto config = json::parse(R"(
    {
      "filter": "XOR",
      "firstCondition":
        {
          "filter": "TRUE"
        },

      "secondCondition":
        {
          "filter": "FALSE"
        }
    }
  )");

  const auto filter = config_parser.parse_filter(config);

  const auto xor_filter = condition_method_cast<XorMethod>(filter);
  ASSERT_NE(xor_filter, nullptr);

  const auto first_condition = xor_filter->first_condition();
  const auto second_condition = xor_filter->second_condition();

  const auto true_filter = condition_method_cast<TrueMethod>(first_condition);
  const auto false_filter =
   condition_method_cast<FalseMethod>(second_condition);

  ASSERT_NE(true_filter, nullptr);
  ASSERT_NE(false_filter, nullptr);

  const auto serialized_config = config_parser.serialize_filter(filter);
  const auto deserialized_filter =
   config_parser.parse_filter(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseAnyConditionMethodIsInvalid)
{
  const auto config = json::parse(R"(
    {
      "filter": "invalid not existed filter should throw exception"
    }
  )");

  EXPECT_THROW(config_parser.parse_filter(config), std::exception);
}

TEST_F(ConfigParserTest, ParseAnyConditionMethodWithInvalidRequiredFields)
{
  const auto config = json::parse(R"(
    {
      "filter": "AND"
    }
  )");

  EXPECT_THROW(config_parser.parse_filter(config), std::exception);
}
