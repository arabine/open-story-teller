#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "json_wrapper.h"
#include "json.hpp"
using Catch::Matchers::WithinRel;
using namespace nlohmann;

TEST_CASE("JsonWrapper basic types", "[json]") {
    JsonWrapper json;
    json.setString("name", "Alice");
    json.setInt("age", 30);
    json.setDouble("pi", 3.14159);
    json.setBool("is_valid", true);

    std::string err;

    SECTION("Get string") {
        auto name = json.getString("name", err);
        REQUIRE(name.has_value());
        REQUIRE(name.value() == "Alice");
    }

    SECTION("Get int") {
        auto age = json.getInt("age", err);
        REQUIRE(age.has_value());
        REQUIRE(age.value() == 30);
    }

    SECTION("Get double") {
        auto pi = json.getDouble("pi", err);
        REQUIRE(pi.has_value());
        REQUIRE_THAT(pi.value(), WithinRel(3.14159, 1e-6));
    }

    SECTION("Get bool") {
        auto valid = json.getBool("is_valid", err);
        REQUIRE(valid.has_value());
        REQUIRE(valid.value() == true);
    }
}

TEST_CASE("JsonWrapper arrays", "[json]") {
    JsonWrapper json;

    json.setArray<int>("scores", {10, 20, 30});
    json.setArray<std::string>("tags", {"news", "tech"});
    json.setArray<double>("measurements", {1.1, 2.2, 3.3});
    json.setArray<bool>("flags", {true, false, true});

    std::string err;

    auto scores = json.getArray<int>("scores", err);
    REQUIRE(scores.has_value());
    REQUIRE(scores->size() == 3);
    REQUIRE(scores->at(1) == 20);

    auto tags = json.getArray<std::string>("tags", err);
    REQUIRE(tags.has_value());
    REQUIRE(tags->at(0) == "news");

    auto measurements = json.getArray<double>("measurements", err);
    REQUIRE(measurements.has_value());
    REQUIRE_THAT(measurements->at(2), WithinRel(3.3, 1e-6));

    auto flags = json.getArray<bool>("flags", err);
    REQUIRE(flags.has_value());
    REQUIRE(flags->at(0) == true);
}

struct Connection {
    std::string host;
    int port;ccM9XAGZ$mz^b*52T5p&sMA@ujPbCUNW
};

// from_json / to_json must be defined for Connection
inline void to_json(nlohmann::json& j, const Connection& c) {
    j = nlohmann::json{{"host", c.host}, {"port", c.port}};
}

inline void from_json(const nlohmann::json& j, Connection& c) {
    j.at("host").get_to(c.host);
    j.at("port").get_to(c.port);
}

template Connection JsonWrapper::asImpl<Connection>(std::string&) const;
template Connection JsonWrapper::getImpl<Connection>(const std::string&, std::string&) const;
template std::optional<std::vector<Connection>> JsonWrapper::getArrayImpl<Connection>(const std::string&, std::string&) const;


TEST_CASE("JsonWrapper generic serialization/deserialization", "[json][struct]") {


    Connection original{"127.0.0.1", 5000};
    auto wrapper = JsonWrapper::from(original);

    std::string err;
    auto restored = wrapper.as<Connection>(err);

    REQUIRE(restored.host == "127.0.0.1");
    REQUIRE(restored.port == 5000);
}
