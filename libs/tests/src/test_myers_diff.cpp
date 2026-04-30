#include <gtest/gtest.h>

#include <memory>
#include <random>
#include <string>
#include <vector>

import pludux;

using namespace pludux;

template<typename T>
void check_basic_invariants(const std::vector<T>& old_seq,
                            const std::vector<T>& new_seq,
                            const Patch<T>& patch)
{
  const auto rebuilt = patch.apply(old_seq);

  // 1. Must reconstruct correctly
  ASSERT_EQ(rebuilt, new_seq);

  // 2. Size invariant
  EXPECT_EQ(rebuilt.size(),
            old_seq.size() - patch.deleted_count() + patch.inserted_count());

  // 3. Copy + delete must consume old sequence
  EXPECT_EQ(patch.copied_count() + patch.deleted_count(), old_seq.size());

  // 4. No zero-length operations
  for(const auto& op : patch.operations()) {
    if(const auto* c = std::get_if<typename Patch<T>::Copy>(&op)) {
      EXPECT_GT(c->count, 0u);
    }
    if(const auto* d = std::get_if<typename Patch<T>::Delete>(&op)) {
      EXPECT_GT(d->count, 0u);
    }
    if(const auto* i = std::get_if<typename Patch<T>::Insert>(&op)) {
      EXPECT_FALSE(i->values.empty());
    }
  }

  // 5. No adjacent same-type ops (coalesced)
  const auto& ops = patch.operations();
  for(std::size_t i = 1; i < ops.size(); ++i) {
    EXPECT_NE(ops[i - 1].index(), ops[i].index());
  }
}

template<typename T>
std::vector<T> collect_inserted_values(const Patch<T>& patch)
{
  std::vector<T> result;

  for(const auto& op : patch.operations()) {
    if(const auto* ins = std::get_if<typename Patch<T>::Insert>(&op)) {
      result.insert(result.end(), ins->values.begin(), ins->values.end());
    }
  }

  return result;
}

template<typename T>
std::vector<std::size_t> collect_copy_counts(const Patch<T>& patch)
{
  std::vector<std::size_t> result;

  for(const auto& op : patch.operations()) {
    if(const auto* copy = std::get_if<typename Patch<T>::Copy>(&op)) {
      result.push_back(copy->count);
    }
  }

  return result;
}

template<typename T>
std::vector<std::size_t> collect_delete_counts(const Patch<T>& patch)
{
  std::vector<std::size_t> result;

  for(const auto& op : patch.operations()) {
    if(const auto* del = std::get_if<typename Patch<T>::Delete>(&op)) {
      result.push_back(del->count);
    }
  }

  return result;
}

TEST(MyersDiffTest, StringBasicExample)
{
  const std::string old_text = "ABCABBA";
  const std::string new_text = "CBABAC";

  const auto patch = diff(old_text, new_text);
  const auto rebuilt = patch.apply(old_text);

  EXPECT_EQ(rebuilt, new_text);
  EXPECT_EQ(patch.copied_count(), 4u);
  EXPECT_EQ(patch.deleted_count(), 3u);
  EXPECT_EQ(patch.inserted_count(), 2u);
}

TEST(MyersDiffTest, IdenticalStringProducesCopyOnly)
{
  const std::string old_text = "HELLO";
  const std::string new_text = "HELLO";

  const auto patch = diff(old_text, new_text);
  const auto rebuilt = patch.apply(old_text);

  EXPECT_EQ(rebuilt, new_text);
  EXPECT_EQ(patch.copied_count(), 5u);
  EXPECT_EQ(patch.deleted_count(), 0u);
  EXPECT_EQ(patch.inserted_count(), 0u);

  ASSERT_EQ(patch.operation_count(), 1u);
  ASSERT_EQ(collect_copy_counts(patch).size(), 1u);
  EXPECT_EQ(collect_copy_counts(patch)[0], 5u);
}

TEST(MyersDiffTest, EmptyToEmpty)
{
  const std::string old_text;
  const std::string new_text;

  const auto patch = diff(old_text, new_text);
  const auto rebuilt = patch.apply(old_text);

  EXPECT_EQ(rebuilt, new_text);
  EXPECT_TRUE(patch.empty());
  EXPECT_EQ(patch.operation_count(), 0u);
  EXPECT_EQ(patch.copied_count(), 0u);
  EXPECT_EQ(patch.deleted_count(), 0u);
  EXPECT_EQ(patch.inserted_count(), 0u);
}

TEST(MyersDiffTest, EmptyOldMeansInsertAll)
{
  const std::string old_text;
  const std::string new_text = "WORLD";

  const auto patch = diff(old_text, new_text);
  const auto rebuilt = patch.apply(old_text);

  EXPECT_EQ(rebuilt, new_text);
  EXPECT_EQ(patch.copied_count(), 0u);
  EXPECT_EQ(patch.deleted_count(), 0u);
  EXPECT_EQ(patch.inserted_count(), 5u);

  ASSERT_EQ(patch.operation_count(), 1u);
  EXPECT_EQ(collect_inserted_values(patch),
            std::vector<char>({'W', 'O', 'R', 'L', 'D'}));
}

TEST(MyersDiffTest, EmptyNewMeansDeleteAll)
{
  const std::string old_text = "WORLD";
  const std::string new_text;

  const auto patch = diff(old_text, new_text);
  const auto rebuilt = patch.apply(old_text);

  EXPECT_EQ(rebuilt, new_text);
  EXPECT_EQ(patch.copied_count(), 0u);
  EXPECT_EQ(patch.deleted_count(), 5u);
  EXPECT_EQ(patch.inserted_count(), 0u);

  ASSERT_EQ(patch.operation_count(), 1u);
  ASSERT_EQ(collect_delete_counts(patch).size(), 1u);
  EXPECT_EQ(collect_delete_counts(patch)[0], 5u);
}

TEST(MyersDiffTest, VectorIntBasic)
{
  const std::vector<int> old_seq{1, 2, 3, 4};
  const std::vector<int> new_seq{1, 3, 4, 5};

  const auto patch = diff(old_seq, new_seq);
  const auto rebuilt = patch.apply(old_seq);

  EXPECT_EQ(rebuilt, new_seq);
  EXPECT_EQ(patch.copied_count(), 3u);
  EXPECT_EQ(patch.deleted_count(), 1u);
  EXPECT_EQ(patch.inserted_count(), 1u);
}

TEST(MyersDiffTest, MixedEditsAreAppliedCorrectly)
{
  const std::vector<int> old_seq{10, 20, 30, 40, 50, 60};
  const std::vector<int> new_seq{10, 30, 35, 40, 60, 70};

  const auto patch = diff(old_seq, new_seq);
  const auto rebuilt = patch.apply(old_seq);

  EXPECT_EQ(rebuilt, new_seq);
  EXPECT_EQ(patch.copied_count(), 4u);
  EXPECT_EQ(patch.deleted_count(), 2u);
  EXPECT_EQ(patch.inserted_count(), 2u);
}

TEST(MyersDiffTest, PatchOperationsAreCoalesced)
{
  const std::vector<int> old_seq{1, 2, 3, 4, 5};
  const std::vector<int> new_seq{1, 2, 8, 9, 5};

  const auto patch = diff(old_seq, new_seq);
  const auto rebuilt = patch.apply(old_seq);

  EXPECT_EQ(rebuilt, new_seq);

  // A reasonable coalesced structure here is:
  // Copy 2, Delete 2, Insert [8, 9], Copy 1
  EXPECT_EQ(patch.copied_count(), 3u);
  EXPECT_EQ(patch.deleted_count(), 2u);
  EXPECT_EQ(patch.inserted_count(), 2u);

  const auto copy_counts = collect_copy_counts(patch);
  const auto delete_counts = collect_delete_counts(patch);
  const auto inserted_values = collect_inserted_values(patch);

  EXPECT_EQ(copy_counts, (std::vector<std::size_t>{2u, 1u}));
  EXPECT_EQ(delete_counts, (std::vector<std::size_t>{2u}));
  EXPECT_EQ(inserted_values, (std::vector<int>{8, 9}));
}

struct Item {
  int id{};
  int payload{};

  bool operator==(const Item&) const = default;
};

TEST(MyersDiffTest, ProjectionCanComparePointeeValue)
{
  std::vector<std::shared_ptr<Item>> old_seq{
   std::make_shared<Item>(Item{1, 100}),
   std::make_shared<Item>(Item{2, 200}),
   std::make_shared<Item>(Item{3, 300})};

  std::vector<std::shared_ptr<Item>> new_seq{
   std::make_shared<Item>(Item{1, 100}),
   std::make_shared<Item>(Item{3, 300}),
   std::make_shared<Item>(Item{4, 400})};

  const auto patch =
   diff(old_seq, new_seq, [](const auto& p) -> const Item& { return *p; });

  const auto rebuilt = patch.apply(old_seq);

  ASSERT_EQ(rebuilt.size(), 3u);
  EXPECT_EQ(rebuilt[0]->id, 1);
  EXPECT_EQ(rebuilt[1]->id, 3);
  EXPECT_EQ(rebuilt[2]->id, 4);
}

TEST(MyersDiffTest, ProjectionCanCompareByIdOnly)
{
  std::vector<std::shared_ptr<Item>> old_seq{
   std::make_shared<Item>(Item{1, 10}),
   std::make_shared<Item>(Item{2, 20}),
   std::make_shared<Item>(Item{3, 30})};

  std::vector<std::shared_ptr<Item>> new_seq{
   std::make_shared<Item>(Item{1, 999}),
   std::make_shared<Item>(Item{3, 777}),
   std::make_shared<Item>(Item{4, 888})};

  const auto patch = diff(
   old_seq, new_seq, std::equal_to<>{}, [](const auto& p) { return p->id; });

  const auto rebuilt = patch.apply(old_seq);

  ASSERT_EQ(rebuilt.size(), 3u);
  EXPECT_EQ(rebuilt[0]->id, 1);
  EXPECT_EQ(rebuilt[1]->id, 3);
  EXPECT_EQ(rebuilt[2]->id, 4);
}

TEST(MyersDiffTest, CustomEqualityCaseInsensitiveChars)
{
  const std::string old_text = "AbCdE";
  const std::string new_text = "aBcDe";

  const auto case_insensitive_equal = [](char lhs, char rhs) {
    auto to_lower = [](unsigned char c) -> unsigned char {
      if(c >= 'A' && c <= 'Z') {
        return static_cast<unsigned char>(c - 'A' + 'a');
      }
      return c;
    };
    return to_lower(static_cast<unsigned char>(lhs)) ==
           to_lower(static_cast<unsigned char>(rhs));
  };

  const auto patch = diff(old_text, new_text, case_insensitive_equal);
  const auto rebuilt = patch.apply(old_text);

  // Since equality is case-insensitive, all characters can be copied from
  // old_text.
  EXPECT_EQ(rebuilt, old_text);
  EXPECT_NE(rebuilt, new_text);

  EXPECT_EQ(patch.copied_count(), old_text.size());
  EXPECT_EQ(patch.deleted_count(), 0u);
  EXPECT_EQ(patch.inserted_count(), 0u);
}

TEST(MyersDiffTest, ApplyAsDifferentResultSequenceType)
{
  const std::vector<char> old_seq{'a', 'b', 'c', 'd'};
  const std::string new_seq = "acXd";

  const auto patch = diff(old_seq, new_seq);
  const auto rebuilt = patch.apply_as<std::string>(old_seq);

  EXPECT_EQ(rebuilt, new_seq);
}

TEST(MyersDiffTest, DifferentOldAndNewSequenceTypes)
{
  const std::vector<char> old_seq{'x', 'y', 'z'};
  const std::string new_seq = "xQzR";

  const auto patch = diff(old_seq, new_seq);
  const auto rebuilt = patch.apply_as<std::string>(old_seq);

  EXPECT_EQ(rebuilt, new_seq);
}

TEST(MyersDiffTest, SingleProjectionOverloadWorks)
{
  std::vector<std::shared_ptr<Item>> old_seq{
   std::make_shared<Item>(Item{1, 10}), std::make_shared<Item>(Item{2, 20})};

  std::vector<std::shared_ptr<Item>> new_seq{
   std::make_shared<Item>(Item{1, 100}), std::make_shared<Item>(Item{3, 300})};

  const auto patch =
   diff(old_seq, new_seq, [](const auto& p) { return p->id; });

  const auto rebuilt = patch.apply(old_seq);

  ASSERT_EQ(rebuilt.size(), 2u);
  EXPECT_EQ(rebuilt[0]->id, 1);
  EXPECT_EQ(rebuilt[1]->id, 3);
}

TEST(MyersDiffTest, EqualityAndSharedProjectionOverloadWorks)
{
  const std::vector<int> old_seq{1, 2, 3};
  const std::vector<int> new_seq{11, 12, 23};

  const auto mod10_equal = [](int lhs, int rhs) {
    return (lhs % 10) == (rhs % 10);
  };

  const auto identity_proj = [](int x) { return x; };

  const auto patch = diff(old_seq, new_seq, mod10_equal, identity_proj);
  const auto rebuilt = patch.apply(old_seq);

  // According to mod-10 equality:
  // 1 == 11, 2 == 12, 3 == 23, so everything can be copied.
  EXPECT_EQ(rebuilt, old_seq);
  EXPECT_EQ(patch.copied_count(), 3u);
  EXPECT_EQ(patch.deleted_count(), 0u);
  EXPECT_EQ(patch.inserted_count(), 0u);
}

TEST(MyersDiffTest, RepeatedElementsStillApplyCorrectly)
{
  const std::string old_text = "AAAAAB";
  const std::string new_text = "AAABAA";

  const auto patch = diff(old_text, new_text);
  const auto rebuilt = patch.apply(old_text);

  EXPECT_EQ(rebuilt, new_text);
}

TEST(MyersDiffTest, PatchCanTransformLargeInsertBlock)
{
  const std::vector<int> old_seq{1, 2, 3};
  const std::vector<int> new_seq{1, 100, 101, 102, 103, 2, 3};

  const auto patch = diff(old_seq, new_seq);
  const auto rebuilt = patch.apply(old_seq);

  EXPECT_EQ(rebuilt, new_seq);
  EXPECT_EQ(patch.inserted_count(), 4u);
  EXPECT_EQ(collect_inserted_values(patch),
            (std::vector<int>{100, 101, 102, 103}));
}

TEST(MyersDiffTest, PatchCanTransformLargeDeleteBlock)
{
  const std::vector<int> old_seq{1, 2, 3, 4, 5, 6, 7};
  const std::vector<int> new_seq{1, 7};

  const auto patch = diff(old_seq, new_seq);
  const auto rebuilt = patch.apply(old_seq);

  EXPECT_EQ(rebuilt, new_seq);
  EXPECT_EQ(patch.deleted_count(), 5u);
}

TEST(MyersDiffEdgeTest, SingleElementSame)
{
  std::vector<int> old_seq{1};
  std::vector<int> new_seq{1};

  auto patch = diff(old_seq, new_seq);
  check_basic_invariants(old_seq, new_seq, patch);
}

TEST(MyersDiffEdgeTest, SingleElementDifferent)
{
  std::vector<int> old_seq{1};
  std::vector<int> new_seq{2};

  auto patch = diff(old_seq, new_seq);
  check_basic_invariants(old_seq, new_seq, patch);

  EXPECT_EQ(patch.deleted_count(), 1u);
  EXPECT_EQ(patch.inserted_count(), 1u);
}

TEST(MyersDiffEdgeTest, NoCommonSubsequence)
{
  std::vector<int> old_seq{1, 2, 3};
  std::vector<int> new_seq{4, 5, 6};

  auto patch = diff(old_seq, new_seq);
  check_basic_invariants(old_seq, new_seq, patch);

  EXPECT_EQ(patch.copied_count(), 0u);
  EXPECT_EQ(patch.deleted_count(), 3u);
  EXPECT_EQ(patch.inserted_count(), 3u);
}

TEST(MyersDiffEdgeTest, AllElementsRepeated)
{
  std::string old_seq = "AAAAAA";
  std::string new_seq = "AAA";

  auto patch = diff(old_seq, new_seq);
  check_basic_invariants(std::vector<char>(old_seq.begin(), old_seq.end()),
                         std::vector<char>(new_seq.begin(), new_seq.end()),
                         patch);
}

TEST(MyersDiffEdgeTest, RepeatedAmbiguousMatches)
{
  std::string old_seq = "ABABABAB";
  std::string new_seq = "BABA";

  auto patch = diff(old_seq, new_seq);

  check_basic_invariants(std::vector<char>(old_seq.begin(), old_seq.end()),
                         std::vector<char>(new_seq.begin(), new_seq.end()),
                         patch);
}

TEST(MyersDiffEdgeTest, AlternatingPattern)
{
  std::vector<int> old_seq{1, 2, 1, 2, 1, 2};
  std::vector<int> new_seq{2, 1, 2, 1, 2, 1};

  auto patch = diff(old_seq, new_seq);
  check_basic_invariants(old_seq, new_seq, patch);
}

TEST(MyersDiffEdgeTest, LargeInsertBlockMiddle)
{
  std::vector<int> old_seq{1, 2, 3, 4, 5};
  std::vector<int> new_seq{1, 2, 100, 101, 102, 3, 4, 5};

  auto patch = diff(old_seq, new_seq);
  check_basic_invariants(old_seq, new_seq, patch);

  EXPECT_EQ(patch.inserted_count(), 3u);
}

TEST(MyersDiffEdgeTest, LargeDeleteBlockMiddle)
{
  std::vector<int> old_seq{1, 2, 3, 4, 5, 6, 7};
  std::vector<int> new_seq{1, 7};

  auto patch = diff(old_seq, new_seq);
  check_basic_invariants(old_seq, new_seq, patch);

  EXPECT_EQ(patch.deleted_count(), 5u);
}

TEST(MyersDiffEdgeTest, PrefixSuffixOnlyChangeMiddle)
{
  std::vector<int> old_seq{1, 2, 3, 4, 5};
  std::vector<int> new_seq{1, 2, 9, 8, 5};

  auto patch = diff(old_seq, new_seq);
  check_basic_invariants(old_seq, new_seq, patch);
}

TEST(MyersDiffEdgeTest, AllEqualUnderProjection)
{
  std::vector<int> old_seq{1, 2, 3, 4};
  std::vector<int> new_seq{5, 6, 7, 8};

  auto patch = diff(old_seq, new_seq, [](int) { return 0; } // everything equal
  );

  auto rebuilt = patch.apply(old_seq);

  // Should copy everything (no insert/delete)
  EXPECT_EQ(rebuilt, old_seq);
  EXPECT_EQ(patch.deleted_count(), 0u);
  EXPECT_EQ(patch.inserted_count(), 0u);
  EXPECT_EQ(patch.copied_count(), old_seq.size());
}

TEST(MyersDiffEdgeTest, RandomStressSmall)
{
  std::mt19937 rng(123);
  std::uniform_int_distribution<int> dist(0, 3);

  for(int iter = 0; iter < 200; ++iter) {
    std::vector<int> old_seq;
    std::vector<int> new_seq;

    int n = rng() % 10;
    int m = rng() % 10;

    for(int i = 0; i < n; ++i) {
      old_seq.push_back(dist(rng));
    }
    for(int i = 0; i < m; ++i) {
      new_seq.push_back(dist(rng));
    }

    auto patch = diff(old_seq, new_seq);
    check_basic_invariants(old_seq, new_seq, patch);
  }
}

TEST(MyersDiffEdgeTest, RandomStressMedium)
{
  std::mt19937 rng(456);
  std::uniform_int_distribution<int> dist(0, 5);

  for(int iter = 0; iter < 100; ++iter) {
    std::vector<int> old_seq;
    std::vector<int> new_seq;

    int n = 50;
    int m = 50;

    for(int i = 0; i < n; ++i) {
      old_seq.push_back(dist(rng));
    }
    for(int i = 0; i < m; ++i) {
      new_seq.push_back(dist(rng));
    }

    auto patch = diff(old_seq, new_seq);
    check_basic_invariants(old_seq, new_seq, patch);
  }
}

TEST(MyersDiffEdgeTest, DegenerateDeleteAllThenInsertAll)
{
  std::vector<int> old_seq{1, 1, 1, 1};
  std::vector<int> new_seq{2, 2, 2, 2};

  auto patch = diff(old_seq, new_seq);
  check_basic_invariants(old_seq, new_seq, patch);

  EXPECT_EQ(patch.copied_count(), 0u);
  EXPECT_EQ(patch.deleted_count(), 4u);
  EXPECT_EQ(patch.inserted_count(), 4u);
}

TEST(MyersDiffEdgeTest, StabilityUnderRepeatedApply)
{
  std::vector<int> old_seq{1, 2, 3, 4};
  std::vector<int> new_seq{1, 3, 4, 5};

  auto patch = diff(old_seq, new_seq);

  auto result1 = patch.apply(old_seq);
  auto result2 = patch.apply(old_seq);

  EXPECT_EQ(result1, result2);
  EXPECT_EQ(result1, new_seq);
}
