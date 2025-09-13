#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <pludux/screener.hpp>

#include <pludux/config_parser.hpp>

using namespace pludux;
using namespace pludux::screener;
using json = nlohmann::json;

class ConfigParserTest : public ::testing::Test {
protected:
  ConfigParser config_parser;
  void SetUp() override
  {
    config_parser.register_default_parsers();

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

TEST_F(ConfigParserTest, ParseScreenerOpenMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "OPEN"
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto open_method = screener_method_cast<OpenMethod>(method);
  ASSERT_NE(open_method, nullptr);

  EXPECT_EQ(open_method->offset(), 0);

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerHighMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "HIGH",
      "offset": 1
  }
  )");

  const auto method = config_parser.parse_method(config);

  const auto high_method = screener_method_cast<HighMethod>(method);
  ASSERT_NE(high_method, nullptr);

  EXPECT_EQ(high_method->offset(), 1);

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerLowMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "LOW",
      "offset": 2
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto low_method = screener_method_cast<LowMethod>(method);
  ASSERT_NE(low_method, nullptr);

  EXPECT_EQ(low_method->offset(), 2);

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerCloseMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "CLOSE",
      "offset": 3
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto close_method = screener_method_cast<CloseMethod>(method);
  ASSERT_NE(close_method, nullptr);

  EXPECT_EQ(close_method->offset(), 3);

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerVolumeMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "VOLUME",
      "offset": 4
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto volume_method = screener_method_cast<VolumeMethod>(method);
  ASSERT_NE(volume_method, nullptr);

  EXPECT_EQ(volume_method->offset(), 4);

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
      "offset": 0,
      "input": {
        "method": "DATA",
        "field": "close",
        "offset": 0
      }
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto sma_method = screener_method_cast<SmaMethod>(method);
  ASSERT_NE(sma_method, nullptr);

  EXPECT_EQ(sma_method->period(), 14);
  EXPECT_EQ(sma_method->offset(), 0);

  const auto input = screener_method_cast<DataMethod>(sma_method->input());
  ASSERT_NE(input, nullptr);

  EXPECT_EQ(input->field(), "close");
  EXPECT_EQ(input->offset(), 0);

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
      "offset": 1,
      "input": {
        "method": "DATA",
        "field": "open",
        "offset": 1
      }
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto ema_method = screener_method_cast<EmaMethod>(method);
  ASSERT_NE(ema_method, nullptr);

  EXPECT_EQ(ema_method->period(), 10);
  EXPECT_EQ(ema_method->offset(), 1);

  const auto input = screener_method_cast<DataMethod>(ema_method->input());
  ASSERT_NE(input, nullptr);

  EXPECT_EQ(input->field(), "open");
  EXPECT_EQ(input->offset(), 1);

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
        "offset": 2,
        "input": {
          "method": "DATA",
          "field": "high",
          "offset": 2
        }
      }
    )");

  const auto method = config_parser.parse_method(config);

  const auto wma_method = screener_method_cast<WmaMethod>(method);
  ASSERT_NE(wma_method, nullptr);

  EXPECT_EQ(wma_method->period(), 20);
  EXPECT_EQ(wma_method->offset(), 2);

  const auto input = screener_method_cast<DataMethod>(wma_method->input());
  ASSERT_NE(input, nullptr);

  EXPECT_EQ(input->field(), "high");
  EXPECT_EQ(input->offset(), 2);

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
        "offset": 3,
        "input": {
          "method": "DATA",
          "field": "low",
          "offset": 3
        }
      }
    )");

  const auto method = config_parser.parse_method(config);

  const auto rma_method = screener_method_cast<RmaMethod>(method);
  ASSERT_NE(rma_method, nullptr);

  EXPECT_EQ(rma_method->period(), 15);
  EXPECT_EQ(rma_method->offset(), 3);

  const auto input = screener_method_cast<DataMethod>(rma_method->input());
  ASSERT_NE(input, nullptr);

  EXPECT_EQ(input->field(), "low");
  EXPECT_EQ(input->offset(), 3);

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
        "offset": 4,
        "input": {
          "method": "DATA",
          "field": "volume",
          "offset": 4
        }
      }
    )");

  const auto method = config_parser.parse_method(config);

  const auto hma_method = screener_method_cast<HmaMethod>(method);
  ASSERT_NE(hma_method, nullptr);

  EXPECT_EQ(hma_method->period(), 25);
  EXPECT_EQ(hma_method->offset(), 4);

  const auto input = screener_method_cast<DataMethod>(hma_method->input());
  ASSERT_NE(input, nullptr);

  EXPECT_EQ(input->field(), "volume");
  EXPECT_EQ(input->offset(), 4);

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
      "offset": 0,
      "input": {
        "method": "DATA",
        "field": "close",
        "offset": 0
      }
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto rsi_method = screener_method_cast<RsiMethod>(method);
  ASSERT_NE(rsi_method, nullptr);

  EXPECT_EQ(rsi_method->period(), 14);
  EXPECT_EQ(rsi_method->offset(), 0);

  const auto input = screener_method_cast<DataMethod>(rsi_method->input());
  ASSERT_NE(input, nullptr);

  EXPECT_EQ(input->field(), "close");
  EXPECT_EQ(input->offset(), 0);

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

  const auto value_method = screener_method_cast<ValueMethod>(method);
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
      "field": "open",
      "offset": 0
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto field_method = screener_method_cast<DataMethod>(method);
  ASSERT_NE(field_method, nullptr);

  EXPECT_EQ(field_method->field(), "open");
  EXPECT_EQ(field_method->offset(), 0);

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
      "multiplier": 1.0,
      "offset": 0,
      "high": {
        "method": "DATA",
        "field": "high"
      },
      "low": {
        "method": "DATA",
        "field": "low"
      },
      "close": {
        "method": "DATA",
        "field": "close"
      }
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto atr_method = screener_method_cast<AtrMethod>(method);
  ASSERT_NE(atr_method, nullptr);

  const auto high = screener_method_cast<DataMethod>(atr_method->high());
  const auto low = screener_method_cast<DataMethod>(atr_method->low());
  const auto close = screener_method_cast<DataMethod>(atr_method->close());
  EXPECT_NE(high, nullptr);
  EXPECT_NE(low, nullptr);
  EXPECT_NE(close, nullptr);
  EXPECT_EQ(atr_method->period(), 14);
  EXPECT_EQ(atr_method->multiplier(), 1.0);
  EXPECT_EQ(atr_method->offset(), 0);

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
      "output": "upper",
      "maType": "SMA",
      "period": 20,
      "stddev": 2.0,
      "offset": 0,
      "input": {
        "method": "DATA",
        "field": "close",
        "offset": 0
      }
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto bb_method = screener_method_cast<BbMethod>(method);
  ASSERT_NE(bb_method, nullptr);

  const auto input = screener_method_cast<DataMethod>(bb_method->input());
  EXPECT_NE(input, nullptr);
  EXPECT_EQ(bb_method->output(), OutputName::UpperBand);
  EXPECT_EQ(bb_method->ma_type(), BbMethod::MaType::sma);
  EXPECT_EQ(bb_method->period(), 20);
  EXPECT_EQ(bb_method->stddev(), 2.0);
  EXPECT_EQ(bb_method->offset(), 0);

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
      "output": "macd",
      "fast": 12,
      "slow": 26,
      "signal": 9,
      "offset": 0,
      "input": {
        "method": "DATA",
        "field": "close",
        "offset": 0
      }
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto macd_method = screener_method_cast<MacdMethod>(method);
  ASSERT_NE(macd_method, nullptr);

  const auto input = screener_method_cast<DataMethod>(macd_method->input());
  EXPECT_NE(input, nullptr);
  EXPECT_EQ(macd_method->output(), OutputName::MacdLine);
  EXPECT_EQ(macd_method->fast_period(), 12);
  EXPECT_EQ(macd_method->slow_period(), 26);
  EXPECT_EQ(macd_method->signal_period(), 9);
  EXPECT_EQ(macd_method->offset(), 0);

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
      "output": "k",
      "kPeriod": 5,
      "kSmooth": 3,
      "dPeriod": 3,
      "offset": 0,
      "high": {
        "method": "DATA",
        "field": "high",
        "offset": 0
      },
      "low": {
        "method": "DATA",
        "field": "low",
        "offset": 0
      },
      "close": {
        "method": "DATA",
        "field": "close",
        "offset": 0
      }
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto stoch_method = screener_method_cast<StochMethod>(method);
  ASSERT_NE(stoch_method, nullptr);

  const auto high = screener_method_cast<DataMethod>(stoch_method->high());
  const auto low = screener_method_cast<DataMethod>(stoch_method->low());
  const auto close = screener_method_cast<DataMethod>(stoch_method->close());
  EXPECT_NE(high, nullptr);
  EXPECT_NE(low, nullptr);
  EXPECT_NE(close, nullptr);
  EXPECT_EQ(stoch_method->output(), OutputName::StochasticK);
  EXPECT_EQ(stoch_method->k_period(), 5);
  EXPECT_EQ(stoch_method->k_smooth(), 3);
  EXPECT_EQ(stoch_method->d_period(), 3);
  EXPECT_EQ(stoch_method->offset(), 0);

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
      "output": "k",
      "rsiPeriod": 14,
      "kPeriod": 5,
      "kSmooth": 3,
      "dPeriod": 3,
      "offset": 0,
      "rsiInput": {
        "method": "DATA",
        "field": "close",
        "offset": 0
      }
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto stoch_rsi_method = screener_method_cast<StochRsiMethod>(method);
  ASSERT_NE(stoch_rsi_method, nullptr);

  const auto rsi_input =
   screener_method_cast<DataMethod>(stoch_rsi_method->rsi_input());
  EXPECT_NE(rsi_input, nullptr);
  EXPECT_EQ(stoch_rsi_method->output(), OutputName::StochasticK);
  EXPECT_EQ(stoch_rsi_method->rsi_period(), 14);
  EXPECT_EQ(stoch_rsi_method->k_period(), 5);
  EXPECT_EQ(stoch_rsi_method->k_smooth(), 3);
  EXPECT_EQ(stoch_rsi_method->d_period(), 3);
  EXPECT_EQ(stoch_rsi_method->offset(), 0);

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
      "output": "middle",
      "ma": {
        "method": "SMA",
        "period": 5,
        "offset": 0,
        "input": {
          "method": "DATA",
          "field": "close",
          "offset": 0
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

  const auto kc_method = screener_method_cast<KcMethod>(method);
  ASSERT_NE(kc_method, nullptr);

  const auto ma_method = screener_method_cast<SmaMethod>(kc_method->ma());
  const auto range_method = screener_method_cast<AtrMethod>(kc_method->range());
  EXPECT_NE(ma_method, nullptr);
  EXPECT_NE(range_method, nullptr);
  EXPECT_EQ(kc_method->offset(), 0);
  EXPECT_EQ(kc_method->output(), OutputName::MiddleBand);
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

  const auto add_method = screener_method_cast<AddMethod>(method);
  ASSERT_NE(add_method, nullptr);
  EXPECT_EQ(add_method->offset(), 0);

  const auto augend = screener_method_cast<ValueMethod>(add_method->augend());
  const auto addend = screener_method_cast<ValueMethod>(add_method->addend());
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

  const auto subtract_method = screener_method_cast<SubtractMethod>(method);
  ASSERT_NE(subtract_method, nullptr);

  const auto minuend =
   screener_method_cast<ValueMethod>(subtract_method->minuend());
  const auto subtrahend =
   screener_method_cast<ValueMethod>(subtract_method->subtrahend());
  ASSERT_NE(minuend, nullptr);
  ASSERT_NE(subtrahend, nullptr);
  EXPECT_EQ(minuend->value(), 100);
  EXPECT_EQ(subtrahend->value(), 30);
  EXPECT_EQ(subtract_method->offset(), 0);

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

  const auto multiply_method = screener_method_cast<MultiplyMethod>(method);
  ASSERT_NE(multiply_method, nullptr);

  const auto multiplicand =
   screener_method_cast<ValueMethod>(multiply_method->multiplicand());
  const auto multiplier =
   screener_method_cast<ValueMethod>(multiply_method->multiplier());
  ASSERT_NE(multiplicand, nullptr);
  ASSERT_NE(multiplier, nullptr);
  EXPECT_EQ(multiplicand->value(), 10);
  EXPECT_EQ(multiplier->value(), 5);
  EXPECT_EQ(multiply_method->offset(), 0);

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

  const auto divide_method = screener_method_cast<DivideMethod>(method);
  ASSERT_NE(divide_method, nullptr);

  const auto dividend =
   screener_method_cast<ValueMethod>(divide_method->dividend());
  const auto divisor =
   screener_method_cast<ValueMethod>(divide_method->divisor());
  ASSERT_NE(dividend, nullptr);
  ASSERT_NE(divisor, nullptr);
  EXPECT_EQ(dividend->value(), 100);
  EXPECT_EQ(divisor->value(), 2);
  EXPECT_EQ(divide_method->offset(), 0);

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

  const auto negate_method = screener_method_cast<NegateMethod>(method);
  ASSERT_NE(negate_method, nullptr);

  const auto operand =
   screener_method_cast<ValueMethod>(negate_method->operand());
  ASSERT_NE(operand, nullptr);
  EXPECT_EQ(operand->value(), 42);
  EXPECT_EQ(negate_method->offset(), 0);

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerChangesMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "CHANGES",
      "input": {
        "method": "DATA",
        "field": "close",
        "offset": 0
      }
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto changes_method = screener_method_cast<ChangesMethod>(method);
  ASSERT_NE(changes_method, nullptr);
  EXPECT_EQ(changes_method->offset(), 0);

  const auto input = screener_method_cast<DataMethod>(changes_method->input());
  ASSERT_NE(input, nullptr);
  EXPECT_EQ(input->field(), "close");
  EXPECT_EQ(input->offset(), 0);

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
        "field": "high",
        "offset": 0
      },
      "subtrahend": {
        "method": "DATA",
        "field": "low",
        "offset": 0
      }
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto abs_diff_method = screener_method_cast<AbsDiffMethod>(method);
  ASSERT_NE(abs_diff_method, nullptr);
  EXPECT_EQ(abs_diff_method->offset(), 0);

  const auto minuend =
   screener_method_cast<DataMethod>(abs_diff_method->minuend());
  const auto subtrahend =
   screener_method_cast<DataMethod>(abs_diff_method->subtrahend());
  ASSERT_NE(minuend, nullptr);
  ASSERT_NE(subtrahend, nullptr);
  EXPECT_EQ(minuend->field(), "high");
  EXPECT_EQ(minuend->offset(), 0);
  EXPECT_EQ(subtrahend->field(), "low");
  EXPECT_EQ(subtrahend->offset(), 0);

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
      "total": 100,
      "percent": 20
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto percentage_method = screener_method_cast<PercentageMethod>(method);
  ASSERT_NE(percentage_method, nullptr);
  EXPECT_EQ(percentage_method->offset(), 0);

  const auto total =
   screener_method_cast<ValueMethod>(percentage_method->total());
  const auto percent =
   screener_method_cast<ValueMethod>(percentage_method->percent());
  ASSERT_NE(total, nullptr);
  ASSERT_NE(percent, nullptr);

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerBullishDivergenceMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "BULLISH_DIVERGENCE",
      "signal": {
        "method": "RSI",
        "period": 14,
        "input": {
          "method": "DATA",
          "field": "close",
          "offset": 0
        }
      },
      "reference": {
        "method": "DATA",
        "field": "low",
        "offset": 0
      },
      "pivotRange": 10,
      "lookbackRange": 20
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto bullish_divergence_method =
   screener_method_cast<BullishDivergenceMethod>(method);
  ASSERT_NE(bullish_divergence_method, nullptr);
  EXPECT_EQ(bullish_divergence_method->pivot_range(), 10);
  EXPECT_EQ(bullish_divergence_method->lookback_range(), 20);

  const auto signal_method =
   screener_method_cast<RsiMethod>(bullish_divergence_method->signal());
  const auto reference_method =
   screener_method_cast<DataMethod>(bullish_divergence_method->reference());
  ASSERT_NE(signal_method, nullptr);
  ASSERT_NE(reference_method, nullptr);

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerHiddenBullishDivergenceMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "HIDDEN_BULLISH_DIVERGENCE",
      "signal": {
        "method": "RSI",
        "period": 14,
        "input": {
          "method": "DATA",
          "field": "close",
          "offset": 0
        }
      },
      "reference": {
        "method": "DATA",
        "field": "low",
        "offset": 0
      },
      "pivotRange": 10,
      "lookbackRange": 20
    }
  )");

  const auto method = config_parser.parse_method(config);

  const auto hidden_bullish_divergence_method =
   screener_method_cast<HiddenBullishDivergenceMethod>(method);
  ASSERT_NE(hidden_bullish_divergence_method, nullptr);
  EXPECT_EQ(hidden_bullish_divergence_method->pivot_range(), 10);
  EXPECT_EQ(hidden_bullish_divergence_method->lookback_range(), 20);

  const auto signal_method =
   screener_method_cast<RsiMethod>(hidden_bullish_divergence_method->signal());
  const auto reference_method = screener_method_cast<DataMethod>(
   hidden_bullish_divergence_method->reference());
  ASSERT_NE(signal_method, nullptr);
  ASSERT_NE(reference_method, nullptr);

  const auto serialized_config = config_parser.serialize_method(method);
  const auto deserialized_config =
   config_parser.parse_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenMethodWithExtends)
{
  const auto base_config = json::parse(R"(
    {
      "method": "SMA",
      "name": "base",
      "period": 14,
      "offset": 1,
      "input": {
        "method": "DATA",
        "field": "close",
        "offset": 0
      }
    }
  )");

  const auto config = json::parse(R"(
    {
      "method": "SMA",
      "extends": "base",
      "period": 20
    }
  )");

  auto config_parser = ConfigParser{};
  config_parser.register_default_parsers();

  const auto base_method = config_parser.parse_method(base_config);
  const auto method = config_parser.parse_method(config);

  const auto sma_method = screener_method_cast<SmaMethod>(method);
  ASSERT_NE(sma_method, nullptr);

  EXPECT_EQ(sma_method->offset(), 1);
  EXPECT_EQ(sma_method->period(), 20);
}

TEST_F(ConfigParserTest, ParseScreenMethodNamedMethod)
{
  const auto named_method_config = json::parse(R"(
    {
      "method": "DATA",
      "name": "foo",
      "field": "close"
    }
  )");

  const auto config = json::parse(R"(
    {
      "method": "RSI",
      "period": 14,
      "input": "foo"
    }
  )");

  auto config_parser = ConfigParser{};
  config_parser.register_default_parsers();

  const auto named_method = config_parser.parse_method(named_method_config);
  const auto method = config_parser.parse_method(config);

  const auto rsi_method = screener_method_cast<RsiMethod>(method);
  ASSERT_NE(rsi_method, nullptr);

  const auto input = rsi_method->input();
  const auto data_method = screener_method_cast<DataMethod>(input);
  ASSERT_NE(data_method, nullptr);

  EXPECT_EQ(data_method->field(), "close");
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

TEST_F(ConfigParserTest, ParseScreenerAllOfFilter)
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

  const auto filter = config_parser.parse_filter(config);

  const auto all_of_filter = screener_filter_cast<AllOfFilter>(filter);
  ASSERT_NE(all_of_filter, nullptr);

  const auto filters = all_of_filter->conditions();
  ASSERT_EQ(filters.size(), 2);

  const auto greater_than_filter =
   screener_filter_cast<GreaterThanFilter>(filters[0]);
  const auto less_than_filter =
   screener_filter_cast<LessThanFilter>(filters[1]);

  EXPECT_NE(greater_than_filter, nullptr);
  EXPECT_NE(less_than_filter, nullptr);

  const auto serialized_config = config_parser.serialize_filter(filter);
  const auto deserialized_filter =
   config_parser.parse_filter(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerAnyOfFilter)
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

  const auto filter = config_parser.parse_filter(config);

  const auto any_of_filter = screener_filter_cast<AnyOfFilter>(filter);
  ASSERT_NE(any_of_filter, nullptr);

  const auto conditions = any_of_filter->conditions();
  ASSERT_EQ(conditions.size(), 2);

  const auto greater_than_filter =
   screener_filter_cast<GreaterThanFilter>(conditions[0]);
  const auto less_than_filter =
   screener_filter_cast<LessThanFilter>(conditions[1]);

  EXPECT_NE(greater_than_filter, nullptr);
  EXPECT_NE(less_than_filter, nullptr);

  const auto serialized_config = config_parser.serialize_filter(filter);
  const auto deserialized_filter =
   config_parser.parse_filter(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerGreaterThanFilter)
{
  const auto config = json::parse(R"(
    {
      "filter": "GREATER_THAN",
      "threshold": 100,
      "target": {
        "method": "DATA",
        "field": "close",
        "offset": 0
      }
    }
  )");

  const auto filter = config_parser.parse_filter(config);

  const auto greater_than_filter =
   screener_filter_cast<GreaterThanFilter>(filter);
  ASSERT_NE(greater_than_filter, nullptr);

  const auto target =
   screener_method_cast<DataMethod>(greater_than_filter->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "close");
  EXPECT_EQ(target->offset(), 0);

  const auto threshold =
   screener_method_cast<ValueMethod>(greater_than_filter->threshold());
  ASSERT_NE(threshold, nullptr);

  EXPECT_EQ(threshold->value(), 100);

  const auto serialized_config = config_parser.serialize_filter(filter);
  const auto deserialized_filter =
   config_parser.parse_filter(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerGreaterEqualFilter)
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

  const auto filter = config_parser.parse_filter(config);

  const auto greater_equal_filter =
   screener_filter_cast<GreaterEqualFilter>(filter);
  ASSERT_NE(greater_equal_filter, nullptr);

  const auto target =
   screener_method_cast<DataMethod>(greater_equal_filter->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "close");
  EXPECT_EQ(target->offset(), 0);

  const auto serialized_config = config_parser.serialize_filter(filter);
  const auto deserialized_filter =
   config_parser.parse_filter(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerLessThanFilter)
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

  const auto filter = config_parser.parse_filter(config);

  const auto less_than_filter = screener_filter_cast<LessThanFilter>(filter);
  ASSERT_NE(less_than_filter, nullptr);

  const auto target =
   screener_method_cast<DataMethod>(less_than_filter->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "close");
  EXPECT_EQ(target->offset(), 0);

  const auto serialized_config = config_parser.serialize_filter(filter);
  const auto deserialized_filter =
   config_parser.parse_filter(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerLessEqualFilter)
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

  const auto filter = config_parser.parse_filter(config);

  const auto less_equal_filter = screener_filter_cast<LessEqualFilter>(filter);
  ASSERT_NE(less_equal_filter, nullptr);

  const auto target =
   screener_method_cast<DataMethod>(less_equal_filter->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "close");
  EXPECT_EQ(target->offset(), 0);

  const auto serialized_config = config_parser.serialize_filter(filter);
  const auto deserialized_filter =
   config_parser.parse_filter(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerEqualFilter)
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
        "field": "close",
        "offset": 0
      }
    }
  )");

  const auto filter = config_parser.parse_filter(config);

  const auto equal_filter = screener_filter_cast<EqualFilter>(filter);
  ASSERT_NE(equal_filter, nullptr);

  const auto target = screener_method_cast<DataMethod>(equal_filter->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "close");
  EXPECT_EQ(target->offset(), 0);

  const auto threshold =
   screener_method_cast<ValueMethod>(equal_filter->threshold());
  ASSERT_NE(threshold, nullptr);

  EXPECT_EQ(threshold->value(), 100);

  const auto serialized_config = config_parser.serialize_filter(filter);
  const auto deserialized_filter =
   config_parser.parse_filter(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerNotEqualFilter)
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
        "field": "close",
        "offset": 0
      }
    }
  )");

  const auto filter = config_parser.parse_filter(config);

  const auto not_equal_filter = screener_filter_cast<NotEqualFilter>(filter);
  ASSERT_NE(not_equal_filter, nullptr);

  const auto target =
   screener_method_cast<DataMethod>(not_equal_filter->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "close");
  EXPECT_EQ(target->offset(), 0);

  const auto threshold =
   screener_method_cast<ValueMethod>(not_equal_filter->threshold());
  ASSERT_NE(threshold, nullptr);

  EXPECT_EQ(threshold->value(), 100);

  const auto serialized_config = config_parser.serialize_filter(filter);
  const auto deserialized_filter =
   config_parser.parse_filter(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerCrossunderFilter)
{
  const auto config = json::parse(R"(
    {
      "filter": "CROSSUNDER",
      "signal": {
        "method": "DATA",
        "field": "close",
        "offset": 0
      },
      "reference": 100
    }
  )");

  const auto filter = config_parser.parse_filter(config);

  const auto crossunder_filter = screener_filter_cast<CrossunderFilter>(filter);
  ASSERT_NE(crossunder_filter, nullptr);

  const auto signal =
   screener_method_cast<DataMethod>(crossunder_filter->signal());
  ASSERT_NE(signal, nullptr);

  EXPECT_EQ(signal->field(), "close");
  EXPECT_EQ(signal->offset(), 0);

  const auto reference =
   screener_method_cast<ValueMethod>(crossunder_filter->reference());
  ASSERT_NE(reference, nullptr);

  EXPECT_EQ(reference->value(), 100);

  const auto serialized_config = config_parser.serialize_filter(filter);
  const auto deserialized_filter =
   config_parser.parse_filter(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerCrossoverFilter)
{
  const auto config = json::parse(R"(
    {
      "filter": "CROSSOVER",
      "signal": {
        "method": "DATA",
        "field": "close",
        "offset": 0
      },
      "reference": {
        "method": "VALUE",
        "value": 100
      }
    }
  )");

  const auto filter = config_parser.parse_filter(config);

  const auto crossover_filter = screener_filter_cast<CrossoverFilter>(filter);
  ASSERT_NE(crossover_filter, nullptr);

  const auto signal =
   screener_method_cast<DataMethod>(crossover_filter->signal());
  ASSERT_NE(signal, nullptr);

  EXPECT_EQ(signal->field(), "close");
  EXPECT_EQ(signal->offset(), 0);

  const auto reference =
   screener_method_cast<ValueMethod>(crossover_filter->reference());
  ASSERT_NE(reference, nullptr);

  EXPECT_EQ(reference->value(), 100);

  const auto serialized_config = config_parser.serialize_filter(filter);
  const auto deserialized_filter =
   config_parser.parse_filter(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerTrueFilter)
{
  const auto config = json::parse(R"(
    {
      "filter": "TRUE"
    }
  )");

  const auto filter = config_parser.parse_filter(config);

  const auto true_filter = screener_filter_cast<TrueFilter>(filter);
  ASSERT_NE(true_filter, nullptr);

  const auto serialized_config = config_parser.serialize_filter(filter);
  const auto deserialized_filter =
   config_parser.parse_filter(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerFalseFilter)
{
  const auto config = json::parse(R"(
    {
      "filter": "FALSE"
    }
  )");

  const auto filter = config_parser.parse_filter(config);

  const auto false_filter = screener_filter_cast<FalseFilter>(filter);
  ASSERT_NE(false_filter, nullptr);

  const auto serialized_config = config_parser.serialize_filter(filter);
  const auto deserialized_filter =
   config_parser.parse_filter(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerAndFilter)
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

  const auto and_filter = screener_filter_cast<AndFilter>(filter);
  ASSERT_NE(and_filter, nullptr);

  const auto first_condition = and_filter->first_condition();
  const auto second_condition = and_filter->second_condition();

  const auto true_filter = screener_filter_cast<TrueFilter>(first_condition);
  const auto false_filter = screener_filter_cast<FalseFilter>(second_condition);

  ASSERT_NE(true_filter, nullptr);
  ASSERT_NE(false_filter, nullptr);

  const auto serialized_config = config_parser.serialize_filter(filter);
  const auto deserialized_filter =
   config_parser.parse_filter(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerOrFilter)
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

  const auto or_filter = screener_filter_cast<OrFilter>(filter);
  ASSERT_NE(or_filter, nullptr);

  const auto first_condition = or_filter->first_condition();
  const auto second_condition = or_filter->second_condition();

  const auto true_filter = screener_filter_cast<TrueFilter>(first_condition);
  const auto false_filter = screener_filter_cast<FalseFilter>(second_condition);

  ASSERT_NE(true_filter, nullptr);
  ASSERT_NE(false_filter, nullptr);

  const auto serialized_config = config_parser.serialize_filter(filter);
  const auto deserialized_filter =
   config_parser.parse_filter(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerNotFilter)
{
  const auto config = json::parse(R"(
    {
      "filter": "NOT",
      "condition": true
    }
  )");

  const auto filter = config_parser.parse_filter(config);

  const auto not_filter = screener_filter_cast<NotFilter>(filter);
  ASSERT_NE(not_filter, nullptr);

  const auto condition = not_filter->condition();
  const auto true_filter = screener_filter_cast<TrueFilter>(condition);

  ASSERT_NE(true_filter, nullptr);

  const auto serialized_config = config_parser.serialize_filter(filter);
  const auto deserialized_filter =
   config_parser.parse_filter(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerXorFilter)
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

  const auto xor_filter = screener_filter_cast<XorFilter>(filter);
  ASSERT_NE(xor_filter, nullptr);

  const auto first_condition = xor_filter->first_condition();
  const auto second_condition = xor_filter->second_condition();

  const auto true_filter = screener_filter_cast<TrueFilter>(first_condition);
  const auto false_filter = screener_filter_cast<FalseFilter>(second_condition);

  ASSERT_NE(true_filter, nullptr);
  ASSERT_NE(false_filter, nullptr);

  const auto serialized_config = config_parser.serialize_filter(filter);
  const auto deserialized_filter =
   config_parser.parse_filter(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerFilterIsInvalid)
{
  const auto config = json::parse(R"(
    {
      "filter": "invalid not existed filter should throw exception"
    }
  )");

  EXPECT_THROW(config_parser.parse_filter(config), std::exception);
}

TEST_F(ConfigParserTest, ParseScreenerFilterWithInvalidRequiredFields)
{
  const auto config = json::parse(R"(
    {
      "filter": "AND"
    }
  )");

  EXPECT_THROW(config_parser.parse_filter(config), std::exception);
}
