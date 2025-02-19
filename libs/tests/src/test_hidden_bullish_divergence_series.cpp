#include <gtest/gtest.h>
#include <pludux/asset_history.hpp>
#include <pludux/series.hpp>

using namespace pludux;

TEST(HiddenBullishDivergenceSeriesTest, Constructor)
{
  const auto signal = DataSeries{"signal"};
  const auto reference = DataSeries{"reference"};
  const auto series = HiddenBullishDivergenceSeries{signal, reference};

  EXPECT_EQ(series.pivot_range(), 5);
  EXPECT_EQ(series.lookback_range(), 60);
}

TEST(HiddenBullishDivergenceSeriesTest, SignalUpReferenceDown)
{
  const auto asset_data = AssetHistory{
   {"signal", {9, 9, 9, 9, 9, 4, 9, 9, 9, 9, 9, 3, 9, 9, 9, 9, 9}},
   {"reference", {9, 9, 9, 9, 9, 2, 9, 9, 9, 9, 9, 3, 9, 9, 9, 9, 9}}};

  const auto signal = asset_data["signal"];
  const auto reference = asset_data["reference"];
  const auto series = HiddenBullishDivergenceSeries{signal, reference};

  EXPECT_EQ(series[0], -1);
  EXPECT_EQ(series[1], -1);
  EXPECT_EQ(series[2], -1);
  EXPECT_EQ(series[3], -1);
  EXPECT_EQ(series[4], -1);
  EXPECT_EQ(series[5], -1);
  EXPECT_EQ(series[6], -1);
  EXPECT_EQ(series[7], -1);
  EXPECT_EQ(series[8], -1);
  EXPECT_EQ(series[9], -1);
  EXPECT_EQ(series[10], -1);
  EXPECT_EQ(series[11], -1);
  EXPECT_EQ(series[12], -1);
  EXPECT_EQ(series[13], -1);
  EXPECT_EQ(series[14], -1);
  EXPECT_EQ(series[15], -1);
  EXPECT_EQ(series[16], -1);
}

TEST(HiddenBullishDivergenceSeriesTest, SignalUpReferenceFlat)
{
  const auto asset_data = AssetHistory{
   {"signal", {9, 9, 9, 9, 9, 4, 9, 9, 9, 9, 9, 3, 9, 9, 9, 9, 9}},
   {"reference", {9, 9, 9, 9, 9, 2, 9, 9, 9, 9, 9, 2, 9, 9, 9, 9, 9}}};

  const auto signal = asset_data["signal"];
  const auto reference = asset_data["reference"];
  const auto series = HiddenBullishDivergenceSeries{signal, reference};

  EXPECT_EQ(series[0], -1);
  EXPECT_EQ(series[1], -1);
  EXPECT_EQ(series[2], -1);
  EXPECT_EQ(series[3], -1);
  EXPECT_EQ(series[4], -1);
  EXPECT_EQ(series[5], -1);
  EXPECT_EQ(series[6], -1);
  EXPECT_EQ(series[7], -1);
  EXPECT_EQ(series[8], -1);
  EXPECT_EQ(series[9], -1);
  EXPECT_EQ(series[10], -1);
  EXPECT_EQ(series[11], -1);
  EXPECT_EQ(series[12], -1);
  EXPECT_EQ(series[13], -1);
  EXPECT_EQ(series[14], -1);
  EXPECT_EQ(series[15], -1);
  EXPECT_EQ(series[16], -1);
}

TEST(HiddenBullishDivergenceSeriesTest, SignalUpReferenceUp)
{
  const auto asset_data = AssetHistory{
   {"signal", {9, 9, 9, 9, 9, 4, 9, 9, 9, 9, 9, 3, 9, 9, 9, 9, 9}},
   {"reference", {9, 9, 9, 9, 9, 3, 9, 9, 9, 9, 9, 2, 9, 9, 9, 9, 9}}};

  const auto signal = asset_data["signal"];
  const auto reference = asset_data["reference"];
  const auto series = HiddenBullishDivergenceSeries{signal, reference};

  EXPECT_EQ(series[0], -1);
  EXPECT_EQ(series[1], -1);
  EXPECT_EQ(series[2], -1);
  EXPECT_EQ(series[3], -1);
  EXPECT_EQ(series[4], -1);
  EXPECT_EQ(series[5], -1);
  EXPECT_EQ(series[6], -1);
  EXPECT_EQ(series[7], -1);
  EXPECT_EQ(series[8], -1);
  EXPECT_EQ(series[9], -1);
  EXPECT_EQ(series[10], -1);
  EXPECT_EQ(series[11], -1);
  EXPECT_EQ(series[12], -1);
  EXPECT_EQ(series[13], -1);
  EXPECT_EQ(series[14], -1);
  EXPECT_EQ(series[15], -1);
  EXPECT_EQ(series[16], -1);
}

TEST(HiddenBullishDivergenceSeriesTest, SignalDownReferenceUp)
{
  const auto asset_data = AssetHistory{
   {"signal", {9, 9, 9, 9, 9, 3, 9, 9, 9, 9, 9, 4, 9, 9, 9, 9, 9}},
   {"reference", {9, 9, 9, 9, 9, 3, 9, 9, 9, 9, 9, 2, 9, 9, 9, 9, 9}}};

  const auto signal = asset_data["signal"];
  const auto reference = asset_data["reference"];
  const auto series = HiddenBullishDivergenceSeries{signal, reference};

  EXPECT_EQ(series[0], -1);
  EXPECT_EQ(series[1], -1);
  EXPECT_EQ(series[2], -1);
  EXPECT_EQ(series[3], -1);
  EXPECT_EQ(series[4], -1);
  EXPECT_EQ(series[5], 11);
  EXPECT_EQ(series[6], -1);
  EXPECT_EQ(series[7], -1);
  EXPECT_EQ(series[8], -1);
  EXPECT_EQ(series[9], -1);
  EXPECT_EQ(series[10], -1);
  EXPECT_EQ(series[11], -1);
  EXPECT_EQ(series[12], -1);
  EXPECT_EQ(series[13], -1);
  EXPECT_EQ(series[14], -1);
  EXPECT_EQ(series[15], -1);
  EXPECT_EQ(series[16], -1);
}

TEST(HiddenBullishDivergenceSeriesTest, SignalDownReferenceUpNotPivotLow)
{
  const auto asset_data = AssetHistory{
   {"signal", {9, 9, 9, 9, 9, 3, 9, 9, 9, 9, 9, 4, 9, 9, 9, 9, 9}},
   {"reference", {9, 9, 9, 9, 9, 3, 9, 9, 9, 9, 9, 2, 1, 9, 9, 9, 9}}};
  const auto signal = asset_data["signal"];
  const auto reference = asset_data["reference"];
  const auto series = HiddenBullishDivergenceSeries{signal, reference};

  EXPECT_EQ(series[0], -1);
  EXPECT_EQ(series[1], -1);
  EXPECT_EQ(series[2], -1);
  EXPECT_EQ(series[3], -1);
  EXPECT_EQ(series[4], -1);
  EXPECT_EQ(series[5], 11);
  EXPECT_EQ(series[6], -1);
  EXPECT_EQ(series[7], -1);
  EXPECT_EQ(series[8], -1);
  EXPECT_EQ(series[9], -1);
  EXPECT_EQ(series[10], -1);
  EXPECT_EQ(series[11], -1);
  EXPECT_EQ(series[12], -1);
  EXPECT_EQ(series[13], -1);
  EXPECT_EQ(series[14], -1);
  EXPECT_EQ(series[15], -1);
  EXPECT_EQ(series[16], -1);
}

TEST(HiddenBullishDivergenceSeriesTest, MultipleSignalDownReferenceUp)
{
  const auto asset_data = AssetHistory{
   {"signal",    {1, 9, 9, 9, 9, 9, 3, 9, 9, 9, 4, 9, 9, 9, 9, 9, 5}},
   {"reference", {5, 9, 9, 4, 9, 9, 3, 9, 9, 9, 2, 9, 9, 9, 9, 9, 1}}};

  const auto signal = asset_data["signal"];
  const auto reference = asset_data["reference"];
  const auto series = HiddenBullishDivergenceSeries{signal, reference, 2};

  EXPECT_EQ(series[0], 6);
  EXPECT_EQ(series[1], -1);
  EXPECT_EQ(series[2], -1);
  EXPECT_EQ(series[3], -1);
  EXPECT_EQ(series[4], -1);
  EXPECT_EQ(series[5], -1);
  EXPECT_EQ(series[6], 10);
  EXPECT_EQ(series[7], -1);
  EXPECT_EQ(series[8], -1);
  EXPECT_EQ(series[9], -1);
  EXPECT_EQ(series[10], 16);
  EXPECT_EQ(series[11], -1);
  EXPECT_EQ(series[12], -1);
  EXPECT_EQ(series[13], -1);
  EXPECT_EQ(series[14], -1);
  EXPECT_EQ(series[15], -1);
  EXPECT_EQ(series[16], -1);
}

TEST(HiddenBullishDivergenceSeriesTest, SignalDownReferenceFlat)
{
  const auto asset_data = AssetHistory{
   {"signal", {9, 9, 9, 9, 9, 3, 9, 9, 9, 9, 9, 4, 9, 9, 9, 9, 9}},
   {"reference", {9, 9, 9, 9, 9, 3, 9, 9, 9, 9, 9, 3, 9, 9, 9, 9, 9}}};
  const auto signal = asset_data["signal"];
  const auto reference = asset_data["reference"];
  const auto series = HiddenBullishDivergenceSeries{signal, reference};

  EXPECT_EQ(series[0], -1);
  EXPECT_EQ(series[1], -1);
  EXPECT_EQ(series[2], -1);
  EXPECT_EQ(series[3], -1);
  EXPECT_EQ(series[4], -1);
  EXPECT_EQ(series[5], -1);
  EXPECT_EQ(series[6], -1);
  EXPECT_EQ(series[7], -1);
  EXPECT_EQ(series[8], -1);
  EXPECT_EQ(series[9], -1);
  EXPECT_EQ(series[10], -1);
  EXPECT_EQ(series[11], -1);
  EXPECT_EQ(series[12], -1);
  EXPECT_EQ(series[13], -1);
  EXPECT_EQ(series[14], -1);
  EXPECT_EQ(series[15], -1);
  EXPECT_EQ(series[16], -1);
}

TEST(HiddenBullishDivergenceSeriesTest, SignalDownReferenceDown)
{
  const auto asset_data = AssetHistory{
   {"signal", {9, 9, 9, 9, 9, 3, 9, 9, 9, 9, 9, 4, 9, 9, 9, 9, 9}},
   {"reference", {9, 9, 9, 9, 9, 3, 9, 9, 9, 9, 9, 4, 9, 9, 9, 9, 9}}};
  const auto signal = asset_data["signal"];
  const auto reference = asset_data["reference"];
  const auto series = HiddenBullishDivergenceSeries{signal, reference};

  EXPECT_EQ(series[0], -1);
  EXPECT_EQ(series[1], -1);
  EXPECT_EQ(series[2], -1);
  EXPECT_EQ(series[3], -1);
  EXPECT_EQ(series[4], -1);
  EXPECT_EQ(series[5], -1);
  EXPECT_EQ(series[6], -1);
  EXPECT_EQ(series[7], -1);
  EXPECT_EQ(series[8], -1);
  EXPECT_EQ(series[9], -1);
  EXPECT_EQ(series[10], -1);
  EXPECT_EQ(series[11], -1);
  EXPECT_EQ(series[12], -1);
  EXPECT_EQ(series[13], -1);
  EXPECT_EQ(series[14], -1);
  EXPECT_EQ(series[15], -1);
  EXPECT_EQ(series[16], -1);
}

TEST(HiddenBullishDivergenceSeriesTest, SignalFlatReferenceUp)
{
  const auto asset_data = AssetHistory{
   {"signal", {9, 9, 9, 9, 9, 3, 9, 9, 9, 9, 9, 3, 9, 9, 9, 9, 9}},
   {"reference", {9, 9, 9, 9, 9, 3, 9, 9, 9, 9, 9, 2, 9, 9, 9, 9, 9}}};
  const auto signal = asset_data["signal"];
  const auto reference = asset_data["reference"];
  const auto series = HiddenBullishDivergenceSeries{signal, reference};

  EXPECT_EQ(series[0], -1);
  EXPECT_EQ(series[1], -1);
  EXPECT_EQ(series[2], -1);
  EXPECT_EQ(series[3], -1);
  EXPECT_EQ(series[4], -1);
  EXPECT_EQ(series[5], -1);
  EXPECT_EQ(series[6], -1);
  EXPECT_EQ(series[7], -1);
  EXPECT_EQ(series[8], -1);
  EXPECT_EQ(series[9], -1);
  EXPECT_EQ(series[10], -1);
  EXPECT_EQ(series[11], -1);
  EXPECT_EQ(series[12], -1);
  EXPECT_EQ(series[13], -1);
  EXPECT_EQ(series[14], -1);
  EXPECT_EQ(series[15], -1);
  EXPECT_EQ(series[16], -1);
}

TEST(HiddenBullishDivergenceSeriesTest, SignalFlatReferenceFlat)
{
  const auto asset_data = AssetHistory{
   {"signal", {9, 9, 9, 9, 9, 3, 9, 9, 9, 9, 9, 3, 9, 9, 9, 9, 9}},
   {"reference", {9, 9, 9, 9, 9, 3, 9, 9, 9, 9, 9, 3, 9, 9, 9, 9, 9}}};
  const auto signal = asset_data["signal"];
  const auto reference = asset_data["reference"];
  const auto series = HiddenBullishDivergenceSeries{signal, reference};

  EXPECT_EQ(series[0], -1);
  EXPECT_EQ(series[1], -1);
  EXPECT_EQ(series[2], -1);
  EXPECT_EQ(series[3], -1);
  EXPECT_EQ(series[4], -1);
  EXPECT_EQ(series[5], -1);
  EXPECT_EQ(series[6], -1);
  EXPECT_EQ(series[7], -1);
  EXPECT_EQ(series[8], -1);
  EXPECT_EQ(series[9], -1);
  EXPECT_EQ(series[10], -1);
  EXPECT_EQ(series[11], -1);
  EXPECT_EQ(series[12], -1);
  EXPECT_EQ(series[13], -1);
  EXPECT_EQ(series[14], -1);
  EXPECT_EQ(series[15], -1);
  EXPECT_EQ(series[16], -1);
}
