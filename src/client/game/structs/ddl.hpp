#pragma once

#include <cstdint>
#include <numeric>

#include "game/structs/macros.hpp"

#include "game/structs/db/xasset/core.hpp"

#include "hash.hpp"
#include "structs/func.hpp"

namespace game {

namespace ddl {
struct DDLMember {
  const char *name;
  int32_t index;
  void *parent;
  int32_t bitSize;
  int32_t limitSize;
  int32_t offset;
  int32_t type;
  int32_t externalIndex;
  uint32_t rangeLimit;
  uint32_t serverDelta;
  uint32_t clientDelta;
  int32_t arraySize;
  int32_t enumIndex;
  int32_t permission;
};

struct DDLHash {
  int32_t hash;
  int32_t index;
};

struct DDLHashTable {
  DDLHash *list;
  int32_t count;
  int32_t max;
};

struct DDLStruct {
  const char *name;
  int32_t bitSize;
  int32_t memberCount;
  DDLMember *members;
  DDLHashTable hashTableUpper;
  DDLHashTable hashTableLower;
};

struct DDLEnum {
  const char *name;
  int32_t memberCount;
  const char **members;
  DDLHashTable hashTable;
};

struct DDLDef {
  char *name;
  uint16_t version;
  uint32_t checksum;
  uint8_t flags;
  int32_t bitSize;
  int32_t byteSize;
  DDLStruct *structList;
  int32_t structCount;
  DDLEnum *enumList;
  int32_t enumCount;
  DDLDef *next;
  int32_t headerBitSize;
  int32_t headerByteSize;
  int32_t reserveSize;
  int32_t userFlagsSize;
  bool paddingUsed;
};
ASSERT_SIZE(DDLDef, 0x60);

struct DDLState {
  bool isValid;
  int32_t offset;
  int32_t arrayIndex;
  DDLMember *member;
  const DDLDef *ddlDef;
};
ASSERT_SIZE(DDLState, 0x20);

struct DDLContext;
typedef fastcallPtr_t<void(DDLContext *ctx, void *data)> DDLWriteCB;

struct DDLContext {
  void *buff;
  int32_t len;
  const DDLDef *def;
  DDLWriteCB writeCB;
  void *userData;
};

PACKED(struct StringTableCell {
  const char *string = nullptr;
  djb2Hash_t hash = 0;
  uint8_t _padding0C[4] = {0};

  static constexpr djb2Hash_t HASH_INITIAL_SEED = 5381;
  static constexpr djb2Hash_t HASH_CONSTANT = 33;

  template <ConstStringLike T>
  static inline constexpr djb2Hash_t string_hash(const T &str) noexcept {
    return djb2<HASH_INITIAL_SEED, HASH_CONSTANT>(str);
  }

  template <ConstStringLike T> inline constexpr void set_string(const T &str) {
    string = stringlike_data(str);
    hash = string_hash(str);
  }

  template <ConstStringLike T>
  inline constexpr StringTableCell(const T &str)
      : string(stringlike_data(str)), hash(string_hash(str)) {}

  inline constexpr StringTableCell() {}
});

typedef std::reference_wrapper<StringTableCell> StringTableCellRef;
typedef std::reference_wrapper<const StringTableCell> ConstStringTableCellRef;
typedef std::optional<StringTableCellRef> OptionalStringTableCellRef;

typedef vec2<size_t> StringTableCoordinate;

template <typename F>
concept stringTableFindPredicate_t =
    ((std::invocable<F, const StringTableCell &> &&
      std::is_same_v<bool, std::invoke_result_t<F, const StringTableCell &>>) ||
     (std::invocable<F, const StringTableCell &,
                     const StringTableCoordinate &> &&
      std::is_same_v<bool,
                     std::invoke_result_t<F, const StringTableCell &,
                                          const StringTableCoordinate &>>) ||
     (std::invocable<F, const StringTableCell &, const StringTableCoordinate &,
                     size_t> &&
      std::is_same_v<
          bool, std::invoke_result_t<F, const StringTableCell &,
                                     const StringTableCoordinate &, size_t>>));

struct StringTable : public db::xasset::NamedXAsset {
  typedef int16_t index_t;

  int32_t columnCount;
  int32_t rowCount;
  StringTableCell *values;
  index_t *cellIndex;

  static inline constexpr size_t MAX_CELL_COUNT =
      static_cast<size_t>(std::numeric_limits<index_t>::max()) +
      1 /* + 1 for one-indexed size */;

  [[nodiscard]] constexpr StringTableCoordinate
  index_coordinate(const size_t idx) const noexcept {
    StringTableCoordinate coord;
    coord.column = idx % static_cast<size_t>(columnCount);
    coord.row = idx / static_cast<size_t>(columnCount);
    return coord;
  }

private:
  template <typename F>
  [[nodiscard]] alwaysinline constexpr bool
  invoke_find_predicate(F &&pred, const StringTableCell &cell,
                        const size_t idx) const noexcept {
    if constexpr (std::invocable<F, const StringTableCell &,
                                 const StringTableCoordinate &, size_t>) {
      return pred(cell, index_coordinate(idx), idx);
    } else if constexpr (std::invocable<F, const StringTableCell &,
                                        const StringTableCoordinate &>) {
      return pred(cell, index_coordinate(idx));
    } else {
      return pred(cell);
    }
  }

public:
  [[nodiscard]] constexpr bool valid_row(const size_t row) const noexcept {
    return values != nullptr && std::cmp_less(row, rowCount);
  }

  [[nodiscard]] constexpr bool
  valid_column(const size_t column) const noexcept {
    return std::cmp_less(column, columnCount);
  }

  [[nodiscard]] constexpr bool
  valid_coordinate(const size_t column, const size_t row) const noexcept {
    return valid_row(row) && valid_column(column);
  }

  [[nodiscard]] constexpr bool empty() const noexcept {
    return rowCount <= 0 || columnCount <= 0 || values == nullptr;
  }

  [[nodiscard]] constexpr size_t
  coordinate_index(const size_t column, const size_t row) const noexcept {
    return (row * static_cast<size_t>(columnCount)) + column;
  }

  [[nodiscard]] constexpr size_t
  coordinate_index(const StringTableCoordinate &coord) const noexcept {
    return coordinate_index(coord.column, coord.row);
  }

  [[nodiscard]] constexpr OptionalStringTableCellRef
  at(const size_t column, const size_t row = 0) const noexcept {
    if (valid_coordinate(column, row)) {
      return values[coordinate_index(column, row)];
    }
    return std::nullopt;
  }

  [[nodiscard]] constexpr OptionalStringTableCellRef
  at(const StringTableCoordinate &coord) const noexcept {
    return at(coord.column, coord.row);
  }

  [[nodiscard]] constexpr OptionalStringTableCellRef
  operator[](const size_t column, const size_t row = 0) noexcept {
    return at(column, row);
  }

  [[nodiscard]] constexpr OptionalStringTableCellRef
  operator[](const size_t column, const size_t row = 0) const noexcept {
    return at(column, row);
  }

  [[nodiscard]] constexpr OptionalStringTableCellRef
  operator[](const StringTableCoordinate &coord) noexcept {
    return at(coord);
  }

  [[nodiscard]] constexpr OptionalStringTableCellRef
  operator[](const StringTableCoordinate &coord) const noexcept {
    return at(coord);
  }

  [[nodiscard]] constexpr std::optional<std::span<StringTableCell>>
  operator[](const size_t row_idx) noexcept {
    return row(row_idx);
  }

  [[nodiscard]] constexpr std::optional<std::span<const StringTableCell>>
  operator[](const size_t row_idx) const noexcept {
    return row(row_idx);
  }

  [[nodiscard]] constexpr std::optional<std::span<const StringTableCell>>
  row(const size_t row_idx) const noexcept {
    return valid_row(row_idx) ? std::optional(std::span<const StringTableCell>{
                                    values + coordinate_index(0, row_idx),
                                    static_cast<size_t>(columnCount)})
                              : std::nullopt;
  }

  [[nodiscard]] constexpr std::optional<std::span<StringTableCell>>
  // NOLINTNEXTLINE(readability-make-member-function-const)
  row(const size_t row_idx) noexcept {
    return valid_row(row_idx) ? std::optional(std::span<StringTableCell>{
                                    values + coordinate_index(0, row_idx),
                                    static_cast<size_t>(columnCount)})
                              : std::nullopt;
  }

  [[nodiscard]] std::vector<ConstStringTableCellRef>
  column(const size_t col_idx) const noexcept {
    std::vector<ConstStringTableCellRef> col_cells{};
    if (valid_column(col_idx)) {
      col_cells.reserve(static_cast<size_t>(rowCount));
      for (size_t r = 0; std::cmp_less(r, rowCount); ++r) {
        const OptionalStringTableCellRef cell = at(col_idx, r);
        if (cell.has_value()) {
          col_cells.emplace_back(cell.value());
        }
      }
    }
    return col_cells;
  }

  template <typename TablePtr, typename SpanType> class RowIteratorTemplate {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = SpanType;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type *;
    using reference = value_type;

    constexpr RowIteratorTemplate(TablePtr table, size_t row) noexcept
        : table_(table), row_(row) {}

    constexpr reference operator*() const noexcept {
      return *table_->row(row_);
    }

    constexpr RowIteratorTemplate &operator++() noexcept {
      ++row_;
      return *this;
    }

    constexpr RowIteratorTemplate operator++(int) noexcept {
      RowIteratorTemplate tmp = *this;
      ++row_;
      return tmp;
    }

    friend constexpr bool operator==(const RowIteratorTemplate &a,
                                     const RowIteratorTemplate &b) noexcept {
      return a.table_ == b.table_ && a.row_ == b.row_;
    }

    friend constexpr bool operator!=(const RowIteratorTemplate &a,
                                     const RowIteratorTemplate &b) noexcept {
      return !(a == b);
    }

  private:
    TablePtr table_;
    size_t row_;
  };

  using RowIterator =
      RowIteratorTemplate<StringTable *, std::span<StringTableCell>>;
  using ConstRowIterator =
      RowIteratorTemplate<const StringTable *,
                          std::span<const StringTableCell>>;

  [[nodiscard]] constexpr RowIterator begin() noexcept { return {this, 0}; }
  [[nodiscard]] constexpr RowIterator end() noexcept {
    return {this, static_cast<size_t>(rowCount)};
  }
  [[nodiscard]] constexpr ConstRowIterator begin() const noexcept {
    return {this, 0};
  }
  [[nodiscard]] constexpr ConstRowIterator end() const noexcept {
    return {this, static_cast<size_t>(rowCount)};
  }
  [[nodiscard]] constexpr ConstRowIterator cbegin() const noexcept {
    return {this, 0};
  }
  [[nodiscard]] constexpr ConstRowIterator cend() const noexcept {
    return {this, static_cast<size_t>(rowCount)};
  }

  template <stringTableFindPredicate_t F>
  [[nodiscard]] constexpr const StringTableCell *
  find(const djb2Hash_t target_hash, F &&pred) const noexcept {
    if (values != nullptr) {
      const size_t total =
          static_cast<size_t>(rowCount) * static_cast<size_t>(columnCount);
      if (cellIndex != nullptr) {
        size_t lo = 0;
        size_t hi = total;
        while (lo < hi) {
          const size_t mid = lo + ((hi - lo) / 2);
          if (values[static_cast<size_t>(cellIndex[mid])].hash >= target_hash) {
            hi = mid;
          } else {
            lo = mid + 1;
          }
        }
        for (size_t i = lo; i < total; ++i) {
          const size_t idx = static_cast<size_t>(cellIndex[i]);
          if (values[idx].hash != target_hash) {
            break;
          }
          if (invoke_find_predicate(pred, values[idx], idx)) {
            return &values[idx];
          }
        }
      } else {
        for (size_t i = 0; i < total; ++i) {
          if (values[i].hash == target_hash &&
              invoke_find_predicate(pred, values[i], i)) {
            return &values[i];
          }
        }
      }
    }
    return nullptr;
  }

  [[nodiscard]] constexpr const StringTableCell *
  find_by_hash(const djb2Hash_t target_hash) const noexcept {
    return find(target_hash, [target_hash](const StringTableCell &c) noexcept {
      return c.hash == target_hash;
    });
  }

  [[nodiscard]] constexpr const StringTableCell *
  find_by_string(const std::string_view &target_str) const noexcept {
    return find(StringTableCell::string_hash(target_str),
                [&target_str](const StringTableCell &c) noexcept {
                  return c.string != nullptr && target_str == c.string;
                });
  }

  [[nodiscard]] constexpr std::string to_csv() const noexcept {
    std::string result;
    if (rowCount > 0 && columnCount > 0 && values != nullptr) {
      for (int32_t row = 0; row < rowCount; ++row) {
        for (int32_t col = 0; col < columnCount; ++col) {
          const size_t idx = coordinate_index(static_cast<size_t>(col),
                                              static_cast<size_t>(row));
          if (values[idx].string != nullptr) {
            const std::string_view text = values[idx].string;
            if (std::ranges::any_of(text, [](char p) {
                  return p == ',' || p == '"' || p == '\n' || p == '\r';
                })) {
              result.push_back('"');
              for (const char p : text) {
                if (p == '"') {
                  result.push_back('"');
                }
                result.push_back(p);
              }
              result.push_back('"');
            } else {
              result.append(text);
            }
          }
          if (col < columnCount - 1) {
            result.push_back(',');
          }
        }
        result.push_back('\n');
      }
    }
    return result;
  }

  StringTable(const std::string_view &csv_data) {
    if (!csv_data.empty()) {
      int32_t current_cols = 0;
      bool in_quotes = false;
      bool has_content = false;

      for (size_t i = 0; i < csv_data.size(); ++i) {
        char c = csv_data[i];
        has_content = true;

        if (in_quotes) {
          switch (c) {
          case '"':
            if (i + 1 < csv_data.size() && csv_data[i + 1] == '"') {
              i++;
            } else {
              in_quotes = false;
            }
            break;
          default:
            break;
          }
        } else {
          switch (c) {
          case '"':
            in_quotes = true;
            break;
          case ',':
            current_cols++;
            break;
          case '\r':
            break;
          case '\n':
            current_cols++;
            columnCount = std::max<int32_t>(current_cols, columnCount);
            rowCount++;
            current_cols = 0;
            break;
          default:
            break;
          }
        }
      }

      if (has_content && csv_data.back() != '\n') {
        current_cols++;
        columnCount = std::max<int32_t>(current_cols, columnCount);
        rowCount++;
      }

      if (rowCount > 0 && columnCount > 0) {
        name = nullptr;

        const size_t totalCells =
            static_cast<size_t>(rowCount) * static_cast<size_t>(columnCount);
        values = new StringTableCell[totalCells];
        std::memset(values, 0, sizeof(StringTableCell) * totalCells);

        int32_t row = 0;
        int32_t col = 0;
        std::string current_field;
        in_quotes = false;

        for (size_t i = 0; i < csv_data.size(); ++i) {
          char c = csv_data[i];

          if (in_quotes) {
            switch (c) {
            case '"':
              if (i + 1 < csv_data.size() && csv_data[i + 1] == '"') {
                current_field.push_back('"');
                i++;
              } else {
                in_quotes = false;
              }
              break;
            default:
              current_field.push_back(c);
              break;
            }
          } else {
            switch (c) {
            case '"':
              in_quotes = true;
              break;
            case ',': {
              const size_t idx = coordinate_index(static_cast<size_t>(col),
                                                  static_cast<size_t>(row));
              char *dup = new char[current_field.size() + 1];
              std::memcpy(dup, current_field.data(), current_field.size());
              dup[current_field.size()] = NULL;
              values[idx].set_string(dup);
              current_field.clear();
              col++;
              break;
            }
            case '\r':
              break;
            case '\n': {
              const size_t idx = coordinate_index(static_cast<size_t>(col),
                                                  static_cast<size_t>(row));
              char *dup = new char[current_field.size() + 1];
              std::memcpy(dup, current_field.data(), current_field.size());
              dup[current_field.size()] = NULL;
              values[idx].set_string(dup);
              current_field.clear();
              row++;
              col = 0;
              break;
            }
            default:
              current_field.push_back(c);
              break;
            }
          }
        }

        if (row < rowCount && col < columnCount &&
            (csv_data.back() != '\n' || !current_field.empty())) {
          const size_t idx = coordinate_index(static_cast<size_t>(col),
                                              static_cast<size_t>(row));
          char *dup = new char[current_field.size() + 1];
          std::memcpy(dup, current_field.data(), current_field.size());
          dup[current_field.size()] = NULL;
          values[idx].set_string(dup);
        }

        if (totalCells <= MAX_CELL_COUNT) {
          cellIndex = new int16_t[totalCells];
          std::iota(cellIndex, &cellIndex[totalCells], 0);
          std::stable_sort(cellIndex, cellIndex + totalCells,
                           [this](int16_t a, int16_t b) noexcept {
                             return values[static_cast<size_t>(a)].hash <
                                    values[static_cast<size_t>(b)].hash;
                           });
        } else {
          cellIndex = nullptr;
        }
      }
    }
  }
};
ASSERT_SIZE(StringTable, 0x20);

struct statValue_t {
  int32_t intValue;
  uint8_t _padding04[4];
};
ASSERT_SIZE(statValue_t, 0x8);

} // namespace ddl
} // namespace game
