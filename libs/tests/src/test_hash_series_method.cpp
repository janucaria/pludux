#include <gtest/gtest.h>

import pludux;

using namespace pludux;

// ============================================================================
// Hash Determinism Tests - Same input should always produce same hash
// ============================================================================

class HashSeriesMethodDeterminismTest : public ::testing::Test {};

TEST_F(HashSeriesMethodDeterminismTest, AtrMethodHashDeterministic)
{
  AtrMethod atr1{14};
  AtrMethod atr2{14};
  EXPECT_EQ(hash_series_method(atr1), hash_series_method(atr2));
}

TEST_F(HashSeriesMethodDeterminismTest, DataMethodHashDeterministic)
{
  DataMethod data1{"close"};
  DataMethod data2{"close"};
  EXPECT_EQ(hash_series_method(data1), hash_series_method(data2));
}

TEST_F(HashSeriesMethodDeterminismTest, SmaMethodHashDeterministic)
{
  SmaMethod<CloseMethod> sma1{20};
  SmaMethod<CloseMethod> sma2{20};
  EXPECT_EQ(hash_series_method(sma1), hash_series_method(sma2));
}

TEST_F(HashSeriesMethodDeterminismTest, EmaMethodHashDeterministic)
{
  EmaMethod<CloseMethod> ema1{12};
  EmaMethod<CloseMethod> ema2{12};
  EXPECT_EQ(hash_series_method(ema1), hash_series_method(ema2));
}

TEST_F(HashSeriesMethodDeterminismTest, MultiplyMethodHashDeterministic)
{
  MultiplyMethod<SmaMethod<CloseMethod>, EmaMethod<CloseMethod>> mult1{
   SmaMethod<CloseMethod>{20}, EmaMethod<CloseMethod>{12}};
  MultiplyMethod<SmaMethod<CloseMethod>, EmaMethod<CloseMethod>> mult2{
   SmaMethod<CloseMethod>{20}, EmaMethod<CloseMethod>{12}};
  EXPECT_EQ(hash_series_method(mult1), hash_series_method(mult2));
}

// ============================================================================
// Hash Uniqueness Tests - Different inputs should produce different hashes
// ============================================================================

class HashSeriesMethodUniquenessTest : public ::testing::Test {};

TEST_F(HashSeriesMethodUniquenessTest, DifferentAtrPeriodsDifferentHash)
{
  AtrMethod atr14{14};
  AtrMethod atr21{21};
  EXPECT_NE(hash_series_method(atr14), hash_series_method(atr21));
}

TEST_F(HashSeriesMethodUniquenessTest, DifferentDataFieldsDifferentHash)
{
  DataMethod data1{"close"};
  DataMethod data2{"open"};
  EXPECT_NE(hash_series_method(data1), hash_series_method(data2));
}

TEST_F(HashSeriesMethodUniquenessTest, DifferentSmaPeriodsDifferentHash)
{
  SmaMethod<CloseMethod> sma1{10};
  SmaMethod<CloseMethod> sma2{20};
  EXPECT_NE(hash_series_method(sma1), hash_series_method(sma2));
}

TEST_F(HashSeriesMethodUniquenessTest, DifferentSmaSourcesDifferentHash)
{
  SmaMethod<OpenMethod> smaOpen{20};
  SmaMethod<CloseMethod> smaClose{20};
  EXPECT_NE(hash_series_method(smaOpen), hash_series_method(smaClose));
}

TEST_F(HashSeriesMethodUniquenessTest, DifferentEmaPeriodsDifferentHash)
{
  EmaMethod<CloseMethod> ema12{12};
  EmaMethod<CloseMethod> ema26{26};
  EXPECT_NE(hash_series_method(ema12), hash_series_method(ema26));
}

TEST_F(HashSeriesMethodUniquenessTest, DifferentOhlcvMethodsDifferentHash)
{
  OpenMethod open;
  HighMethod high;
  LowMethod low;
  CloseMethod close;
  VolumeMethod volume;

  const auto open_hash = hash_series_method(open);
  const auto high_hash = hash_series_method(high);
  const auto low_hash = hash_series_method(low);
  const auto close_hash = hash_series_method(close);
  const auto volume_hash = hash_series_method(volume);

  // All OHLCV methods should produce different hashes
  EXPECT_NE(open_hash, high_hash);
  EXPECT_NE(open_hash, low_hash);
  EXPECT_NE(open_hash, close_hash);
  EXPECT_NE(open_hash, volume_hash);
  EXPECT_NE(high_hash, low_hash);
  EXPECT_NE(high_hash, close_hash);
  EXPECT_NE(high_hash, volume_hash);
  EXPECT_NE(low_hash, close_hash);
  EXPECT_NE(low_hash, volume_hash);
  EXPECT_NE(close_hash, volume_hash);
}

// ============================================================================
// Simple Method Tests
// ============================================================================

class HashSeriesMethodSimpleTest : public ::testing::Test {};

TEST_F(HashSeriesMethodSimpleTest, AtrMethodHashProducesValidHash)
{
  AtrMethod atr{14};
  const auto hash = hash_series_method(atr);
  EXPECT_NE(hash, 0);
}

TEST_F(HashSeriesMethodSimpleTest, DataMethodHashProducesValidHash)
{
  DataMethod data{"close"};
  const auto hash = hash_series_method(data);
  EXPECT_NE(hash, 0);
}

TEST_F(HashSeriesMethodSimpleTest, RvolMethodHashProducesValidHash)
{
  RvolMethod rvol{14};
  const auto hash = hash_series_method(rvol);
  EXPECT_NE(hash, 0);
}

TEST_F(HashSeriesMethodSimpleTest, OhlcvMethodsProduceValidHashes)
{
  const auto open_hash = hash_series_method(OpenMethod{});
  const auto high_hash = hash_series_method(HighMethod{});
  const auto low_hash = hash_series_method(LowMethod{});
  const auto close_hash = hash_series_method(CloseMethod{});
  const auto volume_hash = hash_series_method(VolumeMethod{});

  EXPECT_NE(open_hash, 0);
  EXPECT_NE(high_hash, 0);
  EXPECT_NE(low_hash, 0);
  EXPECT_NE(close_hash, 0);
  EXPECT_NE(volume_hash, 0);
}

TEST_F(HashSeriesMethodSimpleTest, SeriesMethodHashProducesValidHash)
{
  SeriesMethod series{"my_series"};
  const auto hash = hash_series_method(series);
  EXPECT_NE(hash, 0);
}

// ============================================================================
// Templated Method Tests
// ============================================================================

class HashSeriesMethodTemplatedTest : public ::testing::Test {};

TEST_F(HashSeriesMethodTemplatedTest, SmaMethodWithDifferentSources)
{
  SmaMethod<OpenMethod> smaOpen{20};
  SmaMethod<HighMethod> smaHigh{20};
  SmaMethod<LowMethod> smaLow{20};
  SmaMethod<CloseMethod> smaClose{20};
  SmaMethod<VolumeMethod> smaVolume{20};

  const auto open_hash = hash_series_method(smaOpen);
  const auto high_hash = hash_series_method(smaHigh);
  const auto low_hash = hash_series_method(smaLow);
  const auto close_hash = hash_series_method(smaClose);
  const auto volume_hash = hash_series_method(smaVolume);

  // All different sources should produce different hashes
  EXPECT_NE(open_hash, high_hash);
  EXPECT_NE(open_hash, low_hash);
  EXPECT_NE(open_hash, close_hash);
  EXPECT_NE(open_hash, volume_hash);
}

TEST_F(HashSeriesMethodTemplatedTest, EmaMethodWithDifferentPeriods)
{
  EmaMethod<CloseMethod> ema5{5};
  EmaMethod<CloseMethod> ema12{12};
  EmaMethod<CloseMethod> ema26{26};

  const auto hash5 = hash_series_method(ema5);
  const auto hash12 = hash_series_method(ema12);
  const auto hash26 = hash_series_method(ema26);

  EXPECT_NE(hash5, hash12);
  EXPECT_NE(hash12, hash26);
  EXPECT_NE(hash5, hash26);
}

TEST_F(HashSeriesMethodTemplatedTest, RsiMethodWithDifferentSources)
{
  RsiMethod<OpenMethod> rsiOpen{14};
  RsiMethod<CloseMethod> rsiClose{14};

  const auto open_hash = hash_series_method(rsiOpen);
  const auto close_hash = hash_series_method(rsiClose);

  EXPECT_NE(open_hash, close_hash);
}

TEST_F(HashSeriesMethodTemplatedTest, ChangeMethodWithDifferentSources)
{
  ChangeMethod<OpenMethod> changeOpen;
  ChangeMethod<CloseMethod> changeClose;

  const auto open_hash = hash_series_method(changeOpen);
  const auto close_hash = hash_series_method(changeClose);

  EXPECT_NE(open_hash, close_hash);
}

// ============================================================================
// Complex Method Tests
// ============================================================================

class HashSeriesMethodComplexTest : public ::testing::Test {};

TEST_F(HashSeriesMethodComplexTest, BbMethodHashProducesValidHash)
{
  BbMethod<CloseMethod> bb{CloseMethod{}, 20, 2.0};
  const auto hash = hash_series_method(bb);
  EXPECT_NE(hash, 0);
}

TEST_F(HashSeriesMethodComplexTest, MacdMethodHashProducesValidHash)
{
  MacdMethod<CloseMethod> macd{CloseMethod{}, 12, 26, 9};
  const auto hash = hash_series_method(macd);
  EXPECT_NE(hash, 0);
}

TEST_F(HashSeriesMethodComplexTest, KcMethodHashProducesValidHash)
{
  KcMethod<CloseMethod> kc{};
  const auto hash = hash_series_method(kc);
  EXPECT_NE(hash, 0);
}

TEST_F(HashSeriesMethodComplexTest, PercentageMethodHashProducesValidHash)
{
  PercentageMethod<CloseMethod> pct{CloseMethod{}, 100.0};
  const auto hash = hash_series_method(pct);
  EXPECT_NE(hash, 0);
}

TEST_F(HashSeriesMethodComplexTest, LookbackMethodHashProducesValidHash)
{
  LookbackMethod<CloseMethod> lookback{CloseMethod{}, 5};
  const auto hash = hash_series_method(lookback);
  EXPECT_NE(hash, 0);
}

TEST_F(HashSeriesMethodComplexTest, HighestMethodHashProducesValidHash)
{
  HighestMethod<CloseMethod> highest{CloseMethod{}, 20};
  const auto hash = hash_series_method(highest);
  EXPECT_NE(hash, 0);
}

TEST_F(HashSeriesMethodComplexTest, LowestMethodHashProducesValidHash)
{
  LowestMethod<CloseMethod> lowest{CloseMethod{}, 20};
  const auto hash = hash_series_method(lowest);
  EXPECT_NE(hash, 0);
}

TEST_F(HashSeriesMethodComplexTest, MacdMethodsDifferentPeriodsDifferentHash)
{
  MacdMethod<CloseMethod> macd1{CloseMethod{}, 12, 26, 9};
  MacdMethod<CloseMethod> macd2{CloseMethod{}, 5, 35, 5};

  const auto hash1 = hash_series_method(macd1);
  const auto hash2 = hash_series_method(macd2);

  EXPECT_NE(hash1, hash2);
}

TEST_F(HashSeriesMethodComplexTest, BbMethodsDifferentParametersDifferentHash)
{
  BbMethod<CloseMethod> bb1{CloseMethod{}, 20, 2.0};
  BbMethod<CloseMethod> bb2{CloseMethod{}, 20, 3.0};
  BbMethod<CloseMethod> bb3{CloseMethod{}, 10, 2.0};

  const auto hash1 = hash_series_method(bb1);
  const auto hash2 = hash_series_method(bb2);
  const auto hash3 = hash_series_method(bb3);

  EXPECT_NE(hash1, hash2); // Different stddev
  EXPECT_NE(hash1, hash3); // Different period
}

// ============================================================================
// Operator Method Tests
// ============================================================================

class HashSeriesMethodOperatorTest : public ::testing::Test {};

TEST_F(HashSeriesMethodOperatorTest, MultiplyMethodHashProducesValidHash)
{
  MultiplyMethod<SmaMethod<CloseMethod>, EmaMethod<CloseMethod>> mult{
   SmaMethod<CloseMethod>{20}, EmaMethod<CloseMethod>{12}};
  const auto hash = hash_series_method(mult);
  EXPECT_NE(hash, 0);
}

TEST_F(HashSeriesMethodOperatorTest, DivideMethodHashProducesValidHash)
{
  DivideMethod<SmaMethod<CloseMethod>, EmaMethod<CloseMethod>> div{
   SmaMethod<CloseMethod>{20}, EmaMethod<CloseMethod>{12}};
  const auto hash = hash_series_method(div);
  EXPECT_NE(hash, 0);
}

TEST_F(HashSeriesMethodOperatorTest, AddMethodHashProducesValidHash)
{
  AddMethod<SmaMethod<CloseMethod>, EmaMethod<CloseMethod>> add{
   SmaMethod<CloseMethod>{20}, EmaMethod<CloseMethod>{12}};
  const auto hash = hash_series_method(add);
  EXPECT_NE(hash, 0);
}

TEST_F(HashSeriesMethodOperatorTest, SubtractMethodHashProducesValidHash)
{
  SubtractMethod<SmaMethod<CloseMethod>, EmaMethod<CloseMethod>> sub{
   SmaMethod<CloseMethod>{20}, EmaMethod<CloseMethod>{12}};
  const auto hash = hash_series_method(sub);
  EXPECT_NE(hash, 0);
}

TEST_F(HashSeriesMethodOperatorTest, NegateMethodHashProducesValidHash)
{
  NegateMethod<SmaMethod<CloseMethod>> negate{SmaMethod<CloseMethod>{20}};
  const auto hash = hash_series_method(negate);
  EXPECT_NE(hash, 0);
}

TEST_F(HashSeriesMethodOperatorTest, AbsMethodHashProducesValidHash)
{
  AbsMethod<SmaMethod<CloseMethod>> abs{SmaMethod<CloseMethod>{20}};
  const auto hash = hash_series_method(abs);
  EXPECT_NE(hash, 0);
}

TEST_F(HashSeriesMethodOperatorTest, AbsDiffMethodHashProducesValidHash)
{
  AbsDiffMethod<SmaMethod<CloseMethod>, EmaMethod<CloseMethod>> absdiff{
   SmaMethod<CloseMethod>{20}, EmaMethod<CloseMethod>{12}};
  const auto hash = hash_series_method(absdiff);
  EXPECT_NE(hash, 0);
}

TEST_F(HashSeriesMethodOperatorTest, SqrtMethodHashProducesValidHash)
{
  SqrtMethod<SmaMethod<CloseMethod>> sqrt{SmaMethod<CloseMethod>{20}};
  const auto hash = hash_series_method(sqrt);
  EXPECT_NE(hash, 0);
}

TEST_F(HashSeriesMethodOperatorTest, MaxMethodHashProducesValidHash)
{
  MaxMethod<SmaMethod<CloseMethod>, EmaMethod<CloseMethod>> max{
   SmaMethod<CloseMethod>{20}, EmaMethod<CloseMethod>{12}};
  const auto hash = hash_series_method(max);
  EXPECT_NE(hash, 0);
}

TEST_F(HashSeriesMethodOperatorTest, MinMethodHashProducesValidHash)
{
  MinMethod<SmaMethod<CloseMethod>, EmaMethod<CloseMethod>> min{
   SmaMethod<CloseMethod>{20}, EmaMethod<CloseMethod>{12}};
  const auto hash = hash_series_method(min);
  EXPECT_NE(hash, 0);
}

TEST_F(HashSeriesMethodOperatorTest, PositivePartMethodHashProducesValidHash)
{
  PositivePartMethod<SmaMethod<CloseMethod>> pospart{
   SmaMethod<CloseMethod>{20}};
  const auto hash = hash_series_method(pospart);
  EXPECT_NE(hash, 0);
}

TEST_F(HashSeriesMethodOperatorTest, NegativePartMethodHashProducesValidHash)
{
  NegativePartMethod<SmaMethod<CloseMethod>> negpart{
   SmaMethod<CloseMethod>{20}};
  const auto hash = hash_series_method(negpart);
  EXPECT_NE(hash, 0);
}

TEST_F(HashSeriesMethodOperatorTest,
       OperatorMethodsDifferentOperandsDifferentHash)
{
  MultiplyMethod<SmaMethod<CloseMethod>, EmaMethod<CloseMethod>> mult1{
   SmaMethod<CloseMethod>{20}, EmaMethod<CloseMethod>{12}};
  MultiplyMethod<SmaMethod<CloseMethod>, EmaMethod<CloseMethod>> mult2{
   SmaMethod<CloseMethod>{10}, EmaMethod<CloseMethod>{12}};

  const auto hash1 = hash_series_method(mult1);
  const auto hash2 = hash_series_method(mult2);

  EXPECT_NE(hash1, hash2);
}

// ============================================================================
// Stochastic Method Tests
// ============================================================================

class HashSeriesMethodStochasticTest : public ::testing::Test {};

TEST_F(HashSeriesMethodStochasticTest, StochMethodHashProducesValidHash)
{
  StochMethod stoch{14, 3, 3};
  const auto hash = hash_series_method(stoch);
  EXPECT_NE(hash, 0);
}

TEST_F(HashSeriesMethodStochasticTest,
       StochMethodDifferentParametersDifferentHash)
{
  StochMethod stoch1{14, 3, 3};
  StochMethod stoch2{21, 3, 3};
  StochMethod stoch3{14, 5, 3};

  const auto hash1 = hash_series_method(stoch1);
  const auto hash2 = hash_series_method(stoch2);
  const auto hash3 = hash_series_method(stoch3);

  EXPECT_NE(hash1, hash2);
  EXPECT_NE(hash1, hash3);
}

TEST_F(HashSeriesMethodStochasticTest, StochRsiMethodHashProducesValidHash)
{
  StochRsiMethod<CloseMethod> stochrsi{CloseMethod{}, 14, 14, 3, 3};
  const auto hash = hash_series_method(stochrsi);
  EXPECT_NE(hash, 0);
}

TEST_F(HashSeriesMethodStochasticTest, StochRsiMethodDifferentRsiDifferentHash)
{
  StochRsiMethod<CloseMethod> stochrsi1{CloseMethod{}, 14, 14, 3, 3};
  StochRsiMethod<CloseMethod> stochrsi2{CloseMethod{}, 21, 14, 3, 3};

  const auto hash1 = hash_series_method(stochrsi1);
  const auto hash2 = hash_series_method(stochrsi2);

  EXPECT_NE(hash1, hash2);
}

// ============================================================================
// MA Method Tests
// ============================================================================

class HashSeriesMethodMaTest : public ::testing::Test {};

TEST_F(HashSeriesMethodMaTest, RmaMethodHashProducesValidHash)
{
  RmaMethod<CloseMethod> rma{20};
  const auto hash = hash_series_method(rma);
  EXPECT_NE(hash, 0);
}

TEST_F(HashSeriesMethodMaTest, WmaMethodHashProducesValidHash)
{
  WmaMethod<CloseMethod> wma{20};
  const auto hash = hash_series_method(wma);
  EXPECT_NE(hash, 0);
}

TEST_F(HashSeriesMethodMaTest, HmaMethodHashProducesValidHash)
{
  HmaMethod<CloseMethod> hma{20};
  const auto hash = hash_series_method(hma);
  EXPECT_NE(hash, 0);
}

TEST_F(HashSeriesMethodMaTest, AdaptiveMaMethodHashProducesValidHash)
{
  AdaptiveMaMethod<CloseMethod> ama{CloseMethod{}, 34};
  const auto hash = hash_series_method(ama);
  EXPECT_NE(hash, 0);
}

TEST_F(HashSeriesMethodMaTest, MaMethodsDifferentPeriodsDifferentHash)
{
  SmaMethod<CloseMethod> sma10{10};
  SmaMethod<CloseMethod> sma20{20};
  EmaMethod<CloseMethod> ema10{10};
  RmaMethod<CloseMethod> rma10{10};
  WmaMethod<CloseMethod> wma10{10};

  const auto sma10_hash = hash_series_method(sma10);
  const auto sma20_hash = hash_series_method(sma20);
  const auto ema10_hash = hash_series_method(ema10);
  const auto rma10_hash = hash_series_method(rma10);
  const auto wma10_hash = hash_series_method(wma10);

  // Different periods should produce different hashes
  EXPECT_NE(sma10_hash, sma20_hash);

  // Different types with same period should produce different hashes
  EXPECT_NE(sma10_hash, ema10_hash);
  EXPECT_NE(sma10_hash, rma10_hash);
  EXPECT_NE(sma10_hash, wma10_hash);
}

// ============================================================================
// Stddev and TR Method Tests
// ============================================================================

class HashSeriesMethodOtherTest : public ::testing::Test {};

TEST_F(HashSeriesMethodOtherTest, StddevMethodHashProducesValidHash)
{
  StddevMethod<CloseMethod> stddev{20};
  const auto hash = hash_series_method(stddev);
  EXPECT_NE(hash, 0);
}

TEST_F(HashSeriesMethodOtherTest, ValueMethodHashProducesValidHash)
{
  ValueMethod value{3.14};
  const auto hash = hash_series_method(value);
  EXPECT_NE(hash, 0);
}

TEST_F(HashSeriesMethodOtherTest, SelectOutputMethodHashProducesValidHash)
{
  SelectOutputMethod<MacdMethod<CloseMethod>> select{
   MacdMethod<CloseMethod>{CloseMethod{}, 12, 26, 9}, MethodOutput::MacdLine};
  const auto hash = hash_series_method(select);
  EXPECT_NE(hash, 0);
}

TEST_F(HashSeriesMethodOtherTest, RocMethodHashProducesValidHash)
{
  RocMethod<CloseMethod> roc{12};
  const auto hash = hash_series_method(roc);
  EXPECT_NE(hash, 0);
}

// ============================================================================
// Hash Collision Resistance Tests
// ============================================================================

class HashSeriesMethodCollisionTest : public ::testing::Test {};

TEST_F(HashSeriesMethodCollisionTest,
       LargeParameterDifferencesProduceDifferentHash)
{
  AtrMethod atr1{1};
  AtrMethod atr2{1000};
  EXPECT_NE(hash_series_method(atr1), hash_series_method(atr2));
}

TEST_F(HashSeriesMethodCollisionTest,
       SmallParameterDifferencesProduceDifferentHash)
{
  SmaMethod<CloseMethod> sma1{19};
  SmaMethod<CloseMethod> sma2{20};
  EXPECT_NE(hash_series_method(sma1), hash_series_method(sma2));
}

TEST_F(HashSeriesMethodCollisionTest,
       NestedMethodsDifferentSourcesProduceDifferentHash)
{
  SmaMethod<SmaMethod<CloseMethod>> nested1{SmaMethod<CloseMethod>{10}, 20};
  SmaMethod<SmaMethod<OpenMethod>> nested2{SmaMethod<OpenMethod>{10}, 20};
  EXPECT_NE(hash_series_method(nested1), hash_series_method(nested2));
}

TEST_F(HashSeriesMethodCollisionTest, ComplexExpressionHashProducesValidHash)
{
  // Hash a complex composed method: ((SMA * EMA) + (RSI / CLOSE)) * VOLUME
  auto sma = SmaMethod<CloseMethod>{20};
  auto ema = EmaMethod<CloseMethod>{12};
  auto rsi = RsiMethod<CloseMethod>{14};
  auto close = CloseMethod{};
  auto volume = VolumeMethod{};

  MultiplyMethod<SmaMethod<CloseMethod>, EmaMethod<CloseMethod>> mult1{sma,
                                                                       ema};
  DivideMethod<RsiMethod<CloseMethod>, CloseMethod> div{rsi, close};
  AddMethod<MultiplyMethod<SmaMethod<CloseMethod>, EmaMethod<CloseMethod>>,
            DivideMethod<RsiMethod<CloseMethod>, CloseMethod>>
   add{mult1, div};

  const auto hash = hash_series_method(add);
  EXPECT_NE(hash, 0);
}

// ============================================================================
// Edge Case Tests
// ============================================================================

class HashSeriesMethodEdgeCaseTest : public ::testing::Test {};

TEST_F(HashSeriesMethodEdgeCaseTest, EmptyDataMethodHashProducesValidHash)
{
  DataMethod empty;
  const auto hash = hash_series_method(empty);
  EXPECT_NE(hash, 0);
}

TEST_F(HashSeriesMethodEdgeCaseTest,
       DataMethodWithEmptyStringDifferentFromNonEmpty)
{
  DataMethod empty;
  DataMethod named{"data"};
  EXPECT_NE(hash_series_method(empty), hash_series_method(named));
}

TEST_F(HashSeriesMethodEdgeCaseTest, ValueMethodWithZeroDifferentFromNonZero)
{
  ValueMethod zero{0.0};
  ValueMethod nonzero{1.0};
  EXPECT_NE(hash_series_method(zero), hash_series_method(nonzero));
}

TEST_F(HashSeriesMethodEdgeCaseTest,
       ValueMethodWithNegativeDifferentFromPositive)
{
  ValueMethod positive{5.0};
  ValueMethod negative{-5.0};
  EXPECT_NE(hash_series_method(positive), hash_series_method(negative));
}

TEST_F(HashSeriesMethodEdgeCaseTest,
       SeriesMethodWithDifferentNamesDifferentHash)
{
  SeriesMethod series1{"series_a"};
  SeriesMethod series2{"series_b"};
  EXPECT_NE(hash_series_method(series1), hash_series_method(series2));
}

// ============================================================================
// Hash Stability Tests - Multiple runs should produce consistent results
// ============================================================================

class HashSeriesMethodStabilityTest : public ::testing::Test {};

TEST_F(HashSeriesMethodStabilityTest, MultipleHashCallsAreConsistent)
{
  AtrMethod atr{14};
  const auto hash1 = hash_series_method(atr);
  const auto hash2 = hash_series_method(atr);
  const auto hash3 = hash_series_method(atr);
  EXPECT_EQ(hash1, hash2);
  EXPECT_EQ(hash2, hash3);
}

TEST_F(HashSeriesMethodStabilityTest, ComplexMethodHashesAreStable)
{
  MacdMethod<SmaMethod<CloseMethod>> macd{SmaMethod<CloseMethod>{5}, 12, 26, 9};
  const auto hash1 = hash_series_method(macd);
  const auto hash2 = hash_series_method(macd);
  const auto hash3 = hash_series_method(macd);
  EXPECT_EQ(hash1, hash2);
  EXPECT_EQ(hash2, hash3);
}

TEST_F(HashSeriesMethodStabilityTest, OperatorMethodHashesAreStable)
{
  MultiplyMethod<SmaMethod<CloseMethod>, EmaMethod<CloseMethod>> mult{
   SmaMethod<CloseMethod>{20}, EmaMethod<CloseMethod>{12}};
  const auto hash1 = hash_series_method(mult);
  const auto hash2 = hash_series_method(mult);
  EXPECT_EQ(hash1, hash2);
}

// ============================================================================
// Hash AnySeriesMethod Tests - Should produce valid hashes for any method type
// ============================================================================

class HashSeriesMethodAnyTest : public ::testing::Test {};

TEST_F(HashSeriesMethodAnyTest, HashAnySeriesMethodProducesValidHash)
{
  const auto method =
   MultiplyMethod{SmaMethod<CloseMethod>{20}, EmaMethod<CloseMethod>{12}};

  const auto any_method =
   AnySeriesMethod{MultiplyMethod{AnySeriesMethod{SmaMethod<CloseMethod>{20}},
                                  AnySeriesMethod{EmaMethod<CloseMethod>{12}}}};

  const auto hash1 = hash_series_method(method);
  const auto hash2 = hash_series_method(any_method);

  EXPECT_EQ(hash1, hash2);
}
