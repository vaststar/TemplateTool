/// @file JsonValue_tests.cpp
/// @brief Comprehensive unit tests for JsonValue class

#include <ucf/Utilities/JsonUtils/JsonValue.h>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <limits>
#include <map>
#include <string>
#include <vector>

using Catch::Approx;

// ============================================================================
// Constructor Tests
// ============================================================================

TEST_CASE("JsonValue default constructor", "[JsonValue][Constructor]")
{
    ucf::utilities::JsonValue v;
    CHECK(v.isNull());
    CHECK(v.type() == ucf::utilities::JsonType::Null);
}

TEST_CASE("JsonValue nullptr constructor", "[JsonValue][Constructor]")
{
    ucf::utilities::JsonValue v(nullptr);
    CHECK(v.isNull());
    CHECK(v.type() == ucf::utilities::JsonType::Null);
}

TEST_CASE("JsonValue bool constructor", "[JsonValue][Constructor]")
{
    SECTION("true value")
    {
        ucf::utilities::JsonValue v(true);
        CHECK(v.isBool());
        CHECK(v.type() == ucf::utilities::JsonType::Bool);
        CHECK(v.asBool().value() == true);
    }

    SECTION("false value")
    {
        ucf::utilities::JsonValue v(false);
        CHECK(v.isBool());
        CHECK(v.asBool().value() == false);
    }
}

TEST_CASE("JsonValue int32 constructor", "[JsonValue][Constructor]")
{
    SECTION("positive number")
    {
        ucf::utilities::JsonValue v(42);
        CHECK(v.isNumber());
        CHECK(v.isInteger());
        CHECK(!v.isFloat());
        CHECK(v.asInt32().value() == 42);
    }

    SECTION("negative number")
    {
        ucf::utilities::JsonValue v(-100);
        CHECK(v.asInt32().value() == -100);
    }

    SECTION("max value")
    {
        int32_t maxVal = std::numeric_limits<int32_t>::max();
        ucf::utilities::JsonValue v(maxVal);
        CHECK(v.asInt32().value() == maxVal);
    }

    SECTION("min value")
    {
        int32_t minVal = std::numeric_limits<int32_t>::min();
        ucf::utilities::JsonValue v(minVal);
        CHECK(v.asInt32().value() == minVal);
    }
}

TEST_CASE("JsonValue int64 constructor", "[JsonValue][Constructor]")
{
    SECTION("exceeds int32 range")
    {
        int64_t bigVal = static_cast<int64_t>(std::numeric_limits<int32_t>::max()) + 1000;
        ucf::utilities::JsonValue v(bigVal);
        CHECK(v.isNumber());
        CHECK(v.asInt64().value() == bigVal);
        CHECK(!v.asInt32().has_value()); // overflow
    }

    SECTION("max value")
    {
        int64_t maxVal = std::numeric_limits<int64_t>::max();
        ucf::utilities::JsonValue v(maxVal);
        CHECK(v.asInt64().value() == maxVal);
    }
}

TEST_CASE("JsonValue uint32 constructor", "[JsonValue][Constructor]")
{
    SECTION("normal value")
    {
        uint32_t val = 100u;
        ucf::utilities::JsonValue v(val);
        CHECK(v.asUInt32().value() == 100u);
    }

    SECTION("exceeds int32 positive range")
    {
        uint32_t bigVal = static_cast<uint32_t>(std::numeric_limits<int32_t>::max()) + 100u;
        ucf::utilities::JsonValue v(bigVal);
        CHECK(v.asUInt32().value() == bigVal);
        CHECK(!v.asInt32().has_value()); // overflow
    }
}

TEST_CASE("JsonValue uint64 constructor", "[JsonValue][Constructor]")
{
    uint64_t val = 999999999999ULL;
    ucf::utilities::JsonValue v(val);
    CHECK(v.asUInt64().value() == val);
}

TEST_CASE("JsonValue double constructor", "[JsonValue][Constructor]")
{
    SECTION("normal value")
    {
        ucf::utilities::JsonValue v(3.14159);
        CHECK(v.isNumber());
        CHECK(v.isFloat());
        CHECK(!v.isInteger());
        CHECK(v.asDouble().value() == Approx(3.14159));
    }

    SECTION("integer as double")
    {
        ucf::utilities::JsonValue v(42.0);
        CHECK(v.isNumber());
        CHECK(v.asDouble().value() == Approx(42.0));
    }

    SECTION("negative number")
    {
        ucf::utilities::JsonValue v(-2.718);
        CHECK(v.asDouble().value() == Approx(-2.718));
    }

    SECTION("very small number")
    {
        ucf::utilities::JsonValue v(1e-10);
        CHECK(v.asDouble().value() == Approx(1e-10));
    }

    SECTION("very large number")
    {
        ucf::utilities::JsonValue v(1e100);
        CHECK(v.asDouble().value() == Approx(1e100));
    }
}

TEST_CASE("JsonValue string constructor", "[JsonValue][Constructor]")
{
    SECTION("const char*")
    {
        ucf::utilities::JsonValue v("hello");
        CHECK(v.isString());
        CHECK(v.type() == ucf::utilities::JsonType::String);
        CHECK(v.asString().value() == "hello");
    }

    SECTION("std::string lvalue")
    {
        std::string s = "world";
        ucf::utilities::JsonValue v(s);
        CHECK(v.asString().value() == "world");
    }

    SECTION("std::string rvalue")
    {
        ucf::utilities::JsonValue v(std::string("moved"));
        CHECK(v.asString().value() == "moved");
    }

    SECTION("empty string")
    {
        ucf::utilities::JsonValue v("");
        CHECK(v.isString());
        CHECK(v.asString().value() == "");
    }

    SECTION("special characters")
    {
        ucf::utilities::JsonValue v("line1\nline2\ttab\"quote\\backslash");
        CHECK(v.asString().has_value());
    }
}

TEST_CASE("JsonValue vector constructor", "[JsonValue][Constructor]")
{
    SECTION("empty array")
    {
        ucf::utilities::JsonValue v(std::vector<ucf::utilities::JsonValue>{});
        CHECK(v.isArray());
        CHECK(v.type() == ucf::utilities::JsonType::Array);
        CHECK(v.size() == 0);
        CHECK(v.empty());
    }

    SECTION("array with elements")
    {
        std::vector<ucf::utilities::JsonValue> arr = {ucf::utilities::JsonValue(1), ucf::utilities::JsonValue("two"), ucf::utilities::JsonValue(3.0)};
        ucf::utilities::JsonValue v(std::move(arr));
        CHECK(v.isArray());
        CHECK(v.size() == 3);
        CHECK(!v.empty());
    }
}

TEST_CASE("JsonValue map constructor", "[JsonValue][Constructor]")
{
    SECTION("empty object")
    {
        ucf::utilities::JsonValue v(std::map<std::string, ucf::utilities::JsonValue>{});
        CHECK(v.isObject());
        CHECK(v.type() == ucf::utilities::JsonType::Object);
        CHECK(v.size() == 0);
        CHECK(v.empty());
    }

    SECTION("object with elements")
    {
        std::map<std::string, ucf::utilities::JsonValue> obj;
        obj["name"] = ucf::utilities::JsonValue("test");
        obj["value"] = ucf::utilities::JsonValue(42);
        ucf::utilities::JsonValue v(std::move(obj));
        CHECK(v.isObject());
        CHECK(v.size() == 2);
    }
}

// ============================================================================
// Copy and Move Tests
// ============================================================================

TEST_CASE("JsonValue copy constructor", "[JsonValue][CopyMove]")
{
    ucf::utilities::JsonValue original(42);
    ucf::utilities::JsonValue copy(original);

    CHECK(copy.asInt32().value() == 42);
    CHECK(original.asInt32().value() == 42); // original unchanged
}

TEST_CASE("JsonValue move constructor", "[JsonValue][CopyMove]")
{
    ucf::utilities::JsonValue original("hello");
    ucf::utilities::JsonValue moved(std::move(original));

    CHECK(moved.asString().value() == "hello");
    // original is in valid but unspecified state
}

TEST_CASE("JsonValue copy assignment", "[JsonValue][CopyMove]")
{
    ucf::utilities::JsonValue a(100);
    ucf::utilities::JsonValue b("text");
    b = a;

    CHECK(b.asInt32().value() == 100);
    CHECK(a.asInt32().value() == 100);
}

TEST_CASE("JsonValue move assignment", "[JsonValue][CopyMove]")
{
    ucf::utilities::JsonValue a(3.14);
    ucf::utilities::JsonValue b;
    b = std::move(a);

    CHECK(b.asDouble().value() == Approx(3.14));
}

TEST_CASE("JsonValue self assignment", "[JsonValue][CopyMove]")
{
    ucf::utilities::JsonValue v(42);
    v = v; // self assignment
    CHECK(v.asInt32().value() == 42);
}

// ============================================================================
// Type Query Tests
// ============================================================================

TEST_CASE("JsonValue type exclusivity", "[JsonValue][Type]")
{
    SECTION("Null")
    {
        ucf::utilities::JsonValue v;
        CHECK(v.isNull());
        CHECK(!v.isBool());
        CHECK(!v.isNumber());
        CHECK(!v.isString());
        CHECK(!v.isArray());
        CHECK(!v.isObject());
    }

    SECTION("Bool")
    {
        ucf::utilities::JsonValue v(true);
        CHECK(!v.isNull());
        CHECK(v.isBool());
        CHECK(!v.isNumber());
        CHECK(!v.isString());
        CHECK(!v.isArray());
        CHECK(!v.isObject());
    }

    SECTION("Number")
    {
        ucf::utilities::JsonValue v(42);
        CHECK(!v.isNull());
        CHECK(!v.isBool());
        CHECK(v.isNumber());
        CHECK(!v.isString());
        CHECK(!v.isArray());
        CHECK(!v.isObject());
    }

    SECTION("String")
    {
        ucf::utilities::JsonValue v("text");
        CHECK(!v.isNull());
        CHECK(!v.isBool());
        CHECK(!v.isNumber());
        CHECK(v.isString());
        CHECK(!v.isArray());
        CHECK(!v.isObject());
    }

    SECTION("Array")
    {
        ucf::utilities::JsonValue v = ucf::utilities::JsonValue::array();
        CHECK(!v.isNull());
        CHECK(!v.isBool());
        CHECK(!v.isNumber());
        CHECK(!v.isString());
        CHECK(v.isArray());
        CHECK(!v.isObject());
    }

    SECTION("Object")
    {
        ucf::utilities::JsonValue v = ucf::utilities::JsonValue::object();
        CHECK(!v.isNull());
        CHECK(!v.isBool());
        CHECK(!v.isNumber());
        CHECK(!v.isString());
        CHECK(!v.isArray());
        CHECK(v.isObject());
    }
}

TEST_CASE("JsonValue number subtype check", "[JsonValue][Type]")
{
    SECTION("integer")
    {
        ucf::utilities::JsonValue v(42);
        CHECK(v.isInteger());
        CHECK(!v.isFloat());
    }

    SECTION("float")
    {
        ucf::utilities::JsonValue v(3.14);
        CHECK(!v.isInteger());
        CHECK(v.isFloat());
    }

    SECTION("non-number type")
    {
        ucf::utilities::JsonValue v("not a number");
        CHECK(!v.isInteger());
        CHECK(!v.isFloat());
    }
}

// ============================================================================
// Value Accessor Tests
// ============================================================================

TEST_CASE("JsonValue asBool", "[JsonValue][Accessor]")
{
    SECTION("bool value")
    {
        CHECK(ucf::utilities::JsonValue(true).asBool().value() == true);
        CHECK(ucf::utilities::JsonValue(false).asBool().value() == false);
    }

    SECTION("type mismatch returns nullopt")
    {
        CHECK(!ucf::utilities::JsonValue(42).asBool().has_value());
        CHECK(!ucf::utilities::JsonValue("true").asBool().has_value());
        CHECK(!ucf::utilities::JsonValue().asBool().has_value());
    }
}

TEST_CASE("JsonValue asInt32 boundary", "[JsonValue][Accessor]")
{
    SECTION("in range")
    {
        CHECK(ucf::utilities::JsonValue(0).asInt32().value() == 0);
        CHECK(ucf::utilities::JsonValue(-1).asInt32().value() == -1);
    }

    SECTION("overflow returns nullopt")
    {
        int64_t tooBig = static_cast<int64_t>(std::numeric_limits<int32_t>::max()) + 1;
        CHECK(!ucf::utilities::JsonValue(tooBig).asInt32().has_value());

        int64_t tooSmall = static_cast<int64_t>(std::numeric_limits<int32_t>::min()) - 1;
        CHECK(!ucf::utilities::JsonValue(tooSmall).asInt32().has_value());
    }

    SECTION("type mismatch")
    {
        CHECK(!ucf::utilities::JsonValue("42").asInt32().has_value());
        CHECK(!ucf::utilities::JsonValue(true).asInt32().has_value());
    }
}

TEST_CASE("JsonValue asUInt32 boundary", "[JsonValue][Accessor]")
{
    SECTION("negative returns nullopt")
    {
        CHECK(!ucf::utilities::JsonValue(-1).asUInt32().has_value());
    }

    SECTION("normal value")
    {
        CHECK(ucf::utilities::JsonValue(0).asUInt32().value() == 0u);
        uint32_t maxVal = std::numeric_limits<uint32_t>::max();
        CHECK(ucf::utilities::JsonValue(static_cast<uint64_t>(maxVal)).asUInt32().value() == maxVal);
    }
}

TEST_CASE("JsonValue asDouble", "[JsonValue][Accessor]")
{
    SECTION("integer to double")
    {
        CHECK(ucf::utilities::JsonValue(42).asDouble().value() == Approx(42.0));
    }

    SECTION("float value")
    {
        CHECK(ucf::utilities::JsonValue(3.14).asDouble().value() == Approx(3.14));
    }

    SECTION("type mismatch")
    {
        CHECK(!ucf::utilities::JsonValue("3.14").asDouble().has_value());
    }
}

TEST_CASE("JsonValue asString", "[JsonValue][Accessor]")
{
    SECTION("string value")
    {
        CHECK(ucf::utilities::JsonValue("hello").asString().value() == "hello");
    }

    SECTION("type mismatch")
    {
        CHECK(!ucf::utilities::JsonValue(42).asString().has_value());
        CHECK(!ucf::utilities::JsonValue(true).asString().has_value());
    }
}

// ============================================================================
// Array Operation Tests
// ============================================================================

TEST_CASE("JsonValue array size and empty", "[JsonValue][Array]")
{
    SECTION("empty array")
    {
        ucf::utilities::JsonValue arr = ucf::utilities::JsonValue::array();
        CHECK(arr.size() == 0);
        CHECK(arr.empty());
    }

    SECTION("with elements")
    {
        std::vector<ucf::utilities::JsonValue> items = {ucf::utilities::JsonValue(1), ucf::utilities::JsonValue(2), ucf::utilities::JsonValue(3)};
        ucf::utilities::JsonValue arr(std::move(items));
        CHECK(arr.size() == 3);
        CHECK(!arr.empty());
    }

    SECTION("non-array type")
    {
        CHECK(ucf::utilities::JsonValue(42).size() == 0);
        CHECK(ucf::utilities::JsonValue("text").size() == 0);
        CHECK(ucf::utilities::JsonValue().size() == 0);
    }
}

TEST_CASE("JsonValue array get", "[JsonValue][Array]")
{
    std::vector<ucf::utilities::JsonValue> items = {ucf::utilities::JsonValue("a"), ucf::utilities::JsonValue("b"), ucf::utilities::JsonValue("c")};
    ucf::utilities::JsonValue arr(std::move(items));

    SECTION("normal index")
    {
        CHECK(arr.get(static_cast<size_t>(0)).asString().value() == "a");
        CHECK(arr.get(static_cast<size_t>(1)).asString().value() == "b");
        CHECK(arr.get(static_cast<size_t>(2)).asString().value() == "c");
    }

    SECTION("out of bounds returns null")
    {
        ucf::utilities::JsonValue v = arr.get(static_cast<size_t>(100));
        CHECK(v.isNull());
    }
}

TEST_CASE("JsonValue array at", "[JsonValue][Array]")
{
    std::vector<ucf::utilities::JsonValue> items = {ucf::utilities::JsonValue(10), ucf::utilities::JsonValue(20)};
    ucf::utilities::JsonValue arr(std::move(items));

    SECTION("normal index")
    {
        CHECK(arr.at(static_cast<size_t>(0)).asInt32().value() == 10);
        CHECK(arr.at(static_cast<size_t>(1)).asInt32().value() == 20);
    }

    SECTION("out of bounds throws exception")
    {
        CHECK_THROWS_AS(arr.at(static_cast<size_t>(10)), std::out_of_range);
    }
}

TEST_CASE("JsonValue array operator[]", "[JsonValue][Array]")
{
    std::vector<ucf::utilities::JsonValue> items = {ucf::utilities::JsonValue(1), ucf::utilities::JsonValue(2)};
    ucf::utilities::JsonValue arr(std::move(items));

    SECTION("normal access")
    {
        CHECK(arr[static_cast<size_t>(0)].asInt32().value() == 1);
        CHECK(arr[static_cast<size_t>(1)].asInt32().value() == 2);
    }

    SECTION("out of bounds returns null")
    {
        ucf::utilities::JsonValue v = arr[static_cast<size_t>(999)];
        CHECK(v.isNull());
    }
}

TEST_CASE("JsonValue push_back", "[JsonValue][Array]")
{
    SECTION("append to array")
    {
        ucf::utilities::JsonValue arr = ucf::utilities::JsonValue::array();
        arr.push_back(ucf::utilities::JsonValue(1));
        arr.push_back(ucf::utilities::JsonValue("two"));
        arr.push_back(ucf::utilities::JsonValue(3.0));

        CHECK(arr.size() == 3);
        CHECK(arr[static_cast<size_t>(0)].asInt32().value() == 1);
        CHECK(arr[static_cast<size_t>(1)].asString().value() == "two");
        CHECK(arr[static_cast<size_t>(2)].asDouble().value() == Approx(3.0));
    }

    SECTION("non-array auto converts to array")
    {
        ucf::utilities::JsonValue v(42); // number
        CHECK(!v.isArray());

        v.push_back(ucf::utilities::JsonValue("appended"));
        CHECK(v.isArray());
        CHECK(v.size() == 1);
        CHECK(v[static_cast<size_t>(0)].asString().value() == "appended");
    }

    SECTION("null auto converts to array")
    {
        ucf::utilities::JsonValue v;
        CHECK(v.isNull());

        v.push_back(ucf::utilities::JsonValue(100));
        CHECK(v.isArray());
        CHECK(v.size() == 1);
    }

    SECTION("object auto converts to array")
    {
        ucf::utilities::JsonValue v = ucf::utilities::JsonValue::object();
        v.set("key", ucf::utilities::JsonValue("value"));
        CHECK(v.isObject());

        v.push_back(ucf::utilities::JsonValue("item"));
        CHECK(v.isArray());
        CHECK(v.size() == 1);
    }

    SECTION("move semantics")
    {
        ucf::utilities::JsonValue arr = ucf::utilities::JsonValue::array();
        ucf::utilities::JsonValue item("moved");
        arr.push_back(std::move(item));
        CHECK(arr.size() == 1);
    }
}

// ============================================================================
// Object Operation Tests
// ============================================================================

TEST_CASE("JsonValue object size and empty", "[JsonValue][Object]")
{
    SECTION("empty object")
    {
        ucf::utilities::JsonValue obj = ucf::utilities::JsonValue::object();
        CHECK(obj.size() == 0);
        CHECK(obj.empty());
    }

    SECTION("with elements")
    {
        std::map<std::string, ucf::utilities::JsonValue> m;
        m["a"] = ucf::utilities::JsonValue(1);
        m["b"] = ucf::utilities::JsonValue(2);
        ucf::utilities::JsonValue obj(std::move(m));
        CHECK(obj.size() == 2);
        CHECK(!obj.empty());
    }
}

TEST_CASE("JsonValue object contains", "[JsonValue][Object]")
{
    std::map<std::string, ucf::utilities::JsonValue> m;
    m["exists"] = ucf::utilities::JsonValue(true);
    ucf::utilities::JsonValue obj(std::move(m));

    CHECK(obj.contains("exists"));
    CHECK(!obj.contains("not_exists"));
    CHECK(!obj.contains(""));
}

TEST_CASE("JsonValue object get", "[JsonValue][Object]")
{
    std::map<std::string, ucf::utilities::JsonValue> m;
    m["name"] = ucf::utilities::JsonValue("test");
    m["null_val"] = ucf::utilities::JsonValue(nullptr);
    ucf::utilities::JsonValue obj(std::move(m));

    SECTION("existing key")
    {
        CHECK(obj.get("name").asString().value() == "test");
    }

    SECTION("non-existing key returns null")
    {
        ucf::utilities::JsonValue v = obj.get("not_exists");
        CHECK(v.isNull());
    }

    SECTION("value itself is null")
    {
        ucf::utilities::JsonValue v = obj.get("null_val");
        CHECK(v.isNull());
        // use contains to distinguish
        CHECK(obj.contains("null_val"));
    }
}

TEST_CASE("JsonValue object at", "[JsonValue][Object]")
{
    std::map<std::string, ucf::utilities::JsonValue> m;
    m["key"] = ucf::utilities::JsonValue(42);
    ucf::utilities::JsonValue obj(std::move(m));

    SECTION("existing key")
    {
        CHECK(obj.at("key").asInt32().value() == 42);
    }

    SECTION("non-existing key throws exception")
    {
        CHECK_THROWS_AS(obj.at("missing"), std::out_of_range);
    }
}

TEST_CASE("JsonValue object operator[]", "[JsonValue][Object]")
{
    std::map<std::string, ucf::utilities::JsonValue> m;
    m["x"] = ucf::utilities::JsonValue(100);
    ucf::utilities::JsonValue obj(std::move(m));

    SECTION("const char* access")
    {
        CHECK(obj["x"].asInt32().value() == 100);
    }

    SECTION("std::string access")
    {
        std::string key = "x";
        CHECK(obj[key].asInt32().value() == 100);
    }

    SECTION("non-existing returns null")
    {
        CHECK(obj["missing"].isNull());
    }
}

TEST_CASE("JsonValue object keys", "[JsonValue][Object]")
{
    std::map<std::string, ucf::utilities::JsonValue> m;
    m["c"] = ucf::utilities::JsonValue(3);
    m["a"] = ucf::utilities::JsonValue(1);
    m["b"] = ucf::utilities::JsonValue(2);
    ucf::utilities::JsonValue obj(std::move(m));

    std::vector<std::string> k = obj.keys();
    CHECK(k.size() == 3);
    // map is ordered, so should be a, b, c
    CHECK(k[0] == "a");
    CHECK(k[1] == "b");
    CHECK(k[2] == "c");
}

TEST_CASE("JsonValue set", "[JsonValue][Object]")
{
    SECTION("set key-value to object")
    {
        ucf::utilities::JsonValue obj = ucf::utilities::JsonValue::object();
        obj.set("name", ucf::utilities::JsonValue("Alice"));
        obj.set("age", ucf::utilities::JsonValue(30));

        CHECK(obj.size() == 2);
        CHECK(obj["name"].asString().value() == "Alice");
        CHECK(obj["age"].asInt32().value() == 30);
    }

    SECTION("overwrite existing key")
    {
        ucf::utilities::JsonValue obj = ucf::utilities::JsonValue::object();
        obj.set("key", ucf::utilities::JsonValue(1));
        obj.set("key", ucf::utilities::JsonValue(2));

        CHECK(obj.size() == 1);
        CHECK(obj["key"].asInt32().value() == 2);
    }

    SECTION("non-object auto converts to object")
    {
        ucf::utilities::JsonValue v(42);
        CHECK(!v.isObject());

        v.set("key", ucf::utilities::JsonValue("value"));
        CHECK(v.isObject());
        CHECK(v.size() == 1);
        CHECK(v["key"].asString().value() == "value");
    }

    SECTION("null auto converts to object")
    {
        ucf::utilities::JsonValue v;
        CHECK(v.isNull());

        v.set("x", ucf::utilities::JsonValue(1));
        CHECK(v.isObject());
    }

    SECTION("array auto converts to object")
    {
        ucf::utilities::JsonValue v = ucf::utilities::JsonValue::array();
        v.push_back(ucf::utilities::JsonValue(1));
        CHECK(v.isArray());

        v.set("key", ucf::utilities::JsonValue("val"));
        CHECK(v.isObject());
        CHECK(v.size() == 1);
    }

    SECTION("move semantics")
    {
        ucf::utilities::JsonValue obj = ucf::utilities::JsonValue::object();
        ucf::utilities::JsonValue val("moved");
        obj.set("key", std::move(val));
        CHECK(obj["key"].asString().value() == "moved");
    }
}

TEST_CASE("JsonValue erase", "[JsonValue][Object]")
{
    std::map<std::string, ucf::utilities::JsonValue> m;
    m["a"] = ucf::utilities::JsonValue(1);
    m["b"] = ucf::utilities::JsonValue(2);
    ucf::utilities::JsonValue obj(std::move(m));

    SECTION("erase existing key")
    {
        CHECK(obj.erase("a") == true);
        CHECK(obj.size() == 1);
        CHECK(!obj.contains("a"));
    }

    SECTION("erase non-existing key")
    {
        CHECK(obj.erase("not_exist") == false);
        CHECK(obj.size() == 2);
    }

    SECTION("non-object type")
    {
        ucf::utilities::JsonValue arr = ucf::utilities::JsonValue::array();
        CHECK(arr.erase("key") == false);
    }
}

// ============================================================================
// Array/Object Conversion Tests
// ============================================================================

TEST_CASE("JsonValue toArray", "[JsonValue][Conversion]")
{
    SECTION("array conversion")
    {
        std::vector<ucf::utilities::JsonValue> items = {ucf::utilities::JsonValue(1), ucf::utilities::JsonValue(2), ucf::utilities::JsonValue(3)};
        ucf::utilities::JsonValue arr(std::move(items));

        std::vector<ucf::utilities::JsonValue> result = arr.toArray();
        CHECK(result.size() == 3);
        CHECK(result[0].asInt32().value() == 1);
        CHECK(result[1].asInt32().value() == 2);
        CHECK(result[2].asInt32().value() == 3);
    }

    SECTION("non-array returns empty vector")
    {
        ucf::utilities::JsonValue obj = ucf::utilities::JsonValue::object();
        std::vector<ucf::utilities::JsonValue> result = obj.toArray();
        CHECK(result.empty());
    }
}

TEST_CASE("JsonValue toMap", "[JsonValue][Conversion]")
{
    SECTION("object conversion")
    {
        std::map<std::string, ucf::utilities::JsonValue> m;
        m["x"] = ucf::utilities::JsonValue(10);
        m["y"] = ucf::utilities::JsonValue(20);
        ucf::utilities::JsonValue obj(std::move(m));

        std::map<std::string, ucf::utilities::JsonValue> result = obj.toMap();
        CHECK(result.size() == 2);
        CHECK(result["x"].asInt32().value() == 10);
        CHECK(result["y"].asInt32().value() == 20);
    }

    SECTION("non-object returns empty map")
    {
        ucf::utilities::JsonValue arr = ucf::utilities::JsonValue::array();
        std::map<std::string, ucf::utilities::JsonValue> result = arr.toMap();
        CHECK(result.empty());
    }
}

// ============================================================================
// Iterator Tests
// ============================================================================

TEST_CASE("JsonValue array iteration", "[JsonValue][Iterator]")
{
    std::vector<ucf::utilities::JsonValue> items = {ucf::utilities::JsonValue(1), ucf::utilities::JsonValue(2), ucf::utilities::JsonValue(3)};
    ucf::utilities::JsonValue arr(std::move(items));

    SECTION("for-range")
    {
        int sum = 0;
        for (const auto& item : arr)
        {
            sum += item.asInt32().value();
        }
        CHECK(sum == 6);
    }

    SECTION("begin/end")
    {
        auto it = arr.begin();
        CHECK((*it).asInt32().value() == 1);
        ++it;
        CHECK((*it).asInt32().value() == 2);
        ++it;
        CHECK((*it).asInt32().value() == 3);
        ++it;
        CHECK(it == arr.end());
    }

    SECTION("post increment")
    {
        auto it = arr.begin();
        auto oldIt = it++;
        CHECK((*oldIt).asInt32().value() == 1);
        CHECK((*it).asInt32().value() == 2);
    }
}

TEST_CASE("JsonValue empty array iteration", "[JsonValue][Iterator]")
{
    ucf::utilities::JsonValue arr = ucf::utilities::JsonValue::array();
    CHECK(arr.begin() == arr.end());

    int count = 0;
    for (const auto& item : arr)
    {
        (void)item;
        ++count;
    }
    CHECK(count == 0);
}

TEST_CASE("JsonValue object iteration items", "[JsonValue][Iterator]")
{
    std::map<std::string, ucf::utilities::JsonValue> m;
    m["a"] = ucf::utilities::JsonValue(1);
    m["b"] = ucf::utilities::JsonValue(2);
    ucf::utilities::JsonValue obj(std::move(m));

    SECTION("for-range")
    {
        std::map<std::string, int> result;
        for (const auto& [key, val] : obj.items())
        {
            result[key] = val.asInt32().value();
        }
        CHECK(result.size() == 2);
        CHECK(result["a"] == 1);
        CHECK(result["b"] == 2);
    }
}

TEST_CASE("JsonValue empty object iteration", "[JsonValue][Iterator]")
{
    ucf::utilities::JsonValue obj = ucf::utilities::JsonValue::object();
    int count = 0;
    for (const auto& [key, val] : obj.items())
    {
        (void)key;
        (void)val;
        ++count;
    }
    CHECK(count == 0);
}

TEST_CASE("JsonValue iterator copy and move", "[JsonValue][Iterator]")
{
    std::vector<ucf::utilities::JsonValue> items = {ucf::utilities::JsonValue(1), ucf::utilities::JsonValue(2)};
    ucf::utilities::JsonValue arr(std::move(items));

    SECTION("copy")
    {
        auto it1 = arr.begin();
        auto it2 = it1; // copy
        CHECK((*it1).asInt32().value() == (*it2).asInt32().value());
    }

    SECTION("move")
    {
        auto it1 = arr.begin();
        auto it2 = std::move(it1);
        CHECK((*it2).asInt32().value() == 1);
    }
}

// ============================================================================
// Serialization Tests
// ============================================================================

TEST_CASE("JsonValue dump", "[JsonValue][Serialization]")
{
    SECTION("null")
    {
        CHECK(ucf::utilities::JsonValue().dump() == "null");
    }

    SECTION("bool")
    {
        CHECK(ucf::utilities::JsonValue(true).dump() == "true");
        CHECK(ucf::utilities::JsonValue(false).dump() == "false");
    }

    SECTION("number")
    {
        CHECK(ucf::utilities::JsonValue(42).dump() == "42");
    }

    SECTION("string")
    {
        CHECK(ucf::utilities::JsonValue("hello").dump() == "\"hello\"");
    }

    SECTION("array")
    {
        std::vector<ucf::utilities::JsonValue> items = {ucf::utilities::JsonValue(1), ucf::utilities::JsonValue(2)};
        ucf::utilities::JsonValue arr(std::move(items));
        CHECK(arr.dump() == "[1,2]");
    }

    SECTION("object")
    {
        std::map<std::string, ucf::utilities::JsonValue> m;
        m["a"] = ucf::utilities::JsonValue(1);
        ucf::utilities::JsonValue obj(std::move(m));
        CHECK(obj.dump() == "{\"a\":1}");
    }

    SECTION("special character escaping")
    {
        ucf::utilities::JsonValue v("line\nbreak");
        std::string s = v.dump();
        CHECK(s.find("\\n") != std::string::npos);
    }
}

TEST_CASE("JsonValue dumpPretty", "[JsonValue][Serialization]")
{
    std::map<std::string, ucf::utilities::JsonValue> m;
    m["key"] = ucf::utilities::JsonValue("value");
    ucf::utilities::JsonValue obj(std::move(m));

    std::string pretty = obj.dumpPretty(2);
    CHECK(pretty.find('\n') != std::string::npos); // contains newline
    CHECK(pretty.find("  ") != std::string::npos); // contains indent
}

// ============================================================================
// Parsing Tests
// ============================================================================

TEST_CASE("JsonValue parse success", "[JsonValue][Parsing]")
{
    SECTION("null")
    {
        ucf::utilities::JsonValue v = ucf::utilities::JsonValue::parse("null");
        CHECK(v.isNull());
    }

    SECTION("bool")
    {
        CHECK(ucf::utilities::JsonValue::parse("true").asBool().value() == true);
        CHECK(ucf::utilities::JsonValue::parse("false").asBool().value() == false);
    }

    SECTION("number")
    {
        CHECK(ucf::utilities::JsonValue::parse("42").asInt32().value() == 42);
        CHECK(ucf::utilities::JsonValue::parse("-100").asInt32().value() == -100);
        CHECK(ucf::utilities::JsonValue::parse("3.14").asDouble().value() == Approx(3.14));
    }

    SECTION("string")
    {
        CHECK(ucf::utilities::JsonValue::parse("\"hello\"").asString().value() == "hello");
    }

    SECTION("array")
    {
        ucf::utilities::JsonValue arr = ucf::utilities::JsonValue::parse("[1, 2, 3]");
        CHECK(arr.isArray());
        CHECK(arr.size() == 3);
    }

    SECTION("object")
    {
        ucf::utilities::JsonValue obj = ucf::utilities::JsonValue::parse("{\"a\": 1, \"b\": 2}");
        CHECK(obj.isObject());
        CHECK(obj.size() == 2);
        CHECK(obj["a"].asInt32().value() == 1);
    }

    SECTION("nested structure")
    {
        ucf::utilities::JsonValue v = ucf::utilities::JsonValue::parse(R"({"arr": [1, 2], "nested": {"x": true}})");
        CHECK(v.isObject());
        CHECK(v["arr"].isArray());
        CHECK(v["nested"]["x"].asBool().value() == true);
    }
}

TEST_CASE("JsonValue parse failure", "[JsonValue][Parsing]")
{
    SECTION("empty string")
    {
        ucf::utilities::JsonValue v = ucf::utilities::JsonValue::parse("");
        CHECK(v.isNull());
    }

    SECTION("invalid JSON")
    {
        ucf::utilities::JsonValue v = ucf::utilities::JsonValue::parse("{invalid}");
        CHECK(v.isNull());
    }

    SECTION("unclosed bracket")
    {
        ucf::utilities::JsonValue v = ucf::utilities::JsonValue::parse("[1, 2, 3");
        CHECK(v.isNull());
    }

    SECTION("trailing comma (strict mode)")
    {
        ucf::utilities::JsonValue v = ucf::utilities::JsonValue::parse("[1, 2, ]");
        CHECK(v.isNull());
    }
}

TEST_CASE("JsonValue parseEx", "[JsonValue][Parsing]")
{
    SECTION("success")
    {
        auto result = ucf::utilities::JsonValue::parseEx("[1, 2, 3]");
        CHECK(result.ok());
        CHECK(result.error.empty());
        CHECK(result.value.isArray());
    }

    SECTION("failure returns error message")
    {
        auto result = ucf::utilities::JsonValue::parseEx("{bad json}");
        CHECK(!result.ok());
        CHECK(!result.error.empty());
        CHECK(result.value.isNull());
    }
}

// ============================================================================
// Factory Method Tests
// ============================================================================

TEST_CASE("JsonValue::array factory", "[JsonValue][Factory]")
{
    SECTION("empty array")
    {
        ucf::utilities::JsonValue arr = ucf::utilities::JsonValue::array();
        CHECK(arr.isArray());
        CHECK(arr.empty());
    }

    SECTION("from vector")
    {
        std::vector<ucf::utilities::JsonValue> items = {ucf::utilities::JsonValue(1), ucf::utilities::JsonValue(2)};
        ucf::utilities::JsonValue arr = ucf::utilities::JsonValue::array(std::move(items));
        CHECK(arr.isArray());
        CHECK(arr.size() == 2);
    }
}

TEST_CASE("JsonValue::object factory", "[JsonValue][Factory]")
{
    SECTION("empty object")
    {
        ucf::utilities::JsonValue obj = ucf::utilities::JsonValue::object();
        CHECK(obj.isObject());
        CHECK(obj.empty());
    }

    SECTION("from map")
    {
        std::map<std::string, ucf::utilities::JsonValue> m;
        m["k"] = ucf::utilities::JsonValue("v");
        ucf::utilities::JsonValue obj = ucf::utilities::JsonValue::object(std::move(m));
        CHECK(obj.isObject());
        CHECK(obj.size() == 1);
    }
}

// ============================================================================
// Comparison Operator Tests
// ============================================================================

TEST_CASE("JsonValue operator==", "[JsonValue][Comparison]")
{
    SECTION("null")
    {
        CHECK(ucf::utilities::JsonValue() == ucf::utilities::JsonValue());
        CHECK(ucf::utilities::JsonValue(nullptr) == ucf::utilities::JsonValue());
    }

    SECTION("bool")
    {
        CHECK(ucf::utilities::JsonValue(true) == ucf::utilities::JsonValue(true));
        CHECK_FALSE(ucf::utilities::JsonValue(true) == ucf::utilities::JsonValue(false));
    }

    SECTION("number")
    {
        CHECK(ucf::utilities::JsonValue(42) == ucf::utilities::JsonValue(42));
        CHECK(ucf::utilities::JsonValue(42) == ucf::utilities::JsonValue(static_cast<int64_t>(42)));
        CHECK_FALSE(ucf::utilities::JsonValue(42) == ucf::utilities::JsonValue(43));
    }

    SECTION("string")
    {
        CHECK(ucf::utilities::JsonValue("hello") == ucf::utilities::JsonValue("hello"));
        CHECK_FALSE(ucf::utilities::JsonValue("hello") == ucf::utilities::JsonValue("world"));
    }

    SECTION("array")
    {
        std::vector<ucf::utilities::JsonValue> arr1 = {ucf::utilities::JsonValue(1), ucf::utilities::JsonValue(2)};
        std::vector<ucf::utilities::JsonValue> arr2 = {ucf::utilities::JsonValue(1), ucf::utilities::JsonValue(2)};
        CHECK(ucf::utilities::JsonValue(std::move(arr1)) == ucf::utilities::JsonValue(std::move(arr2)));
    }

    SECTION("object")
    {
        std::map<std::string, ucf::utilities::JsonValue> m1, m2;
        m1["a"] = ucf::utilities::JsonValue(1);
        m2["a"] = ucf::utilities::JsonValue(1);
        CHECK(ucf::utilities::JsonValue(std::move(m1)) == ucf::utilities::JsonValue(std::move(m2)));
    }

    SECTION("different types not equal")
    {
        CHECK_FALSE(ucf::utilities::JsonValue(42) == ucf::utilities::JsonValue("42"));
        CHECK_FALSE(ucf::utilities::JsonValue(true) == ucf::utilities::JsonValue(1));
        CHECK_FALSE(ucf::utilities::JsonValue() == ucf::utilities::JsonValue(0));
    }
}

TEST_CASE("JsonValue operator!=", "[JsonValue][Comparison]")
{
    CHECK(ucf::utilities::JsonValue(1) != ucf::utilities::JsonValue(2));
    CHECK_FALSE(ucf::utilities::JsonValue(1) != ucf::utilities::JsonValue(1));
}

// ============================================================================
// Multi-byte String Tests
// ============================================================================

TEST_CASE("JsonValue multibyte string - Chinese", "[JsonValue][Multibyte]")
{
    SECTION("construction")
    {
        // 中文测试
        std::string chinese = "中文测试";
        ucf::utilities::JsonValue v(chinese);
        CHECK(v.isString());
        CHECK(v.asString().value() == chinese);
    }

    SECTION("parse and dump roundtrip")
    {
        // 你好 - hello
        std::string chinese = "你好";
        std::string jsonStr = "\"" + chinese + "\"";
        ucf::utilities::JsonValue v = ucf::utilities::JsonValue::parse(jsonStr);
        CHECK(v.asString().value() == chinese);
        CHECK(v.dump() == jsonStr);
    }

    SECTION("as object key")
    {
        // 键 - key,  值 - value
        std::string key = "键";
        std::string value = "值";
        ucf::utilities::JsonValue obj = ucf::utilities::JsonValue::object();
        obj.set(key, ucf::utilities::JsonValue(value));
        CHECK(obj.contains(key));
        CHECK(obj[key].asString().value() == value);
    }
}

TEST_CASE("JsonValue multibyte string - Japanese", "[JsonValue][Multibyte]")
{
    SECTION("hiragana and katakana")
    {
        // こんにちは - hello
        std::string japanese = "こんにちは";
        ucf::utilities::JsonValue v(japanese);
        CHECK(v.asString().value() == japanese);
    }

    SECTION("kanji")
    {
        // 日本語 - Japanese language
        std::string kanji = "日本語";
        ucf::utilities::JsonValue v = ucf::utilities::JsonValue::parse("\"" + kanji + "\"");
        CHECK(v.asString().value() == kanji);
    }
}

TEST_CASE("JsonValue multibyte string - Korean", "[JsonValue][Multibyte]")
{
    // 안녕하세요 - hello
    std::string korean = "안녕하세요";
    ucf::utilities::JsonValue v(korean);
    CHECK(v.isString());
    CHECK(v.asString().value() == korean);
}

TEST_CASE("JsonValue multibyte string - Emoji", "[JsonValue][Multibyte]")
{
    SECTION("basic emoji")
    {
        // 😀 grinning face
        std::string emoji = "😀";
        ucf::utilities::JsonValue v(emoji);
        CHECK(v.asString().value() == emoji);
    }

    SECTION("multiple emoji")
    {
        // 😀🎉🚀 grinning, party, rocket
        std::string emojis = "😀🎉🚀";
        ucf::utilities::JsonValue v(emojis);
        CHECK(v.asString().value() == emojis);
    }

    SECTION("emoji with text")
    {
        // Hello 🌍 World
        std::string mixed = "Hello 🌍 World";
        ucf::utilities::JsonValue v(mixed);
        CHECK(v.asString().value() == mixed);
    }
}

TEST_CASE("JsonValue multibyte string - Mixed languages", "[JsonValue][Multibyte]")
{
    // Mix of English, Chinese, Japanese, and Emoji:
    // "Hello 世界 こんにちは 🌏"
    std::string mixed = "Hello 世界 こんにちは 🌏";

    ucf::utilities::JsonValue v(mixed);
    CHECK(v.isString());
    CHECK(v.asString().value() == mixed);

    // Roundtrip through dump/parse
    std::string json = v.dump();
    ucf::utilities::JsonValue parsed = ucf::utilities::JsonValue::parse(json);
    CHECK(parsed.asString().value() == mixed);
}

TEST_CASE("JsonValue multibyte string - Special UTF-8 sequences", "[JsonValue][Multibyte]")
{
    SECTION("2-byte sequence - Latin extended")
    {
        // éàü - e-acute, a-grave, u-umlaut
        std::string latin = "éàü";
        ucf::utilities::JsonValue v(latin);
        CHECK(v.asString().value() == latin);
    }

    SECTION("3-byte sequence - CJK")
    {
        // 一二三 - one, two, three in Chinese
        std::string cjk = "一二三";
        ucf::utilities::JsonValue v(cjk);
        CHECK(v.asString().value() == cjk);
    }

    SECTION("4-byte sequence - Emoji and rare characters")
    {
        // 😊💻 - smiling face, laptop
        std::string fourByte = "😊💻";
        ucf::utilities::JsonValue v(fourByte);
        CHECK(v.asString().value() == fourByte);
    }
}

TEST_CASE("JsonValue multibyte string - JSON escape sequences", "[JsonValue][Multibyte]")
{
    SECTION("parse escaped unicode")
    {
        // \u4e2d\u6587 => 中文
        ucf::utilities::JsonValue v = ucf::utilities::JsonValue::parse("\"\\u4e2d\\u6587\"");
        std::string expected = "中文";
        CHECK(v.asString().value() == expected);
    }

    SECTION("parse escaped emoji (surrogate pair)")
    {
        // \uD83D\uDE00 => 😀 (grinning face, via UTF-16 surrogate pair)
        ucf::utilities::JsonValue v = ucf::utilities::JsonValue::parse("\"\\uD83D\\uDE00\"");
        std::string expected = "😀";
        CHECK(v.asString().value() == expected);
    }
}

TEST_CASE("JsonValue multibyte string in complex structures", "[JsonValue][Multibyte]")
{
    SECTION("array with multibyte strings")
    {
        ucf::utilities::JsonValue arr = ucf::utilities::JsonValue::array();
        arr.push_back(ucf::utilities::JsonValue("一"));
        arr.push_back(ucf::utilities::JsonValue("二"));
        arr.push_back(ucf::utilities::JsonValue("三"));

        CHECK(arr.size() == 3);
        CHECK(arr[static_cast<size_t>(0)].asString().value() == "一");
        CHECK(arr[static_cast<size_t>(1)].asString().value() == "二");
        CHECK(arr[static_cast<size_t>(2)].asString().value() == "三");
    }

    SECTION("nested object with multibyte keys and values")
    {
        // 外层 - outer layer, 内层 - inner layer, 值 - value
        std::string outerKey = "外层";
        std::string innerKey = "内层";
        std::string value = "值";

        ucf::utilities::JsonValue obj = ucf::utilities::JsonValue::object();
        ucf::utilities::JsonValue inner = ucf::utilities::JsonValue::object();
        inner.set(innerKey, ucf::utilities::JsonValue(value));
        obj.set(outerKey, std::move(inner));

        CHECK(obj[outerKey][innerKey].asString().value() == value);
    }

    SECTION("parse complex multibyte JSON")
    {
        // JSON with escaped unicode:
        // 名前: 张三
        // 年龄: 25
        // 城市: [北京, 上海]
        std::string json = R"({
            "\u540D\u524D": "\u5F20\u4E09",
            "\u5E74\u9F84": 25,
            "\u57CE\u5E02": ["\u5317\u4EAC", "\u4E0A\u6D77"]
        })";

        ucf::utilities::JsonValue v = ucf::utilities::JsonValue::parse(json);
        CHECK(v.isObject());

        std::string nameKey = "名前";
        std::string nameVal = "张三";
        CHECK(v[nameKey].asString().value() == nameVal);
    }
}

// ============================================================================
// Edge Cases and Special Situations Tests
// ============================================================================

TEST_CASE("JsonValue deep nesting", "[JsonValue][Edge]")
{
    // Create deep nested structure via parse
    ucf::utilities::JsonValue deep = ucf::utilities::JsonValue::parse(R"({
        "l1": {
            "l2": {
                "l3": {
                    "value": 42
                }
            }
        }
    })");

    CHECK(deep["l1"]["l2"]["l3"]["value"].asInt32().value() == 42);
}

TEST_CASE("JsonValue large array", "[JsonValue][Edge]")
{
    ucf::utilities::JsonValue arr = ucf::utilities::JsonValue::array();
    for (int i = 0; i < 1000; ++i)
    {
        arr.push_back(ucf::utilities::JsonValue(i));
    }
    CHECK(arr.size() == 1000);
    CHECK(arr[static_cast<size_t>(999)].asInt32().value() == 999);
}

TEST_CASE("JsonValue large object", "[JsonValue][Edge]")
{
    ucf::utilities::JsonValue obj = ucf::utilities::JsonValue::object();
    for (int i = 0; i < 100; ++i)
    {
        obj.set("key" + std::to_string(i), ucf::utilities::JsonValue(i));
    }
    CHECK(obj.size() == 100);
    CHECK(obj["key50"].asInt32().value() == 50);
}

TEST_CASE("JsonValue serialize then parse", "[JsonValue][Edge]")
{
    std::map<std::string, ucf::utilities::JsonValue> m;
    m["name"] = ucf::utilities::JsonValue("test");
    m["value"] = ucf::utilities::JsonValue(3.14159);
    m["arr"] = ucf::utilities::JsonValue::array();
    ucf::utilities::JsonValue original(std::move(m));

    std::string json = original.dump();
    ucf::utilities::JsonValue parsed = ucf::utilities::JsonValue::parse(json);

    CHECK(parsed["name"].asString().value() == original["name"].asString().value());
    CHECK(parsed["value"].asDouble().value() ==
          Approx(original["value"].asDouble().value()));
}

TEST_CASE("JsonValue empty key", "[JsonValue][Edge]")
{
    ucf::utilities::JsonValue obj = ucf::utilities::JsonValue::object();
    obj.set("", ucf::utilities::JsonValue("empty key"));

    CHECK(obj.contains(""));
    CHECK(obj[""].asString().value() == "empty key");
}

TEST_CASE("JsonValue special numbers", "[JsonValue][Edge]")
{
    SECTION("zero")
    {
        CHECK(ucf::utilities::JsonValue(0).asInt32().value() == 0);
        CHECK(ucf::utilities::JsonValue(0.0).asDouble().value() == Approx(0.0));
    }

    SECTION("negative zero")
    {
        ucf::utilities::JsonValue v(-0.0);
        CHECK(v.asDouble().value() == Approx(0.0));
    }
}

TEST_CASE("JsonValue mixed type array", "[JsonValue][Edge]")
{
    std::vector<ucf::utilities::JsonValue> items = {
        ucf::utilities::JsonValue(),           // null
        ucf::utilities::JsonValue(true),       // bool
        ucf::utilities::JsonValue(42),         // int
        ucf::utilities::JsonValue(3.14),       // double
        ucf::utilities::JsonValue("text"),     // string
        ucf::utilities::JsonValue::array(),    // array
        ucf::utilities::JsonValue::object()    // object
    };
    ucf::utilities::JsonValue arr(std::move(items));

    CHECK(arr.size() == 7);
    CHECK(arr[static_cast<size_t>(0)].isNull());
    CHECK(arr[static_cast<size_t>(1)].isBool());
    CHECK(arr[static_cast<size_t>(2)].isNumber());
    CHECK(arr[static_cast<size_t>(3)].isNumber());
    CHECK(arr[static_cast<size_t>(4)].isString());
    CHECK(arr[static_cast<size_t>(5)].isArray());
    CHECK(arr[static_cast<size_t>(6)].isObject());
}

TEST_CASE("JsonValue container operations on non-container types", "[JsonValue][Edge]")
{
    ucf::utilities::JsonValue num(42);

    SECTION("get returns null")
    {
        CHECK(num.get(static_cast<size_t>(0)).isNull());
        CHECK(num.get("key").isNull());
    }

    SECTION("contains returns false")
    {
        CHECK(!num.contains("any"));
    }

    SECTION("keys returns empty")
    {
        CHECK(num.keys().empty());
    }

    SECTION("erase returns false")
    {
        CHECK(!num.erase("any"));
    }

    SECTION("toArray returns empty")
    {
        CHECK(num.toArray().empty());
    }

    SECTION("toMap returns empty")
    {
        CHECK(num.toMap().empty());
    }
}

// ============================================================================
// Additional Edge Case Tests (added to catch subtle bugs)
// ============================================================================

TEST_CASE("JsonValue at() returns independent values", "[JsonValue][Array][EdgeCase]")
{
    // This test catches the bug where at() returned a reference to a static thread_local
    // which would cause data corruption when multiple at() calls are made
    std::vector<ucf::utilities::JsonValue> items = {ucf::utilities::JsonValue(10), ucf::utilities::JsonValue(20), ucf::utilities::JsonValue(30)};
    ucf::utilities::JsonValue arr(std::move(items));

    SECTION("multiple at() calls return independent values")
    {
        ucf::utilities::JsonValue a = arr.at(static_cast<size_t>(0));
        ucf::utilities::JsonValue b = arr.at(static_cast<size_t>(1));
        ucf::utilities::JsonValue c = arr.at(static_cast<size_t>(2));
        
        // All values should be independent, not overwritten
        CHECK(a.asInt32().value() == 10);
        CHECK(b.asInt32().value() == 20);
        CHECK(c.asInt32().value() == 30);
    }

    SECTION("store at() results in vector")
    {
        std::vector<ucf::utilities::JsonValue> results;
        results.push_back(arr.at(static_cast<size_t>(0)));
        results.push_back(arr.at(static_cast<size_t>(1)));
        results.push_back(arr.at(static_cast<size_t>(2)));
        
        CHECK(results[0].asInt32().value() == 10);
        CHECK(results[1].asInt32().value() == 20);
        CHECK(results[2].asInt32().value() == 30);
    }
}

TEST_CASE("JsonValue object at() returns independent values", "[JsonValue][Object][EdgeCase]")
{
    std::map<std::string, ucf::utilities::JsonValue> items = {
        {"a", ucf::utilities::JsonValue(100)},
        {"b", ucf::utilities::JsonValue(200)},
        {"c", ucf::utilities::JsonValue(300)}
    };
    ucf::utilities::JsonValue obj(std::move(items));

    SECTION("multiple at() calls return independent values")
    {
        ucf::utilities::JsonValue va = obj.at("a");
        ucf::utilities::JsonValue vb = obj.at("b");
        ucf::utilities::JsonValue vc = obj.at("c");
        
        CHECK(va.asInt32().value() == 100);
        CHECK(vb.asInt32().value() == 200);
        CHECK(vc.asInt32().value() == 300);
    }
}

TEST_CASE("JsonValue isInteger with special float values", "[JsonValue][Number][EdgeCase]")
{
    SECTION("infinity is not integer")
    {
        ucf::utilities::JsonValue v(std::numeric_limits<double>::infinity());
        CHECK(v.isNumber());
        CHECK(!v.isInteger());
        CHECK(v.isFloat());
    }

    SECTION("negative infinity is not integer")
    {
        ucf::utilities::JsonValue v(-std::numeric_limits<double>::infinity());
        CHECK(v.isNumber());
        CHECK(!v.isInteger());
        CHECK(v.isFloat());
    }

    SECTION("NaN is not integer")
    {
        ucf::utilities::JsonValue v(std::numeric_limits<double>::quiet_NaN());
        CHECK(v.isNumber());
        CHECK(!v.isInteger());
        CHECK(v.isFloat());
    }

    SECTION("whole number double is integer")
    {
        ucf::utilities::JsonValue v(42.0);
        CHECK(v.isNumber());
        CHECK(v.isInteger());
        CHECK(!v.isFloat());
    }
}

TEST_CASE("JsonValue iterator on default-constructed object", "[JsonValue][Iterator][EdgeCase]")
{
    SECTION("default iterator equality")
    {
        ucf::utilities::JsonValue::ConstIterator it1;
        ucf::utilities::JsonValue::ConstIterator it2;
        CHECK(it1 == it2);
    }

    SECTION("iterating null returns empty range")
    {
        ucf::utilities::JsonValue nullVal;
        int count = 0;
        for (const auto& item : nullVal)
        {
            (void)item;
            ++count;
        }
        CHECK(count == 0);
    }

    SECTION("items() on non-object returns empty range")
    {
        ucf::utilities::JsonValue arr = ucf::utilities::JsonValue::array();
        arr.push_back(1);
        
        int count = 0;
        for (const auto& [key, val] : arr.items())
        {
            (void)key;
            (void)val;
            ++count;
        }
        CHECK(count == 0);
    }
}

TEST_CASE("JsonValue push_back type conversion behavior", "[JsonValue][Array][EdgeCase]")
{
    SECTION("push_back on null converts to array")
    {
        ucf::utilities::JsonValue v;
        CHECK(v.isNull());
        v.push_back(42);
        CHECK(v.isArray());
        CHECK(v.size() == 1);
        CHECK(v.get(static_cast<size_t>(0)).asInt32().value() == 42);
    }

    SECTION("push_back on string converts to array (data loss)")
    {
        ucf::utilities::JsonValue v("original string");
        CHECK(v.isString());
        v.push_back(42);
        CHECK(v.isArray());
        CHECK(v.size() == 1);
        // Original string is lost - this is documented behavior
    }
}

TEST_CASE("JsonValue set type conversion behavior", "[JsonValue][Object][EdgeCase]")
{
    SECTION("set on null converts to object")
    {
        ucf::utilities::JsonValue v;
        CHECK(v.isNull());
        v.set("key", 42);
        CHECK(v.isObject());
        CHECK(v.contains("key"));
        CHECK(v.get("key").asInt32().value() == 42);
    }

    SECTION("set on array converts to object (data loss)")
    {
        ucf::utilities::JsonValue v = ucf::utilities::JsonValue::array();
        v.push_back(1);
        v.push_back(2);
        CHECK(v.isArray());
        v.set("key", 42);
        CHECK(v.isObject());
        CHECK(v.size() == 1);
        // Original array is lost - this is documented behavior
    }
}

TEST_CASE("JsonValue nested structure access", "[JsonValue][Nested][EdgeCase]")
{
    // Test deeply nested access
    auto inner = ucf::utilities::JsonValue::object();
    inner.set("value", 42);
    
    auto middle = ucf::utilities::JsonValue::array();
    middle.push_back(inner);
    
    auto outer = ucf::utilities::JsonValue::object();
    outer.set("data", middle);

    SECTION("chained access")
    {
        ucf::utilities::JsonValue result = outer.get("data").get(static_cast<size_t>(0)).get("value");
        CHECK(result.asInt32().value() == 42);
    }

    SECTION("invalid path returns null")
    {
        ucf::utilities::JsonValue result = outer.get("missing").get("path");
        CHECK(result.isNull());
    }
}

TEST_CASE("JsonValue comparison edge cases", "[JsonValue][Comparison][EdgeCase]")
{
    SECTION("null equals null")
    {
        ucf::utilities::JsonValue a;
        ucf::utilities::JsonValue b(nullptr);
        CHECK(a == b);
    }

    SECTION("different types are not equal")
    {
        ucf::utilities::JsonValue num(42);
        ucf::utilities::JsonValue str("42");
        CHECK(num != str);
    }

    SECTION("empty array equals empty array")
    {
        ucf::utilities::JsonValue a = ucf::utilities::JsonValue::array();
        ucf::utilities::JsonValue b = ucf::utilities::JsonValue::array();
        CHECK(a == b);
    }

    SECTION("empty object equals empty object")
    {
        ucf::utilities::JsonValue a = ucf::utilities::JsonValue::object();
        ucf::utilities::JsonValue b = ucf::utilities::JsonValue::object();
        CHECK(a == b);
    }

    SECTION("empty array not equal to empty object")
    {
        ucf::utilities::JsonValue arr = ucf::utilities::JsonValue::array();
        ucf::utilities::JsonValue obj = ucf::utilities::JsonValue::object();
        CHECK(arr != obj);
    }
}

// ============================================================================
// Regression Tests - at() return by value (previously returned dangling reference)
// ============================================================================

TEST_CASE("JsonValue at() returns independent values - array", "[JsonValue][Regression][at]")
{
    auto arr = ucf::utilities::JsonValue::array();
    arr.push_back(10);
    arr.push_back(20);
    arr.push_back(30);

    SECTION("multiple at() calls return independent values")
    {
        // This test would fail with the old implementation that used thread_local static
        // because the second call would overwrite the first value
        ucf::utilities::JsonValue a = arr.at(0);
        ucf::utilities::JsonValue b = arr.at(1);
        ucf::utilities::JsonValue c = arr.at(2);

        // All three values should be independent
        CHECK(a.asInt32().value() == 10);
        CHECK(b.asInt32().value() == 20);
        CHECK(c.asInt32().value() == 30);
    }

    SECTION("at() value persists after another at() call")
    {
        ucf::utilities::JsonValue first = arr.at(0);
        CHECK(first.asInt32().value() == 10);

        // Call at() again with different index
        ucf::utilities::JsonValue second = arr.at(2);
        CHECK(second.asInt32().value() == 30);

        // First value should still be valid
        CHECK(first.asInt32().value() == 10);
    }

    SECTION("store multiple at() results in vector")
    {
        std::vector<ucf::utilities::JsonValue> values;
        for (size_t i = 0; i < arr.size(); ++i)
        {
            values.push_back(arr.at(i));
        }

        CHECK(values[0].asInt32().value() == 10);
        CHECK(values[1].asInt32().value() == 20);
        CHECK(values[2].asInt32().value() == 30);
    }
}

TEST_CASE("JsonValue at() returns independent values - object", "[JsonValue][Regression][at]")
{
    auto obj = ucf::utilities::JsonValue::object();
    obj.set("a", 100);
    obj.set("b", 200);
    obj.set("c", 300);

    SECTION("multiple at() calls return independent values")
    {
        ucf::utilities::JsonValue a = obj.at("a");
        ucf::utilities::JsonValue b = obj.at("b");
        ucf::utilities::JsonValue c = obj.at("c");

        CHECK(a.asInt32().value() == 100);
        CHECK(b.asInt32().value() == 200);
        CHECK(c.asInt32().value() == 300);
    }

    SECTION("at() value persists after another at() call")
    {
        ucf::utilities::JsonValue first = obj.at("a");
        CHECK(first.asInt32().value() == 100);

        ucf::utilities::JsonValue second = obj.at("c");
        CHECK(second.asInt32().value() == 300);

        // First should still be valid
        CHECK(first.asInt32().value() == 100);
    }

    SECTION("store multiple at() results in map")
    {
        std::map<std::string, ucf::utilities::JsonValue> values;
        values["a"] = obj.at("a");
        values["b"] = obj.at("b");
        values["c"] = obj.at("c");

        CHECK(values["a"].asInt32().value() == 100);
        CHECK(values["b"].asInt32().value() == 200);
        CHECK(values["c"].asInt32().value() == 300);
    }
}

// ============================================================================
// Edge Cases - isInteger() with special floating point values
// ============================================================================

TEST_CASE("JsonValue isInteger with special float values", "[JsonValue][EdgeCase][isInteger]")
{
    SECTION("infinity is not integer")
    {
        ucf::utilities::JsonValue posInf(std::numeric_limits<double>::infinity());
        CHECK(posInf.isNumber());
        CHECK(!posInf.isInteger());
        CHECK(posInf.isFloat());
    }

    SECTION("negative infinity is not integer")
    {
        ucf::utilities::JsonValue negInf(-std::numeric_limits<double>::infinity());
        CHECK(negInf.isNumber());
        CHECK(!negInf.isInteger());
        CHECK(negInf.isFloat());
    }

    SECTION("NaN is not integer")
    {
        ucf::utilities::JsonValue nan(std::numeric_limits<double>::quiet_NaN());
        CHECK(nan.isNumber());
        CHECK(!nan.isInteger());
        CHECK(nan.isFloat());
    }

    SECTION("float with no fractional part is integer")
    {
        ucf::utilities::JsonValue v(42.0);
        CHECK(v.isNumber());
        CHECK(v.isInteger());
        CHECK(!v.isFloat());
    }

    SECTION("float with fractional part is not integer")
    {
        ucf::utilities::JsonValue v(42.5);
        CHECK(v.isNumber());
        CHECK(!v.isInteger());
        CHECK(v.isFloat());
    }
}

// ============================================================================
// Edge Cases - Default iterator dereference safety
// ============================================================================

TEST_CASE("JsonValue iterator on non-container types", "[JsonValue][Iterator][EdgeCase]")
{
    SECTION("iterating over null returns empty range")
    {
        ucf::utilities::JsonValue null;
        int count = 0;
        for ([[maybe_unused]] const auto& item : null)
        {
            ++count;
        }
        CHECK(count == 0);
    }

    SECTION("iterating over number returns empty range")
    {
        ucf::utilities::JsonValue num(42);
        int count = 0;
        for ([[maybe_unused]] const auto& item : num)
        {
            ++count;
        }
        CHECK(count == 0);
    }

    SECTION("iterating over string returns empty range")
    {
        ucf::utilities::JsonValue str("hello");
        int count = 0;
        for ([[maybe_unused]] const auto& item : str)
        {
            ++count;
        }
        CHECK(count == 0);
    }

    SECTION("items() on non-object returns empty range")
    {
        ucf::utilities::JsonValue arr = ucf::utilities::JsonValue::array({1, 2, 3});
        int count = 0;
        for ([[maybe_unused]] const auto& [key, value] : arr.items())
        {
            ++count;
        }
        CHECK(count == 0);
    }
}

// ============================================================================
// Edge Cases - push_back/set type conversion behavior
// ============================================================================

TEST_CASE("JsonValue push_back type conversion", "[JsonValue][EdgeCase][push_back]")
{
    SECTION("push_back on null converts to array")
    {
        ucf::utilities::JsonValue v;
        CHECK(v.isNull());
        v.push_back(42);
        CHECK(v.isArray());
        CHECK(v.size() == 1);
        CHECK(v.get(0).asInt32().value() == 42);
    }

    SECTION("push_back on number converts to array (original data lost)")
    {
        ucf::utilities::JsonValue v(100);
        CHECK(v.isNumber());
        v.push_back(42);
        CHECK(v.isArray());
        CHECK(v.size() == 1);
        // Note: original value 100 is lost - this is documented behavior
        CHECK(v.get(0).asInt32().value() == 42);
    }

    SECTION("push_back on object converts to array (original data lost)")
    {
        auto v = ucf::utilities::JsonValue::object();
        v.set("key", "value");
        CHECK(v.isObject());
        v.push_back(42);
        CHECK(v.isArray());
        CHECK(v.size() == 1);
        // Note: original key-value is lost
    }
}

TEST_CASE("JsonValue set type conversion", "[JsonValue][EdgeCase][set]")
{
    SECTION("set on null converts to object")
    {
        ucf::utilities::JsonValue v;
        CHECK(v.isNull());
        v.set("key", 42);
        CHECK(v.isObject());
        CHECK(v.get("key").asInt32().value() == 42);
    }

    SECTION("set on number converts to object (original data lost)")
    {
        ucf::utilities::JsonValue v(100);
        CHECK(v.isNumber());
        v.set("key", 42);
        CHECK(v.isObject());
        CHECK(v.get("key").asInt32().value() == 42);
        // Note: original value 100 is lost
    }

    SECTION("set on array converts to object (original data lost)")
    {
        auto v = ucf::utilities::JsonValue::array({1, 2, 3});
        CHECK(v.isArray());
        v.set("key", 42);
        CHECK(v.isObject());
        CHECK(v.size() == 1);
        // Note: original array data is lost
    }
}

// ============================================================================
// Edge Cases - Empty containers
// ============================================================================

TEST_CASE("JsonValue empty container operations", "[JsonValue][EdgeCase][Empty]")
{
    SECTION("empty array size and iteration")
    {
        auto arr = ucf::utilities::JsonValue::array();
        CHECK(arr.size() == 0);
        CHECK(arr.empty());

        int count = 0;
        for ([[maybe_unused]] const auto& item : arr)
        {
            ++count;
        }
        CHECK(count == 0);
    }

    SECTION("empty object size and iteration")
    {
        auto obj = ucf::utilities::JsonValue::object();
        CHECK(obj.size() == 0);
        CHECK(obj.empty());
        CHECK(obj.keys().empty());

        int count = 0;
        for ([[maybe_unused]] const auto& [key, value] : obj.items())
        {
            ++count;
        }
        CHECK(count == 0);
    }

    SECTION("access on empty array")
    {
        auto arr = ucf::utilities::JsonValue::array();
        CHECK(arr.get(static_cast<size_t>(0)).isNull());
        CHECK(arr[static_cast<size_t>(0)].isNull());
        CHECK_THROWS_AS(arr.at(static_cast<size_t>(0)), std::out_of_range);
    }

    SECTION("access on empty object")
    {
        auto obj = ucf::utilities::JsonValue::object();
        CHECK(obj.get("any").isNull());
        CHECK(obj["any"].isNull());
        CHECK(!obj.contains("any"));
        CHECK_THROWS_AS(obj.at("any"), std::out_of_range);
    }
}

// ============================================================================
// Edge Cases - Large numbers precision
// ============================================================================

TEST_CASE("JsonValue large number precision", "[JsonValue][EdgeCase][Precision]")
{
    SECTION("uint64 max value")
    {
        uint64_t maxVal = std::numeric_limits<uint64_t>::max();
        ucf::utilities::JsonValue v(maxVal);
        CHECK(v.asUInt64().value() == maxVal);
        CHECK(!v.asInt64().has_value()); // overflow to signed
    }

    SECTION("int64 min value")
    {
        int64_t minVal = std::numeric_limits<int64_t>::min();
        ucf::utilities::JsonValue v(minVal);
        CHECK(v.asInt64().value() == minVal);
        CHECK(!v.asUInt64().has_value()); // negative
    }

    SECTION("double with very large value")
    {
        double largeVal = 1e308;
        ucf::utilities::JsonValue v(largeVal);
        CHECK(v.asDouble().value() == Approx(largeVal));
        CHECK(!v.asInt64().has_value()); // not integer
    }

    SECTION("double with very small fractional part")
    {
        double val = 42.0000000001;
        ucf::utilities::JsonValue v(val);
        CHECK(!v.isInteger()); // has fractional part
        CHECK(v.isFloat());
    }
}
