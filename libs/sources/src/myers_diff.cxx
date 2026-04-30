module;

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <functional>
#include <ranges>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

export module pludux:myers_diff;

namespace pludux {

template<typename Sequence>
concept RandomAccessSizedSequence =
 std::ranges::random_access_range<Sequence> &&
 std::ranges::sized_range<Sequence>;

template<typename Sequence>
using sequence_type_t = std::remove_cvref_t<Sequence>;

template<typename Sequence>
using sequence_value_t = std::ranges::range_value_t<sequence_type_t<Sequence>>;

template<typename Sequence>
using sequence_reference_t =
 std::ranges::range_reference_t<sequence_type_t<Sequence>>;

template<typename Sequence, typename Value>
concept PushBackSequence =
 std::default_initializable<sequence_type_t<Sequence>> &&
 requires(sequence_type_t<Sequence>& seq, Value&& value) {
   seq.push_back(std::forward<Value>(value));
 };

template<typename Sequence, typename Iterator>
concept InsertRangeSequence =
 requires(sequence_type_t<Sequence>& seq, Iterator first, Iterator last) {
   seq.insert(seq.end(), first, last);
 };

namespace detail {

template<typename Sequence>
constexpr bool has_reserve_v =
 requires(sequence_type_t<Sequence>& seq, std::size_t n) { seq.reserve(n); };

template<typename Sequence>
decltype(auto) indexed_at(const Sequence& seq, std::ptrdiff_t index)
{
  return seq[static_cast<std::size_t>(index)];
}

template<typename Sequence>
std::size_t sequence_size(const Sequence& seq)
{
  return static_cast<std::size_t>(std::ranges::size(seq));
}

template<typename Sequence>
void maybe_reserve(Sequence& seq, std::size_t count)
{
  if constexpr(has_reserve_v<Sequence>) {
    seq.reserve(count);
  }
}

template<typename Sequence, typename Value>
void append_one(Sequence& seq, Value&& value)
{
  seq.push_back(std::forward<Value>(value));
}

template<typename Sequence, typename Range>
void append_range(Sequence& seq, const Range& range)
{
  if constexpr(InsertRangeSequence<Sequence, typename Range::const_iterator>) {
    seq.insert(seq.end(), range.begin(), range.end());
  } else {
    for(const auto& value : range) {
      append_one(seq, value);
    }
  }
}

enum class AtomicEditKind { Copy, Delete, Insert };

template<typename T>
struct AtomicEdit {
  AtomicEditKind kind{};
  T value{};
};

} // namespace detail

export template<typename T>
class Patch {
public:
  struct Copy {
    std::size_t count{};
  };

  struct Delete {
    std::size_t count{};
  };

  struct Insert {
    std::vector<T> values;
  };

  using Operation = std::variant<Copy, Delete, Insert>;

  Patch() = default;

  explicit Patch(std::vector<Operation> operations)
  : operations_(std::move(operations))
  {
  }

  [[nodiscard]] const std::vector<Operation>& operations() const noexcept
  {
    return operations_;
  }

  [[nodiscard]] bool empty() const noexcept
  {
    return operations_.empty();
  }

  [[nodiscard]] std::size_t operation_count() const noexcept
  {
    return operations_.size();
  }

  [[nodiscard]] std::size_t inserted_count() const noexcept
  {
    std::size_t total = 0;
    for(const auto& op : operations_) {
      if(const auto* ins = std::get_if<Insert>(&op)) {
        total += ins->values.size();
      }
    }
    return total;
  }

  [[nodiscard]] std::size_t deleted_count() const noexcept
  {
    std::size_t total = 0;
    for(const auto& op : operations_) {
      if(const auto* del = std::get_if<Delete>(&op)) {
        total += del->count;
      }
    }
    return total;
  }

  [[nodiscard]] std::size_t copied_count() const noexcept
  {
    std::size_t total = 0;
    for(const auto& op : operations_) {
      if(const auto* copy = std::get_if<Copy>(&op)) {
        total += copy->count;
      }
    }
    return total;
  }

  template<typename Sequence>
    requires PushBackSequence<Sequence, sequence_value_t<Sequence>> &&
             std::constructible_from<sequence_value_t<Sequence>, const T&>
  [[nodiscard]] sequence_type_t<Sequence>
  apply(const Sequence& old_sequence) const
  {
    using Result = sequence_type_t<Sequence>;

    Result result{};
    detail::maybe_reserve(result, copied_count() + inserted_count());

    auto it = std::ranges::begin(old_sequence);

    for(const auto& op : operations_) {
      std::visit(
       [&](const auto& command) {
         using Op = std::decay_t<decltype(command)>;

         if constexpr(std::same_as<Op, Copy>) {
           for(std::size_t i = 0; i < command.count; ++i, ++it) {
             detail::append_one(result, *it);
           }
         } else if constexpr(std::same_as<Op, Delete>) {
           std::ranges::advance(it, static_cast<std::ptrdiff_t>(command.count));
         } else if constexpr(std::same_as<Op, Insert>) {
           detail::append_range(result, command.values);
         }
       },
       op);
    }

    return result;
  }

  template<typename ResultSequence, typename OldSequence>
    requires PushBackSequence<ResultSequence,
                              sequence_value_t<ResultSequence>> &&
             std::constructible_from<sequence_value_t<ResultSequence>,
                                     sequence_reference_t<OldSequence>> &&
             std::constructible_from<sequence_value_t<ResultSequence>, const T&>
  [[nodiscard]] ResultSequence apply_as(const OldSequence& old_sequence) const
  {
    ResultSequence result{};
    detail::maybe_reserve(result, copied_count() + inserted_count());

    auto it = std::ranges::begin(old_sequence);

    for(const auto& op : operations_) {
      std::visit(
       [&](const auto& command) {
         using Op = std::decay_t<decltype(command)>;

         if constexpr(std::same_as<Op, Copy>) {
           for(std::size_t i = 0; i < command.count; ++i, ++it) {
             detail::append_one(result, *it);
           }
         } else if constexpr(std::same_as<Op, Delete>) {
           std::ranges::advance(it, static_cast<std::ptrdiff_t>(command.count));
         } else if constexpr(std::same_as<Op, Insert>) {
           detail::append_range(result, command.values);
         }
       },
       op);
    }

    return result;
  }

private:
  std::vector<Operation> operations_;
};

namespace detail {

template<typename T>
class PatchBuilder {
public:
  using PatchType = Patch<T>;
  using Operation = typename PatchType::Operation;
  using Copy = typename PatchType::Copy;
  using Delete = typename PatchType::Delete;
  using Insert = typename PatchType::Insert;

  void copy_one()
  {
    if(auto* copy = last_as<Copy>()) {
      ++copy->count;
      return;
    }
    operations_.emplace_back(Copy{1});
  }

  void delete_one()
  {
    if(auto* del = last_as<Delete>()) {
      ++del->count;
      return;
    }
    operations_.emplace_back(Delete{1});
  }

  void insert_one(const T& value)
  {
    if(auto* ins = last_as<Insert>()) {
      ins->values.push_back(value);
      return;
    }
    operations_.emplace_back(Insert{{value}});
  }

  void insert_one(T&& value)
  {
    if(auto* ins = last_as<Insert>()) {
      ins->values.push_back(std::move(value));
      return;
    }
    operations_.emplace_back(Insert{{std::move(value)}});
  }

  [[nodiscard]] PatchType build() &&
  {
    return PatchType{std::move(operations_)};
  }

private:
  template<typename Op>
  Op* last_as() noexcept
  {
    if(operations_.empty()) {
      return nullptr;
    }
    return std::get_if<Op>(&operations_.back());
  }

  std::vector<Operation> operations_;
};

} // namespace detail

export template<RandomAccessSizedSequence OldSequence,
                RandomAccessSizedSequence NewSequence,
                typename Equal,
                typename OldProj,
                typename NewProj>
  requires std::predicate<
   Equal&,
   std::invoke_result_t<OldProj&, sequence_reference_t<OldSequence>>,
   std::invoke_result_t<NewProj&, sequence_reference_t<NewSequence>>>
[[nodiscard]] auto diff(const OldSequence& old_sequence,
                        const NewSequence& new_sequence,
                        Equal equal,
                        OldProj old_proj,
                        NewProj new_proj)
 -> Patch<sequence_value_t<NewSequence>>
{
  using NewValue = sequence_value_t<NewSequence>;
  using AtomicEdit = detail::AtomicEdit<NewValue>;
  using AtomicEditKind = detail::AtomicEditKind;

  const std::ptrdiff_t n =
   static_cast<std::ptrdiff_t>(std::ranges::size(old_sequence));
  const std::ptrdiff_t m =
   static_cast<std::ptrdiff_t>(std::ranges::size(new_sequence));
  const std::ptrdiff_t max_d = n + m;
  const std::ptrdiff_t offset = max_d;

  if(max_d == 0) {
    return {};
  }

  std::vector<std::ptrdiff_t> v(static_cast<std::size_t>(2 * max_d + 1), 0);
  std::vector<std::vector<std::ptrdiff_t>> trace;
  trace.reserve(static_cast<std::size_t>(max_d + 1));

  auto at = [&](std::vector<std::ptrdiff_t>& arr,
                std::ptrdiff_t k) -> std::ptrdiff_t& {
    return arr[static_cast<std::size_t>(offset + k)];
  };

  auto at_const = [&](const std::vector<std::ptrdiff_t>& arr,
                      std::ptrdiff_t k) -> const std::ptrdiff_t& {
    return arr[static_cast<std::size_t>(offset + k)];
  };

  auto matches = [&](std::ptrdiff_t x, std::ptrdiff_t y) {
    return std::invoke(
     equal,
     std::invoke(old_proj, detail::indexed_at(old_sequence, x)),
     std::invoke(new_proj, detail::indexed_at(new_sequence, y)));
  };

  at(v, 1) = 0;

  std::ptrdiff_t found_d = 0;
  bool found = false;

  for(std::ptrdiff_t d = 0; d <= max_d; ++d) {
    trace.push_back(v);

    for(std::ptrdiff_t k = -d; k <= d; k += 2) {
      std::ptrdiff_t x = 0;

      if(k == -d || (k != d && at_const(v, k - 1) < at_const(v, k + 1))) {
        x = at_const(v, k + 1);
      } else {
        x = at_const(v, k - 1) + 1;
      }

      std::ptrdiff_t y = x - k;

      while(x < n && y < m && matches(x, y)) {
        ++x;
        ++y;
      }

      at(v, k) = x;

      if(x >= n && y >= m) {
        found_d = d;
        found = true;
        break;
      }
    }

    if(found) {
      break;
    }
  }

  std::vector<AtomicEdit> reversed_atomic_edits;
  reversed_atomic_edits.reserve(static_cast<std::size_t>(n + m));

  std::ptrdiff_t x = n;
  std::ptrdiff_t y = m;

  for(std::ptrdiff_t d = found_d; d > 0; --d) {
    const auto& v_prev = trace[static_cast<std::size_t>(d)];
    const std::ptrdiff_t k = x - y;

    std::ptrdiff_t prev_k = 0;
    if(k == -d ||
       (k != d && at_const(v_prev, k - 1) < at_const(v_prev, k + 1))) {
      prev_k = k + 1;
    } else {
      prev_k = k - 1;
    }

    const std::ptrdiff_t prev_x = at_const(v_prev, prev_k);
    const std::ptrdiff_t prev_y = prev_x - prev_k;

    while(x > prev_x && y > prev_y) {
      reversed_atomic_edits.push_back(
       AtomicEdit{AtomicEditKind::Copy, NewValue{}});
      --x;
      --y;
    }

    if(x == prev_x) {
      reversed_atomic_edits.push_back(AtomicEdit{
       AtomicEditKind::Insert, detail::indexed_at(new_sequence, y - 1)});
      --y;
    } else {
      reversed_atomic_edits.push_back(
       AtomicEdit{AtomicEditKind::Delete, NewValue{}});
      --x;
    }
  }

  while(x > 0 && y > 0) {
    reversed_atomic_edits.push_back(
     AtomicEdit{AtomicEditKind::Copy, NewValue{}});
    --x;
    --y;
  }

  while(x > 0) {
    reversed_atomic_edits.push_back(
     AtomicEdit{AtomicEditKind::Delete, NewValue{}});
    --x;
  }

  while(y > 0) {
    reversed_atomic_edits.push_back(AtomicEdit{
     AtomicEditKind::Insert, detail::indexed_at(new_sequence, y - 1)});
    --y;
  }

  std::reverse(reversed_atomic_edits.begin(), reversed_atomic_edits.end());

  detail::PatchBuilder<NewValue> builder;

  for(auto& edit : reversed_atomic_edits) {
    switch(edit.kind) {
    case AtomicEditKind::Copy:
      builder.copy_one();
      break;
    case AtomicEditKind::Delete:
      builder.delete_one();
      break;
    case AtomicEditKind::Insert:
      builder.insert_one(std::move(edit.value));
      break;
    }
  }

  return std::move(builder).build();
}

export template<RandomAccessSizedSequence OldSequence,
                RandomAccessSizedSequence NewSequence>
[[nodiscard]] auto diff(const OldSequence& old_sequence,
                        const NewSequence& new_sequence)
 -> Patch<sequence_value_t<NewSequence>>
{
  return diff(old_sequence,
              new_sequence,
              std::equal_to<>{},
              std::identity{},
              std::identity{});
}

export template<RandomAccessSizedSequence OldSequence,
                RandomAccessSizedSequence NewSequence,
                typename Equal>
  requires std::predicate<Equal&,
                          sequence_reference_t<OldSequence>,
                          sequence_reference_t<NewSequence>>
[[nodiscard]] auto diff(const OldSequence& old_sequence,
                        const NewSequence& new_sequence,
                        Equal equal) -> Patch<sequence_value_t<NewSequence>>
{
  return diff(old_sequence,
              new_sequence,
              std::move(equal),
              std::identity{},
              std::identity{});
}

export template<RandomAccessSizedSequence OldSequence,
                RandomAccessSizedSequence NewSequence,
                typename Equal,
                typename Proj>
[[nodiscard]] auto diff(const OldSequence& old_sequence,
                        const NewSequence& new_sequence,
                        Equal equal,
                        Proj proj) -> Patch<sequence_value_t<NewSequence>>
{
  return diff(
   old_sequence, new_sequence, std::move(equal), proj, std::move(proj));
}

export template<RandomAccessSizedSequence OldSequence,
                RandomAccessSizedSequence NewSequence,
                typename Proj>
  requires (!std::predicate<Proj&,
                            sequence_reference_t<OldSequence>,
                            sequence_reference_t<NewSequence>>)
[[nodiscard]] auto diff(const OldSequence& old_sequence,
                        const NewSequence& new_sequence,
                        Proj proj) -> Patch<sequence_value_t<NewSequence>>
{
  return diff(
   old_sequence, new_sequence, std::equal_to<>{}, proj, std::move(proj));
}

} // namespace pludux