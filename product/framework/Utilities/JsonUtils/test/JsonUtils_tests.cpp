#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <ucf/Utilities/JsonUtils/JsonBuilder.h>
#include <ucf/Utilities/JsonUtils/JsonParser.h>

using namespace ucf::utilities;

TEST_CASE("JsonBuilder - basic types", "[JsonUtils]")
{
    JsonBuilder builder;
    builder.add("string", "hello")
           .add("int", 42)
           .add("double", 3.14)
           .add("bool", true)
           .addNull("nothing");
    
    std::string json = builder.build();
    
    JsonParser parser(json);
    REQUIRE(parser.isValid());
    REQUIRE(parser.getString("string") == "hello");
    REQUIRE(parser.getInt("int") == 42);
    REQUIRE_THAT(parser.getDouble("double").value(), Catch::Matchers::WithinRel(3.14, 0.001));
    REQUIRE(parser.getBool("bool") == true);
    REQUIRE(parser.isNull("nothing"));
}

TEST_CASE("JsonBuilder - nested object", "[JsonUtils]")
{
    JsonBuilder inner;
    inner.add("x", 10).add("y", 20);
    
    JsonBuilder outer;
    outer.add("name", "point")
         .addObject("coords", inner);
    
    std::string json = outer.build();
    
    JsonParser parser(json);
    REQUIRE(parser.isValid());
    
    auto coords = parser.getObject("coords");
    REQUIRE(coords.has_value());
    REQUIRE(coords->getInt("x") == 10);
    REQUIRE(coords->getInt("y") == 20);
}

TEST_CASE("JsonBuilder - arrays", "[JsonUtils]")
{
    JsonBuilder builder;
    builder.addStringArray("names", {"Alice", "Bob", "Charlie"})
           .addNumberArray("scores", std::vector<int64_t>{100, 95, 87});
    
    std::string json = builder.build();
    
    JsonParser parser(json);
    REQUIRE(parser.isValid());
    
    auto names = parser.getStringArray("names");
    REQUIRE(names.size() == 3);
    REQUIRE(names[0] == "Alice");
    
    auto scores = parser.getIntArray("scores");
    REQUIRE(scores.size() == 3);
    REQUIRE(scores[0] == 100);
}

TEST_CASE("JsonBuilder - array of objects", "[JsonUtils]")
{
    std::vector<JsonBuilder> items;
    
    JsonBuilder item1;
    item1.add("id", 1).add("name", "Item 1");
    items.push_back(std::move(item1));
    
    JsonBuilder item2;
    item2.add("id", 2).add("name", "Item 2");
    items.push_back(std::move(item2));
    
    JsonBuilder builder;
    builder.addArray("items", items);
    
    std::string json = builder.build();
    
    JsonParser parser(json);
    auto parsedItems = parser.getObjectArray("items");
    REQUIRE(parsedItems.size() == 2);
    REQUIRE(parsedItems[0].getInt("id") == 1);
    REQUIRE(parsedItems[1].getString("name") == "Item 2");
}

TEST_CASE("JsonParser - invalid JSON", "[JsonUtils]")
{
    JsonParser parser("not valid json");
    REQUIRE_FALSE(parser.isValid());
    REQUIRE_FALSE(parser.getError().empty());
}

TEST_CASE("JsonParser - missing key returns nullopt", "[JsonUtils]")
{
    JsonParser parser(R"({"a": 1})");
    REQUIRE(parser.isValid());
    REQUIRE_FALSE(parser.getString("missing").has_value());
    REQUIRE_FALSE(parser.getInt("missing").has_value());
}

TEST_CASE("JsonBuilder - pretty print", "[JsonUtils]")
{
    JsonBuilder builder;
    builder.add("key", "value");
    
    std::string compact = builder.build();
    std::string pretty = builder.buildPretty(2);
    
    REQUIRE(compact.find('\n') == std::string::npos);
    REQUIRE(pretty.find('\n') != std::string::npos);
}

TEST_CASE("JsonBuilder - copy constructor", "[JsonUtils]")
{
    JsonBuilder original;
    original.add("key", "value");
    
    JsonBuilder copy(original);
    copy.add("another", 123);
    
    // Original should be unchanged
    JsonParser originalParser(original.build());
    REQUIRE(originalParser.getString("key") == "value");
    REQUIRE_FALSE(originalParser.contains("another"));
    
    // Copy should have both
    JsonParser copyParser(copy.build());
    REQUIRE(copyParser.getString("key") == "value");
    REQUIRE(copyParser.getInt("another") == 123);
}

TEST_CASE("JsonParser - getKeys", "[JsonUtils]")
{
    JsonParser parser(R"({"a": 1, "b": 2, "c": 3})");
    REQUIRE(parser.isValid());
    
    auto keys = parser.getKeys();
    REQUIRE(keys.size() == 3);
}

TEST_CASE("JsonParser - getArraySize", "[JsonUtils]")
{
    JsonParser parser(R"({"items": [1, 2, 3, 4, 5]})");
    REQUIRE(parser.isValid());
    REQUIRE(parser.getArraySize("items") == 5);
    REQUIRE(parser.getArraySize("missing") == 0);
}
