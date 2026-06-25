#include <gtest/gtest.h>

#include <utility>

#include <jsoncons/json.hpp>

import pludux;

using namespace pludux;
using json = jsoncons::ojson;

using SignalAllOfNode = pludux::AllOfNode;
using SignalAnyOfNode = pludux::AnyOfNode;
using SignalAlwaysNode = pludux::TrueNode;
using SignalNeverNode = pludux::FalseNode;
using SignalEqualNode = pludux::EqualNode;
using SignalNotEqualNode = pludux::NotEqualNode;
using SignalGreaterThanNode = pludux::GreaterThanNode;
using SignalGreaterEqualNode = pludux::GreaterEqualNode;
using SignalLessThanNode = pludux::LessThanNode;
using SignalLessEqualNode = pludux::LessEqualNode;
using SignalCrossoverNode = pludux::CrossoverNode;
using SignalCrossunderNode = pludux::CrossunderNode;
using SignalNotNode = pludux::LogicalNotNode;
using SignalAndNode = pludux::LogicalAndNode;
using SignalOrNode = pludux::LogicalOrNode;
using SignalXorNode = pludux::LogicalXorNode;

auto node_context = NodeToErasedMethodContext{};

class ParsedConfigNodeMethod {
public:
  ParsedConfigNodeMethod(ErasedNode node)
  : node_{std::move(node)}
  , method_{node_to_erased_method(node_, node_context)}
  {
  }

  auto method(this const ParsedConfigNodeMethod& self) noexcept
   -> const AnySeriesMethod&
  {
    return self.method_;
  }

  auto node(this const ParsedConfigNodeMethod& self) noexcept
   -> const ErasedNode&
  {
    return self.node_;
  }

  auto operator==(this const ParsedConfigNodeMethod& self,
                  const ParsedConfigNodeMethod& other) noexcept -> bool
  {
    return self.method_ == other.method_;
  }

private:
  ErasedNode node_;
  AnySeriesMethod method_;
};

template<typename UMethod>
auto series_method_cast(
 const ParsedConfigNodeMethod& parsed_node_method) noexcept -> const UMethod*
{
  const auto& method = parsed_node_method.method();
  return series_method_cast<UMethod>(method);
}

class ConfigParserTest : public ::testing::Test {
protected:
  ConfigParser config_parser;

  auto parse_node_method(const jsoncons::ojson& config)
   -> ParsedConfigNodeMethod
  {
    return ParsedConfigNodeMethod{config_parser.parse_node(config)};
  }

  auto
  serialize_node_method(const ParsedConfigNodeMethod& parsed_node_method) const
   -> jsoncons::ojson
  {
    return config_parser.serialize_node(parsed_node_method.node());
  }

  void SetUp() override
  {
    config_parser = std::move(make_default_registered_config_parser());

    parse_node_method(json::parse(R"(
      {
        "method": "DATA",
        "params": {
          "name": "high",
          "field": "high"
        }
      }
    )"));

    parse_node_method(json::parse(R"(
      {
        "method": "DATA",
        "params": {
          "name": "low",
          "field": "low"
        }
      }
    )"));

    parse_node_method(json::parse(R"(
      {
        "method": "DATA",
        "params": {
          "name": "close",
          "field": "close"
        }
      }
    )"));

    parse_node_method(json::parse(R"(
      {
        "method": "DATA",
        "params": {
          "name": "volume",
          "field": "volume"
        }
      }
    )"));
  }
};

TEST_F(ConfigParserTest, ParseScreenerSeriesNodeMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "SERIES_NODE",
      "params": {
        "name": "close"
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto series_node_method = series_method_cast<SeriesNodeMethod>(method);
  ASSERT_NE(series_node_method, nullptr);

  EXPECT_EQ(series_node_method->name(), "close");

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerSeriesValueMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "SERIES_VALUE",
      "params": {
        "name": "close"
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto series_value_method =
   series_method_cast<SeriesValueMethod>(method);
  ASSERT_NE(series_value_method, nullptr);

  EXPECT_EQ(series_value_method->name(), "close");

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerLookbackMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "LOOKBACK",
      "params": {
        "period": 3,
        "source": {
          "method": "CLOSE"
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto lookback_method =
   series_method_cast<LookbackMethod<AnySeriesMethod>>(method);
  ASSERT_NE(lookback_method, nullptr);

  EXPECT_EQ(lookback_method->period(), 3);
  const auto source_method =
   series_method_cast<CloseMethod>(lookback_method->source());
  ASSERT_NE(source_method, nullptr);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseNumericInputNode)
{
  const auto config = json::parse(R"(
    {
      "method": "INPUT",
      "params": {
        "label": "Length",
        "representation": "UnsignedInteger",
        "value": 14.9
      }
    }
  )");

  const auto parsed_node = config_parser.parse_node(config);
  const auto* input_node = node_cast<NumericInputNode>(parsed_node);

  ASSERT_NE(input_node, nullptr);
  EXPECT_EQ(input_node->label(), "Length");
  EXPECT_EQ(input_node->representation(),
            NumericInputNode::ValueRepresentation::UnsignedInteger);
  EXPECT_DOUBLE_EQ(input_node->value(), 14.9);

  const auto serialized_config = config_parser.serialize_node(parsed_node);
  const auto deserialized_node = config_parser.parse_node(serialized_config);
  EXPECT_EQ(parsed_node, deserialized_node);
}

TEST_F(ConfigParserTest, ParseScreenerSelectOutputMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "SELECT_OUTPUT",
      "params": {
        "output": "upper-band",
        "source": {
          "method": "MACD",
          "params": {
            "fastPeriod": 12,
            "slowPeriod": 26,
            "signalPeriod": 9,
            "input": {
              "method": "CLOSE"
            }
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto select_output_method =
   series_method_cast<SelectOutputMethod<AnySeriesMethod>>(method);
  ASSERT_NE(select_output_method, nullptr);
  EXPECT_EQ(select_output_method->output(), MethodOutput::UpperBand);

  const auto macd_method = series_method_cast<MacdMethod<AnySeriesMethod>>(
   select_output_method->source());
  ASSERT_NE(macd_method, nullptr);

  const auto source = series_method_cast<CloseMethod>(macd_method->source());
  EXPECT_NE(source, nullptr);
  EXPECT_EQ(macd_method->fast_period(), 12);
  EXPECT_EQ(macd_method->slow_period(), 26);
  EXPECT_EQ(macd_method->signal_period(), 9);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerOpenMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "OPEN"
    }
  )");

  const auto method = parse_node_method(config);

  const auto open_method = series_method_cast<OpenMethod>(method);
  ASSERT_NE(open_method, nullptr);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerHighMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "HIGH"
    }
  )");

  const auto method = parse_node_method(config);

  const auto high_method = series_method_cast<HighMethod>(method);
  ASSERT_NE(high_method, nullptr);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerLowMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "LOW"
    }
  )");

  const auto method = parse_node_method(config);

  const auto low_method = series_method_cast<LowMethod>(method);
  ASSERT_NE(low_method, nullptr);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerCloseMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "CLOSE"
    }
  )");

  const auto method = parse_node_method(config);

  const auto close_method = series_method_cast<CloseMethod>(method);
  ASSERT_NE(close_method, nullptr);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerVolumeMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "VOLUME"
    }
  )");

  const auto method = parse_node_method(config);

  const auto volume_method = series_method_cast<VolumeMethod>(method);
  ASSERT_NE(volume_method, nullptr);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerSmaMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "SMA",
      "params": {
        "period": 14,
        "source": {
          "method": "DATA",
          "params": {
            "field": "close"
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto sma_method =
   series_method_cast<SmaMethod<AnySeriesMethod>>(method);
  ASSERT_NE(sma_method, nullptr);

  EXPECT_EQ(sma_method->period(), 14);

  const auto source = series_method_cast<DataMethod>(sma_method->source());
  ASSERT_NE(source, nullptr);

  EXPECT_EQ(source->field(), "close");

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerEmaMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "EMA",
      "params": {
        "period": 10,
        "source": {
          "method": "DATA",
          "params": {
            "field": "open"
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto ema_method =
   series_method_cast<EmaMethod<AnySeriesMethod>>(method);
  ASSERT_NE(ema_method, nullptr);

  EXPECT_EQ(ema_method->period(), 10);

  const auto source = series_method_cast<DataMethod>(ema_method->source());
  ASSERT_NE(source, nullptr);

  EXPECT_EQ(source->field(), "open");

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerWmaMethod)
{
  const auto config = json::parse(R"(
      {
        "method": "WMA",
        "params": {
          "period": 20,
          "source": {
            "method": "DATA",
            "params": {
              "field": "high"
            }
          }
        }
      }
    )");

  const auto method = parse_node_method(config);

  const auto wma_method =
   series_method_cast<WmaMethod<AnySeriesMethod>>(method);
  ASSERT_NE(wma_method, nullptr);

  EXPECT_EQ(wma_method->period(), 20);

  const auto source = series_method_cast<DataMethod>(wma_method->source());
  ASSERT_NE(source, nullptr);

  EXPECT_EQ(source->field(), "high");

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerRmaMethod)
{
  const auto config = json::parse(R"(
      {
        "method": "RMA",
        "params": {
          "period": 15,
          "source": {
            "method": "DATA",
            "params": {
              "field": "low"
            }
          }
        }
      }
    )");

  const auto method = parse_node_method(config);

  const auto rma_method =
   series_method_cast<RmaMethod<AnySeriesMethod>>(method);
  ASSERT_NE(rma_method, nullptr);

  EXPECT_EQ(rma_method->period(), 15);

  const auto source = series_method_cast<DataMethod>(rma_method->source());
  ASSERT_NE(source, nullptr);

  EXPECT_EQ(source->field(), "low");

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerHmaMethod)
{
  const auto config = json::parse(R"(
      {
        "method": "HMA",
        "params": {
          "period": 25,
          "source": {
            "method": "DATA",
            "params": {
              "field": "volume"
            }
          }
        }
      }
    )");

  const auto method = parse_node_method(config);

  const auto hma_method =
   series_method_cast<HmaMethod<AnySeriesMethod>>(method);
  ASSERT_NE(hma_method, nullptr);

  EXPECT_EQ(hma_method->period(), 25);

  const auto source = series_method_cast<DataMethod>(hma_method->source());
  ASSERT_NE(source, nullptr);

  EXPECT_EQ(source->field(), "volume");

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerRsiMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "RSI",
      "params": {
        "period": 14,
        "source": {
          "method": "DATA",
          "params": {
            "field": "close"
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto rsi_method =
   series_method_cast<RsiMethod<AnySeriesMethod>>(method);
  ASSERT_NE(rsi_method, nullptr);

  EXPECT_EQ(rsi_method->period(), 14);

  const auto source = series_method_cast<DataMethod>(rsi_method->source());
  ASSERT_NE(source, nullptr);

  EXPECT_EQ(source->field(), "close");

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerStddevMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "STDDEV",
      "params": {
        "period": 20,
        "source": {
          "method": "DATA",
          "params": {
            "field": "close"
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto stddev_method =
   series_method_cast<StddevMethod<AnySeriesMethod>>(method);
  ASSERT_NE(stddev_method, nullptr);

  EXPECT_EQ(stddev_method->period(), 20);

  const auto source = series_method_cast<DataMethod>(stddev_method->source());
  ASSERT_NE(source, nullptr);

  EXPECT_EQ(source->field(), "close");

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerValueMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "VALUE",
      "params": {
        "value": 100
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto value_method = series_method_cast<ValueMethod>(method);
  ASSERT_NE(value_method, nullptr);

  EXPECT_EQ(value_method->value(), 100);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerDataMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "DATA",
      "params": {
        "field": "open"
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto field_method = series_method_cast<DataMethod>(method);
  ASSERT_NE(field_method, nullptr);

  EXPECT_EQ(field_method->field(), "open");

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerAtrMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "ATR",
      "params": {
        "period": 14,
        "maSmoothingType": "RMA"
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto atr_method = series_method_cast<AtrMethod>(method);
  ASSERT_NE(atr_method, nullptr);

  EXPECT_EQ(atr_method->period(), 14);
  EXPECT_EQ(atr_method->ma_smoothing_type(), MaMethodType::Rma);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerBbMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "BB",
      "params": {
        "maType": "SMA",
        "period": 20,
        "stddev": 2.0,
        "maSource": {
          "method": "DATA",
          "params": {
            "field": "close"
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto bb_method = series_method_cast<BbMethod<AnySeriesMethod>>(method);
  ASSERT_NE(bb_method, nullptr);

  const auto ma_source = series_method_cast<DataMethod>(bb_method->source());
  EXPECT_NE(ma_source, nullptr);
  EXPECT_EQ(bb_method->ma_method_type(), MaMethodType::Sma);
  EXPECT_EQ(bb_method->period(), 20);
  EXPECT_EQ(bb_method->stddev(), 2.0);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerMacdMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "MACD",
      "params": {
        "fast": 12,
        "slow": 26,
        "signal": 9,
        "source": {
          "method": "DATA",
          "params": {
            "field": "close"
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto macd_method =
   series_method_cast<MacdMethod<AnySeriesMethod>>(method);
  ASSERT_NE(macd_method, nullptr);

  const auto source = series_method_cast<DataMethod>(macd_method->source());
  EXPECT_NE(source, nullptr);
  EXPECT_EQ(macd_method->fast_period(), 12);
  EXPECT_EQ(macd_method->slow_period(), 26);
  EXPECT_EQ(macd_method->signal_period(), 9);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerStochMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "STOCH",
      "params": {
        "kPeriod": 5,
        "kSmooth": 3,
        "dPeriod": 3
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto stoch_method = series_method_cast<StochMethod>(method);
  ASSERT_NE(stoch_method, nullptr);

  EXPECT_EQ(stoch_method->k_period(), 5);
  EXPECT_EQ(stoch_method->k_smooth(), 3);
  EXPECT_EQ(stoch_method->d_period(), 3);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerStochRsiMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "STOCH_RSI",
      "params": {
        "rsiPeriod": 14,
        "kPeriod": 5,
        "kSmooth": 3,
        "dPeriod": 3,
        "rsiSource": {
          "method": "DATA",
          "params": {
            "field": "close"
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

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

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerKcMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "KC",
      "params": {
        "maMethodType": "SMA",
        "maPeriod": 5,
        "maSource": {
          "method": "DATA",
          "params": {
            "field": "close"
          }
        },
        "bandMethodType": "ATR",
        "bandAtrPeriod": 14,
        "multiplier": 1.0
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto kc_method = series_method_cast<KcMethod<AnySeriesMethod>>(method);
  ASSERT_NE(kc_method, nullptr);

  const auto ma_source_method =
   series_method_cast<DataMethod>(kc_method->source());
  EXPECT_NE(ma_source_method, nullptr);
  EXPECT_EQ(kc_method->ma_method_type(), MaMethodType::Sma);
  EXPECT_EQ(kc_method->period(), 5);
  EXPECT_EQ(kc_method->band_method_type(), KcBandMethodType::Atr);
  EXPECT_EQ(kc_method->band_atr_period(), 14);
  EXPECT_EQ(kc_method->multiplier(), 1.0);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerDonchianChannelMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "DC",
      "params": {
        "period": 5
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto dc_method = series_method_cast<DonchianChannelMethod>(method);
  ASSERT_NE(dc_method, nullptr);

  EXPECT_EQ(dc_method->period(), 5);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerAddMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "ADD",
      "params": {
        "augend": 50,
        "addend": {
          "method": "VALUE",
          "params": {
            "value": 25
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto add_method =
   series_method_cast<AddMethod<AnySeriesMethod, AnySeriesMethod>>(method);
  ASSERT_NE(add_method, nullptr);

  const auto augend = series_method_cast<ValueMethod>(add_method->left());
  const auto addend = series_method_cast<ValueMethod>(add_method->right());
  ASSERT_NE(augend, nullptr);
  ASSERT_NE(addend, nullptr);
  EXPECT_EQ(augend->value(), 50);
  EXPECT_EQ(addend->value(), 25);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerSubtractMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "SUBTRACT",
      "params": {
        "minuend": {
          "method": "VALUE",
          "params": {
            "value": 100
          }
        },
        "subtrahend": {
          "method": "VALUE",
          "params": {
            "value": 30
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto subtract_method =
   series_method_cast<SubtractMethod<AnySeriesMethod, AnySeriesMethod>>(method);
  ASSERT_NE(subtract_method, nullptr);

  const auto minuend = series_method_cast<ValueMethod>(subtract_method->left());
  const auto subtrahend =
   series_method_cast<ValueMethod>(subtract_method->right());
  ASSERT_NE(minuend, nullptr);
  ASSERT_NE(subtrahend, nullptr);
  EXPECT_EQ(minuend->value(), 100);
  EXPECT_EQ(subtrahend->value(), 30);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerMultiplyMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "MULTIPLY",
      "params": {
        "multiplicand": {
          "method": "VALUE",
          "params": {
            "value": 10
          }
        },
        "multiplier": {
          "method": "VALUE",
          "params": {
            "value": 5
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto multiply_method =
   series_method_cast<MultiplyMethod<AnySeriesMethod, AnySeriesMethod>>(method);
  ASSERT_NE(multiply_method, nullptr);

  const auto multiplicand =
   series_method_cast<ValueMethod>(multiply_method->left());
  const auto multiplier =
   series_method_cast<ValueMethod>(multiply_method->right());
  ASSERT_NE(multiplicand, nullptr);
  ASSERT_NE(multiplier, nullptr);
  EXPECT_EQ(multiplicand->value(), 10);
  EXPECT_EQ(multiplier->value(), 5);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerDivideMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "DIVIDE",
      "params": {
        "dividend": {
          "method": "VALUE",
          "params": {
            "value": 100
          }
        },
        "divisor": {
          "method": "VALUE",
          "params": {
            "value": 2
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto divide_method =
   series_method_cast<DivideMethod<AnySeriesMethod, AnySeriesMethod>>(method);
  ASSERT_NE(divide_method, nullptr);

  const auto dividend = series_method_cast<ValueMethod>(divide_method->left());
  const auto divisor = series_method_cast<ValueMethod>(divide_method->right());
  ASSERT_NE(dividend, nullptr);
  ASSERT_NE(divisor, nullptr);
  EXPECT_EQ(dividend->value(), 100);
  EXPECT_EQ(divisor->value(), 2);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerNegateMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "NEGATE",
      "params": {
        "operand": {
          "method": "VALUE",
          "params": {
            "value": 42
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto negate_method =
   series_method_cast<NegateMethod<AnySeriesMethod>>(method);
  ASSERT_NE(negate_method, nullptr);

  const auto operand =
   series_method_cast<ValueMethod>(negate_method->operand());
  ASSERT_NE(operand, nullptr);
  EXPECT_EQ(operand->value(), 42);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerSqrtMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "SQRT",
      "params": {
        "operand": {
          "method": "VALUE",
          "params": {
            "value": 16
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto sqrt_method =
   series_method_cast<SqrtMethod<AnySeriesMethod>>(method);
  ASSERT_NE(sqrt_method, nullptr);

  const auto operand = series_method_cast<ValueMethod>(sqrt_method->operand());
  ASSERT_NE(operand, nullptr);
  EXPECT_EQ(operand->value(), 16);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerChangeMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "CHANGE",
      "params": {
        "source": {
          "method": "DATA",
          "params": {
            "field": "close"
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto changes_method =
   series_method_cast<ChangeMethod<AnySeriesMethod>>(method);
  ASSERT_NE(changes_method, nullptr);

  const auto source = series_method_cast<DataMethod>(changes_method->source());
  ASSERT_NE(source, nullptr);
  EXPECT_EQ(source->field(), "close");

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerAbsDiffMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "ABS_DIFF",
      "params": {
        "minuend": {
          "method": "DATA",
          "params": {
            "field": "high"
          }
        },
        "subtrahend": {
          "method": "DATA",
          "params": {
            "field": "low"
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto abs_diff_method =
   series_method_cast<AbsDiffMethod<AnySeriesMethod, AnySeriesMethod>>(method);
  ASSERT_NE(abs_diff_method, nullptr);

  const auto minuend = series_method_cast<DataMethod>(abs_diff_method->left());
  const auto subtrahend =
   series_method_cast<DataMethod>(abs_diff_method->right());
  ASSERT_NE(minuend, nullptr);
  ASSERT_NE(subtrahend, nullptr);
  EXPECT_EQ(minuend->field(), "high");
  EXPECT_EQ(subtrahend->field(), "low");

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerPercentageMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "PERCENTAGE",
      "params": {
        "base": {
          "method": "VALUE",
          "params": {
            "value": 100
          }
        },
        "percent": 20
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto percentage_method =
   series_method_cast<PercentageMethod<AnySeriesMethod>>(method);
  ASSERT_NE(percentage_method, nullptr);

  const auto base = series_method_cast<ValueMethod>(percentage_method->base());
  ASSERT_NE(base, nullptr);
  EXPECT_EQ(base->value(), 100);

  const auto percent = percentage_method->percent();
  EXPECT_EQ(percent, 20);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerInvalidMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "INVALID"
    }
  )");

  EXPECT_THROW(parse_node_method(config), std::invalid_argument);
}

TEST_F(ConfigParserTest, ParseScreenerAllOfMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "ALL_OF",
      "params": {
        "items": [
          {
            "method": "GREATER_THAN",
            "params": {
              "threshold": {
                "method": "VALUE",
                "params": {
                  "value": 100
                }
              },
              "target": {
                "method": "DATA",
                "params": {
                  "field": "close"
                }
              }
            }
          },
          {
            "method": "LESS_THAN",
            "params": {
              "threshold": {
                "method": "VALUE",
                "params": {
                  "value": 200
                }
              },
              "target": {
                "method": "DATA",
                "params": {
                  "field": "close"
                }
              }
            }
          }
        ]
      }
    }
  )");

  const auto filter = config_parser.parse_node(config);
  ASSERT_EQ(config.at("params").at("items").size(), 2);
  const auto parsed_all_of = node_cast<SignalAllOfNode>(filter);
  ASSERT_NE(parsed_all_of, nullptr);
  ASSERT_EQ(parsed_all_of->conditions().size(), 2);

  const auto serialized_config = config_parser.serialize_node(filter);

  const auto deserialized_filter = config_parser.parse_node(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerAnyOfMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "ANY_OF",
      "params": {
        "items": [
          {
            "method": "GREATER_THAN",
            "params": {
              "threshold": {
                "method": "VALUE",
                "params": {
                  "value": 100
                }
              },
              "target": {
                "method": "DATA",
                "params": {
                  "field": "close"
                }
              }
            }
          },
          {
            "method": "LESS_THAN",
            "params": {
              "threshold": {
                "method": "VALUE",
                "params": {
                  "value": 200
                }
              },
              "target": {
                "method": "DATA",
                "params": {
                  "field": "close"
                }
              }
            }
          }
        ]
      }
    }
  )");

  const auto filter = config_parser.parse_node(config);
  ASSERT_EQ(config.at("params").at("items").size(), 2);
  const auto parsed_any_of = node_cast<SignalAnyOfNode>(filter);
  ASSERT_NE(parsed_any_of, nullptr);
  ASSERT_EQ(parsed_any_of->conditions().size(), 2);

  const auto serialized_config = config_parser.serialize_node(filter);

  const auto deserialized_filter = config_parser.parse_node(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerGreaterThanMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "GREATER_THAN",
      "params": {
        "threshold": {
          "method": "VALUE",
          "params": {
            "value": 100
          }
        },
        "target": {
          "method": "DATA",
          "params": {
            "field": "close"
          }
        }
      }
    }
  )");

  const auto filter = config_parser.parse_node(config);

  const auto greater_than_filter = node_cast<SignalGreaterThanNode>(filter);
  ASSERT_NE(greater_than_filter, nullptr);

  const auto target = node_cast<DataNode>(greater_than_filter->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "close");

  const auto threshold = node_cast<ValueNode>(greater_than_filter->threshold());
  ASSERT_NE(threshold, nullptr);

  EXPECT_EQ(threshold->value(), 100);

  const auto serialized_config = config_parser.serialize_node(filter);
  const auto deserialized_filter = config_parser.parse_node(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerGreaterEqualMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "GREATER_EQUAL",
      "params": {
        "threshold": {
          "method": "VALUE",
          "params": {
            "value": 100
          }
        },
        "target": {
          "method": "DATA",
          "params": {
            "field": "close"
          }
        }
      }
    }
  )");

  const auto filter = config_parser.parse_node(config);

  const auto greater_equal_filter = node_cast<SignalGreaterEqualNode>(filter);
  ASSERT_NE(greater_equal_filter, nullptr);

  const auto target = node_cast<DataNode>(greater_equal_filter->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "close");

  const auto serialized_config = config_parser.serialize_node(filter);
  const auto deserialized_filter = config_parser.parse_node(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerLessThanMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "LESS_THAN",
      "params": {
        "threshold": {
          "method": "VALUE",
          "params": {
            "value": 100
          }
        },
        "target": {
          "method": "DATA",
          "params": {
            "field": "close"
          }
        }
      }
    }
  )");

  const auto filter = config_parser.parse_node(config);

  const auto less_than_filter = node_cast<SignalLessThanNode>(filter);
  ASSERT_NE(less_than_filter, nullptr);

  const auto target = node_cast<DataNode>(less_than_filter->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "close");

  const auto serialized_config = config_parser.serialize_node(filter);
  const auto deserialized_filter = config_parser.parse_node(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerLessEqualMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "LESS_EQUAL",
      "params": {
        "threshold": {
          "method": "VALUE",
          "params": {
            "value": 100
          }
        },
        "target": {
          "method": "DATA",
          "params": {
            "field": "close"
          }
        }
      }
    }
  )");

  const auto filter = config_parser.parse_node(config);

  const auto less_equal_filter = node_cast<SignalLessEqualNode>(filter);
  ASSERT_NE(less_equal_filter, nullptr);

  const auto target = node_cast<DataNode>(less_equal_filter->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "close");

  const auto serialized_config = config_parser.serialize_node(filter);
  const auto deserialized_filter = config_parser.parse_node(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerEqualMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "EQUAL",
      "params": {
        "threshold": {
          "method": "VALUE",
          "params": {
            "value": 100
          }
        },
        "target": {
          "method": "DATA",
          "params": {
            "field": "close"
          }
        }
      }
    }
  )");

  const auto filter = config_parser.parse_node(config);

  const auto equal_filter = node_cast<SignalEqualNode>(filter);
  ASSERT_NE(equal_filter, nullptr);

  const auto target = node_cast<DataNode>(equal_filter->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "close");

  const auto threshold = node_cast<ValueNode>(equal_filter->threshold());
  ASSERT_NE(threshold, nullptr);

  EXPECT_EQ(threshold->value(), 100);

  const auto serialized_config = config_parser.serialize_node(filter);
  const auto deserialized_filter = config_parser.parse_node(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerNotEqualMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "NOT_EQUAL",
      "params": {
        "threshold": {
          "method": "VALUE",
          "params": {
            "value": 100
          }
        },
        "target": {
          "method": "DATA",
          "params": {
            "field": "close"
          }
        }
      }
    }
  )");

  const auto filter = config_parser.parse_node(config);

  const auto not_equal_filter = node_cast<SignalNotEqualNode>(filter);
  ASSERT_NE(not_equal_filter, nullptr);

  const auto target = node_cast<DataNode>(not_equal_filter->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "close");

  const auto threshold = node_cast<ValueNode>(not_equal_filter->threshold());
  ASSERT_NE(threshold, nullptr);

  EXPECT_EQ(threshold->value(), 100);

  const auto serialized_config = config_parser.serialize_node(filter);
  const auto deserialized_filter = config_parser.parse_node(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerCrossunderMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "CROSSUNDER",
      "params": {
        "value": {
          "method": "DATA",
          "params": {
            "field": "close"
          }
        },
        "baseline": {
          "method": "VALUE",
          "params": {
            "value": 100
          }
        }
      }
    }
  )");

  const auto filter = config_parser.parse_node(config);

  const auto crossunder_filter = node_cast<SignalCrossunderNode>(filter);
  ASSERT_NE(crossunder_filter, nullptr);

  const auto signal = node_cast<DataNode>(crossunder_filter->source());
  ASSERT_NE(signal, nullptr);

  EXPECT_EQ(signal->field(), "close");

  const auto reference = node_cast<ValueNode>(crossunder_filter->reference());
  ASSERT_NE(reference, nullptr);

  EXPECT_EQ(reference->value(), 100);

  const auto serialized_config = config_parser.serialize_node(filter);
  const auto deserialized_filter = config_parser.parse_node(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerCrossoverMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "CROSSOVER",
      "params": {
        "value": {
          "method": "DATA",
          "params": {
            "field": "close"
          }
        },
        "baseline": {
          "method": "VALUE",
          "params": {
            "value": 100
          }
        }
      }
    }
  )");

  const auto filter = config_parser.parse_node(config);

  const auto crossover_filter = node_cast<SignalCrossoverNode>(filter);
  ASSERT_NE(crossover_filter, nullptr);

  const auto signal = node_cast<DataNode>(crossover_filter->source());
  ASSERT_NE(signal, nullptr);

  EXPECT_EQ(signal->field(), "close");

  const auto reference = node_cast<ValueNode>(crossover_filter->reference());
  ASSERT_NE(reference, nullptr);

  EXPECT_EQ(reference->value(), 100);

  const auto serialized_config = config_parser.serialize_node(filter);
  const auto deserialized_filter = config_parser.parse_node(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerAlwaysMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "ALWAYS"
    }
  )");

  const auto filter = config_parser.parse_node(config);

  const auto always_filter = node_cast<SignalAlwaysNode>(filter);
  ASSERT_NE(always_filter, nullptr);

  const auto serialized_config = config_parser.serialize_node(filter);
  const auto deserialized_filter = config_parser.parse_node(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerNeverMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "NEVER"
    }
  )");

  const auto filter = config_parser.parse_node(config);

  const auto never_filter = node_cast<SignalNeverNode>(filter);
  ASSERT_NE(never_filter, nullptr);

  const auto serialized_config = config_parser.serialize_node(filter);
  const auto deserialized_filter = config_parser.parse_node(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerAndMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "AND",
      "params": {
        "firstCondition": {
          "method": "ALWAYS"
        },
        "secondCondition": {
          "method": "NEVER"
        }
      }
    }
  )");

  const auto filter = config_parser.parse_node(config);

  const auto and_filter = node_cast<SignalAndNode>(filter);
  ASSERT_NE(and_filter, nullptr);

  const auto first_condition = and_filter->first_condition();
  const auto second_condition = and_filter->second_condition();

  const auto always_filter = node_cast<SignalAlwaysNode>(first_condition);
  const auto never_filter = node_cast<SignalNeverNode>(second_condition);

  ASSERT_NE(always_filter, nullptr);
  ASSERT_NE(never_filter, nullptr);

  const auto serialized_config = config_parser.serialize_node(filter);
  const auto deserialized_filter = config_parser.parse_node(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerOrMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "OR",
      "params": {
        "firstCondition": {
          "method": "ALWAYS"
        },
        "secondCondition": {
          "method": "NEVER"
        }
      }
    }
  )");

  const auto filter = config_parser.parse_node(config);

  const auto or_filter = node_cast<SignalOrNode>(filter);
  ASSERT_NE(or_filter, nullptr);

  const auto first_condition = or_filter->first_condition();
  const auto second_condition = or_filter->second_condition();

  const auto always_filter = node_cast<SignalAlwaysNode>(first_condition);
  const auto never_filter = node_cast<SignalNeverNode>(second_condition);

  ASSERT_NE(always_filter, nullptr);
  ASSERT_NE(never_filter, nullptr);

  const auto serialized_config = config_parser.serialize_node(filter);
  const auto deserialized_filter = config_parser.parse_node(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerNotMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "NOT",
      "params": {
        "condition": true
      }
    }
  )");

  const auto filter = config_parser.parse_node(config);

  const auto not_filter = node_cast<SignalNotNode>(filter);
  ASSERT_NE(not_filter, nullptr);

  const auto other_condition = not_filter->other_condition();
  const auto always_filter = node_cast<SignalAlwaysNode>(other_condition);

  ASSERT_NE(always_filter, nullptr);

  const auto serialized_config = config_parser.serialize_node(filter);
  const auto deserialized_filter = config_parser.parse_node(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerXorMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "XOR",
      "params": {
        "firstCondition": {
          "method": "ALWAYS"
        },
        "secondCondition": {
          "method": "NEVER"
        }
      }
    }
  )");

  const auto filter = config_parser.parse_node(config);

  const auto xor_filter = node_cast<SignalXorNode>(filter);
  ASSERT_NE(xor_filter, nullptr);

  const auto first_condition = xor_filter->first_condition();
  const auto second_condition = xor_filter->second_condition();

  const auto always_filter = node_cast<SignalAlwaysNode>(first_condition);
  const auto never_filter = node_cast<SignalNeverNode>(second_condition);

  ASSERT_NE(always_filter, nullptr);
  ASSERT_NE(never_filter, nullptr);

  const auto serialized_config = config_parser.serialize_node(filter);
  const auto deserialized_filter = config_parser.parse_node(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseAnyConditionMethodIsInvalid)
{
  const auto config = json::parse(R"(
    {
      "method": "invalid not existed filter should throw exception"
    }
  )");

  EXPECT_THROW(config_parser.parse_node(config), std::exception);
}

TEST_F(ConfigParserTest, ParseAnyConditionMethodWithInvalidRequiredFields)
{
  const auto config = json::parse(R"(
    {
      "method": "AND"
    }
  )");

  EXPECT_THROW(config_parser.parse_node(config), std::exception);
}

TEST_F(ConfigParserTest, SeriesNodeRegistrySerializationDeserialization)
{
  const auto config = json::parse(R"(
    {
      "name1": {
        "method": "DATA",
        "params": {
          "field": "close"
        }
      },
      "name2": {
        "method": "VALUE",
        "params": {
          "value": 100
        }
      }
    }
  )");

  auto series_nodes = OrderedNamedRegistry<ErasedNode>{};
  series_nodes.set("name1", DataNode{"close"});
  series_nodes.set("name2", ValueNode{100});

  auto serialize_series_nodes =
   [this](const OrderedNamedRegistry<ErasedNode>& series_nodes) {
     auto series_nodes_config = jsoncons::ojson{};
     for(const auto& [series_name, series_node] : series_nodes) {
       series_nodes_config[series_name] =
        config_parser.serialize_node(series_node);
     }
     return series_nodes_config;
   };
  auto parse_series_nodes = [this](const jsoncons::ojson& series_nodes_config) {
    auto parsed_series_nodes = OrderedNamedRegistry<ErasedNode>{};
    for(const auto& [series_name, series_config] :
        series_nodes_config.object_range()) {
      parsed_series_nodes.set(series_name,
                              config_parser.parse_node(series_config));
    }
    return parsed_series_nodes;
  };

  const auto serialized_config = serialize_series_nodes(series_nodes);

  const auto deserialized_series_nodes = parse_series_nodes(serialized_config);
  const auto deserialized_config = parse_series_nodes(config);
  EXPECT_EQ(deserialized_config, deserialized_series_nodes);
  EXPECT_EQ(series_nodes, deserialized_series_nodes);
}
