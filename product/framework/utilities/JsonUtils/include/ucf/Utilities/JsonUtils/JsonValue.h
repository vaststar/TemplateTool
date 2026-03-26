#pragma once

#include <ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h>

#include <cstdint>
#include <initializer_list>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace ucf::utilities {

/// JSON value type enumeration
enum class JsonType
{
    Null,
    Bool,
    Number,
    String,
    Array,
    Object
};

/// Unified JSON value type
/// Can represent any JSON value: null, bool, number, string, array, object
class Utilities_EXPORT JsonValue
{
public:
    // ==========================================
    // Constructors
    // ==========================================

    /// Construct null value
    JsonValue();
    JsonValue(std::nullptr_t);

    /// Construct boolean value
    JsonValue(bool value);

    /// Construct number (integer)
    JsonValue(int32_t value);
    JsonValue(int64_t value);
    JsonValue(uint32_t value);
    JsonValue(uint64_t value);

    /// Construct number (floating point)
    JsonValue(double value);

    /// Construct string
    JsonValue(const std::string& value);
    JsonValue(std::string&& value);
    JsonValue(const char* value);

    /// Construct array
    explicit JsonValue(std::vector<JsonValue> arr);

    /// Construct object
    explicit JsonValue(std::map<std::string, JsonValue> obj);

    // ==========================================
    // Copy and Move
    // ==========================================

    JsonValue(const JsonValue& other);
    JsonValue(JsonValue&& other) noexcept;
    JsonValue& operator=(const JsonValue& other);
    JsonValue& operator=(JsonValue&& other) noexcept;
    ~JsonValue();

    // ==========================================
    // Type Query
    // ==========================================

    /// Get value type
    JsonType type() const;

    /// Basic type checks
    bool isNull() const;
    bool isBool() const;
    bool isNumber() const;
    bool isString() const;
    bool isArray() const;
    bool isObject() const;

    /// Number subtype checks (only meaningful when isNumber() is true)
    bool isInteger() const;
    bool isFloat() const;

    // ==========================================
    // Value Getters (returns nullopt on type mismatch or overflow)
    // ==========================================

    std::optional<bool> asBool() const;
    std::optional<int32_t> asInt32() const;
    std::optional<int64_t> asInt64() const;
    std::optional<uint32_t> asUInt32() const;
    std::optional<uint64_t> asUInt64() const;
    std::optional<double> asDouble() const;
    std::optional<std::string> asString() const;

    // ==========================================
    // Array Operations (only valid for Array type)
    // ==========================================

    /// Get size of array/object, returns 0 for non-container types
    size_t size() const;

    /// Check if array/object is empty
    bool empty() const;

    /// Get element by index, returns null JsonValue if out of bounds
    /// Use isNull() on return value to check validity
    JsonValue get(size_t index) const;

    /// Get element by index, throws std::out_of_range if out of bounds
    JsonValue at(size_t index) const;

    /// Operator access (returns null value if out of bounds)
    JsonValue operator[](size_t index) const;

    /// Append element to array (converts to array if not already)
    void push_back(const JsonValue& value);
    void push_back(JsonValue&& value);

    // ==========================================
    // Object Operations (only valid for Object type)
    // ==========================================

    /// Check if key exists
    bool contains(const std::string& key) const;

    /// Get value by key, returns null JsonValue if key not found
    /// Use isNull() on return value to check validity (note: value itself may be null)
    /// Recommend using with contains()
    JsonValue get(const std::string& key) const;

    /// Get value by key, throws std::out_of_range if key not found
    JsonValue at(const std::string& key) const;

    /// Operator access (returns null value if key not found)
    JsonValue operator[](const std::string& key) const;
    JsonValue operator[](const char* key) const;

    /// Get all keys
    std::vector<std::string> keys() const;

    /// Set key-value pair (converts to object if not already)
    void set(const std::string& key, const JsonValue& value);
    void set(const std::string& key, JsonValue&& value);

    /// Erase key (no effect if not an object), returns whether erase succeeded
    bool erase(const std::string& key);

    // ==========================================
    // Array/Object Conversion
    // ==========================================

    /// Convert to JsonValue array
    std::vector<JsonValue> toArray() const;

    /// Convert to key-value map
    std::map<std::string, JsonValue> toMap() const;

    // ==========================================
    // Iterators (supports for-range)
    // ==========================================

    class ConstIterator;
    class ObjectIterator;
    class ObjectRange;

    /// Array iteration
    ConstIterator begin() const;
    ConstIterator end() const;

    /// Object iteration (returns pair<string, JsonValue>)
    ObjectRange items() const;

    // ==========================================
    // Serialization
    // ==========================================

    /// Serialize to compact JSON string
    std::string dump() const;

    /// Serialize to formatted JSON string
    std::string dumpPretty(int indent = 2) const;

    // ==========================================
    // Static Factory Methods
    // ==========================================

    /// Parse result structure (forward declaration)
    struct ParseResult;

    /// Parse JSON string, returns null JsonValue on failure
    /// Use isNull() on return value to check parse success
    static JsonValue parse(const std::string& jsonStr);

    /// Parse JSON string, returns value and error message
    static ParseResult parseEx(const std::string& jsonStr);

    /// Create empty array
    static JsonValue array();

    /// Create array from vector
    static JsonValue array(std::vector<JsonValue> arr);

    /// Create empty object
    static JsonValue object();

    /// Create object from map
    static JsonValue object(std::map<std::string, JsonValue> obj);

    // ==========================================
    // Comparison Operators
    // ==========================================

    bool operator==(const JsonValue& other) const;
    bool operator!=(const JsonValue& other) const;

private:
    class Impl;
    std::unique_ptr<Impl> mImpl;

    // Internal constructor
    explicit JsonValue(std::unique_ptr<Impl> impl);
};

// ==========================================
// ParseResult Definition (after class definition since it contains JsonValue)
// ==========================================

/// Parse result structure
struct JsonValue::ParseResult
{
    JsonValue value;
    std::string error;
    bool ok() const { return error.empty(); }
    ParseResult() = default;
    ~ParseResult() = default;
    ParseResult(const ParseResult&) = default;
    ParseResult(ParseResult&&) = default;
    ParseResult& operator=(const ParseResult&) = default;
    ParseResult& operator=(ParseResult&&) = default;
};

// ==========================================
// Iterator Definitions
// ==========================================

/// Array element iterator
class Utilities_EXPORT JsonValue::ConstIterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = JsonValue;
    using difference_type = std::ptrdiff_t;
    using pointer = const JsonValue*;
    using reference = const JsonValue&;

    ConstIterator();
    ~ConstIterator();
    ConstIterator(const ConstIterator& other);
    ConstIterator(ConstIterator&& other) noexcept;
    ConstIterator& operator=(const ConstIterator& other);
    ConstIterator& operator=(ConstIterator&& other) noexcept;

    JsonValue operator*() const;
    ConstIterator& operator++();
    ConstIterator operator++(int);
    bool operator==(const ConstIterator& other) const;
    bool operator!=(const ConstIterator& other) const;

private:
    friend class JsonValue;
    class Impl;
    std::unique_ptr<Impl> mImpl;
    explicit ConstIterator(std::unique_ptr<Impl> impl);
};

/// Object key-value pair iterator
class Utilities_EXPORT JsonValue::ObjectIterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::pair<std::string, JsonValue>;
    using difference_type = std::ptrdiff_t;

    ObjectIterator();
    ~ObjectIterator();
    ObjectIterator(const ObjectIterator& other);
    ObjectIterator(ObjectIterator&& other) noexcept;
    ObjectIterator& operator=(const ObjectIterator& other);
    ObjectIterator& operator=(ObjectIterator&& other) noexcept;

    value_type operator*() const;
    ObjectIterator& operator++();
    ObjectIterator operator++(int);
    bool operator==(const ObjectIterator& other) const;
    bool operator!=(const ObjectIterator& other) const;

private:
    friend class JsonValue;
    class Impl;
    std::unique_ptr<Impl> mImpl;
    explicit ObjectIterator(std::unique_ptr<Impl> impl);
};

/// Object iteration range (for for-range)
class Utilities_EXPORT JsonValue::ObjectRange
{
public:
    ObjectRange(ObjectIterator beginIt, ObjectIterator endIt);
    ObjectRange(const ObjectRange& other);
    ObjectRange(ObjectRange&& other) noexcept;
    ObjectRange& operator=(const ObjectRange& other);
    ObjectRange& operator=(ObjectRange&& other) noexcept;
    ~ObjectRange();

    ObjectIterator begin() const;
    ObjectIterator end() const;

private:
    ObjectIterator mBegin;
    ObjectIterator mEnd;
};

} // namespace ucf::utilities
