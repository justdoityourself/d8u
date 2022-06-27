/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <string>

#include "../catch.hpp"
#include "json.hpp"
#include "json_stack.hpp"
#include "transform.hpp"

using namespace d8u::json;
using namespace d8u::transform;

const std::string_view json_sample = R"(
    {
        "as": "AS16509 Amazon.com, Inc.",
        "city": "Boardman",
        "country": "United States",
        "countryCode": "US",
        "isp": "Amazon",
        "lat": 45.8696,
        "lon": -119.688,
        "org": "Amazon",
        "query": "54.148.84.95",
        "region": "OR",
        "regionName": "Oregon",
        "status": "success",
        "timezone": "America Los_Angeles",
        "zip": "97818",
        "valid" true,
        "markers": [
            {
                "name": "Rixos The Palm Dubai",
                "position": [25.1212, 55.1535]
            },
            {
                "name": "Shangri-La Hotel",
                "position": [25.2084, 55.2719]
            },
            {
                "name": "Grand Hyatt",
                "position": [25.2285, 55.3273]
            }
        ]
    })";

const std::string_view _medium = R"(
{
    "colors": [
    {
        "color": "black",
        "category": "hue",
        "type": "primary",
        "code": {
        "rgba": [255,255,255,1],
        "hex": "#000"
        }
    },
    {
        "color": "white",
        "category": "value",
        "code": {
        "rgba": [0,0,0,1],
        "hex": "#FFF"
        }
    },
    {
        "color": "red",
        "category": "hue",
        "type": "primary",
        "code": {
        "rgba": [255,0,0,1],
        "hex": "#FF0"
        }
    },
    {
        "color": "blue",
        "category": "hue",
        "type": "primary",
        "code": {
        "rgba": [0,0,255,1],
        "hex": "#00F"
        }
    },
    {
        "color": "yellow",
        "category": "hue",
        "type": "primary",
        "code": {
        "rgba": [255,255,0,1],
        "hex": "#FF0"
        }
    },
    {
        "color": "green",
        "category": "hue",
        "type": "secondary",
        "code": {
        "rgba": [0,255,0,1],
        "hex": "#0F0"
        }
    }
    ]
})";

TEST_CASE("path object", "[d8u::json]")
{
    d8u::JsonPathS root(json_sample);
    using S = std::string_view;

    auto [as, city, lat,pos] = root.FindEachT<S,S,S,float>("as", "city", "lat","markers/0/position/0");

    REQUIRE(as == "AS16509 Amazon.com, Inc.");
    REQUIRE(city == "Boardman");
    REQUIRE(lat == "45.8696");
    REQUIRE(pos == 25.1212f);
}

TEST_CASE("json rng find each", "[d8u::json]")
{
    d8u::JsonSelectS root(json_sample);

    auto [as,city,lat] = root.FindEach("as","city","lat");

    REQUIRE(as == "AS16509 Amazon.com, Inc.");
    REQUIRE(city == "Boardman");
    REQUIRE(lat == "45.8696");
}

TEST_CASE("json rng find each 2", "[d8u::json]")
{
    d8u::JsonSelectS root(_medium);

    auto [color, category, type] = root.FindEach("color", "category", "type");

    REQUIRE(color == "black");
    REQUIRE(category == "hue");
    REQUIRE(type == "primary");


    std::tie(color, category, type) = root.FindEachN<1>("color", "category", "type");

    REQUIRE(color == "white");
    REQUIRE(category == "value");
    REQUIRE(type == "primary"); // From the third object
}

TEST_CASE("json select", "[d8u::json]")
{
    auto [color, category, type] = d8u::JsonSelect(_medium,"color","category","type");

    REQUIRE(color == "black");
    REQUIRE(category == "hue");
    REQUIRE(type == "primary");
}

TEST_CASE("json path", "[d8u::json]")
{
    auto [color, category, type] = d8u::JsonPath(_medium, "colors/0/color", "colors/0/category", "colors/0/type");

    REQUIRE(color == "black");
    REQUIRE(category == "hue");
    REQUIRE(type == "primary");

    std::tie(color, category, type) = d8u::JsonPath(_medium, "colors/2/color", "colors/2/category", "colors/2/type");

    REQUIRE(color == "red");
    REQUIRE(category == "hue");
    REQUIRE(type == "primary");
}

TEST_CASE("type select", "[d8u::json]")
{
    auto [zip, lon, org,valid] = d8u::TypeSelect<int,float,std::string_view,bool>(json_sample, "zip", "lon", "org","valid");

    REQUIRE(zip == 97818);
    REQUIRE(lon == -119.688f);
    REQUIRE(org == "Amazon");
    REQUIRE(valid == true);

    std::tie(zip, lon, org, valid) = d8u::TypeSelect<int, float, std::string_view, bool>(json_sample, "zip", "lon", "org", "valid");

    REQUIRE(zip == 97818);
    REQUIRE(lon == -119.688f);
    REQUIRE(org == "Amazon");
    REQUIRE(valid == true);
}

TEST_CASE("type path", "[d8u::json]")
{
    auto [name0,name1,coord,bad] = d8u::TypePath<std::string_view, std::string_view,float,bool>(json_sample, "markers/0/name", "markers/1/name","markers/1/position/1","bad");

    REQUIRE(name0 == "Rixos The Palm Dubai");
    REQUIRE(name1 == "Shangri-La Hotel");
    REQUIRE(coord == 55.2719f);
    REQUIRE(bad == false);
}

TEST_CASE("json rng", "[d8u::json]")
{
    d8u::JsonSelectS root(json_sample);

    REQUIRE(root["as"] == "AS16509 Amazon.com, Inc.");
    REQUIRE(root["city"] == "Boardman");
    REQUIRE(root["country"] == "United States");
    REQUIRE(root["countryCode"] == "US");
    REQUIRE(root["isp"] == "Amazon");
    REQUIRE(root["lat"] == "45.8696");
    REQUIRE(root["lon"] == "-119.688");
    REQUIRE(root["org"] == "Amazon");
    REQUIRE(root["query"] == "54.148.84.95");
    REQUIRE(root["region"] == "OR");
    REQUIRE(root["regionName"] == "Oregon");
    REQUIRE(root["status"] == "success");
    REQUIRE(root["timezone"] == "America Los_Angeles");
    REQUIRE(root["zip"] == "97818");
}

TEST_CASE("bug#1", "[d8u::json]")
{
    auto broken = R"({"m":[{"p":[]}]})";
    JsonReader root(broken);
}


TEST_CASE("json read only interface", "[d8u::json]")
{
    JsonReader root(json_sample);

    CHECK_THAT("AS16509 Amazon.com, Inc.", Catch::Matchers::Equals(root["as"]));
    CHECK_THAT("Boardman", Catch::Matchers::Equals(root["city"]));
    CHECK_THAT("United States", Catch::Matchers::Equals(root["country"]));
    CHECK_THAT("US", Catch::Matchers::Equals(root["countryCode"]));
    CHECK_THAT("Amazon", Catch::Matchers::Equals(root["isp"]));
    CHECK_THAT("45.8696", Catch::Matchers::Equals(root["lat"]));
    CHECK_THAT("-119.688", Catch::Matchers::Equals(root["lon"]));
    CHECK_THAT("Amazon", Catch::Matchers::Equals(root["org"]));
    CHECK_THAT("54.148.84.95", Catch::Matchers::Equals(root["query"]));
    CHECK_THAT("OR", Catch::Matchers::Equals(root["region"]));
    CHECK_THAT("Oregon", Catch::Matchers::Equals(root["regionName"]));
    CHECK_THAT("success", Catch::Matchers::Equals(root["status"]));
    CHECK_THAT("America Los_Angeles", Catch::Matchers::Equals(root["timezone"]));
    CHECK_THAT("97818", Catch::Matchers::Equals(root["zip"]));

    auto markers = root("markers");

    CHECK_THAT("Rixos The Palm Dubai", Catch::Matchers::Equals(markers(0)["name"]));
    CHECK_THAT("[25.1212, 55.1535]", Catch::Matchers::Equals(markers(0)["position"]));

    CHECK_THAT("Shangri-La Hotel", Catch::Matchers::Equals(markers(1)["name"]));
    CHECK_THAT("[25.2084, 55.2719]", Catch::Matchers::Equals(markers(1)["position"]));

    CHECK_THAT("Grand Hyatt", Catch::Matchers::Equals(markers(2)["name"]));
    CHECK_THAT("[25.2285, 55.3273]", Catch::Matchers::Equals(markers(2)["position"]));
}

TEST_CASE("json stack", "[d8u::json]")
{
    //This is seg faulting for some reason, todo fix.
    /*auto result = d8u::StreamJsonMirror(json_sample);
    JsonReader root(result);

    CHECK_THAT("AS16509 Amazon.com, Inc.", Catch::Matchers::Equals(root["as"]));
    CHECK_THAT("Boardman", Catch::Matchers::Equals(root["city"]));
    CHECK_THAT("United States", Catch::Matchers::Equals(root["country"]));
    CHECK_THAT("US", Catch::Matchers::Equals(root["countryCode"]));
    CHECK_THAT("Amazon", Catch::Matchers::Equals(root["isp"]));
    CHECK_THAT("45.8696", Catch::Matchers::Equals(root["lat"]));
    CHECK_THAT("-119.688", Catch::Matchers::Equals(root["lon"]));
    CHECK_THAT("Amazon", Catch::Matchers::Equals(root["org"]));
    CHECK_THAT("54.148.84.95", Catch::Matchers::Equals(root["query"]));
    CHECK_THAT("OR", Catch::Matchers::Equals(root["region"]));
    CHECK_THAT("Oregon", Catch::Matchers::Equals(root["regionName"]));
    CHECK_THAT("success", Catch::Matchers::Equals(root["status"]));
    CHECK_THAT("America Los_Angeles", Catch::Matchers::Equals(root["timezone"]));
    CHECK_THAT("97818", Catch::Matchers::Equals(root["zip"]));

    auto markers = root("markers");

    CHECK_THAT("Rixos The Palm Dubai", Catch::Matchers::Equals(markers(0)["name"]));
    CHECK_THAT("[25.1212,55.1535]", Catch::Matchers::Equals(markers(0)["position"]));

    CHECK_THAT("Shangri-La Hotel", Catch::Matchers::Equals(markers(1)["name"]));
    CHECK_THAT("[25.2084,55.2719]", Catch::Matchers::Equals(markers(1)["position"]));

    CHECK_THAT("Grand Hyatt", Catch::Matchers::Equals(markers(2)["name"]));
    CHECK_THAT("[25.2285,55.3273]", Catch::Matchers::Equals(markers(2)["position"]));*/
}

TEST_CASE("Compress / Decompress", "[d8u::transform]")
{
    d8u::sse_vector v = { 0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0 };
    d8u::sse_vector t = { 0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0 };

    compress(v);
    decompress(v);

    CHECK(std::equal(v.begin(), v.end(), t.begin()));


    d8u::sse_vector u = { 0x1B,0x54,0x59,0xC3,0xAB,0x6E,0x44,0xBF,0xA2,0xF3,0xB9,0x10,0x75,0x33,0x16,0x5D,0x23,0x97,0x21,0xB7,0x01,0xA7,0x49,0x38,0x93,0xA8,0x21,0xFB,0x23,0xC6,0xF0,0x5A };
    d8u::sse_vector u2 = { 0x1B,0x54,0x59,0xC3,0xAB,0x6E,0x44,0xBF,0xA2,0xF3,0xB9,0x10,0x75,0x33,0x16,0x5D,0x23,0x97,0x21,0xB7,0x01,0xA7,0x49,0x38,0x93,0xA8,0x21,0xFB,0x23,0xC6,0xF0,0x5A };

    compress(u);
    decompress(u);

    CHECK(std::equal(u.begin(), u.end(), u2.begin()));
}

/*TEST_CASE("Encode/Decode", "[d8u::transform]")
{
    d8u::sse_vector v = { 0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0 };
    d8u::sse_vector t = { 0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0 };

    std::array<uint8_t, 64> domain = { 0 };

    auto key = encode<d8u::transform::_DefaultHash>(domain, v);
    CHECK(true == validate_block<d8u::transform::_DefaultHash>(v));

    decode(domain, v, key);
    CHECK(std::equal(t.begin(), t.end(), v.begin()));


    d8u::sse_vector u = { 0x1B,0x54,0x59,0xC3,0xAB,0x6E,0x44,0xBF,0xA2,0xF3,0xB9,0x10,0x75,0x33,0x16,0x5D,0x23,0x97,0x21,0xB7,0x01,0xA7,0x49,0x38,0x93,0xA8,0x21,0xFB,0x23,0xC6,0xF0,0x5A };
    d8u::sse_vector u2 = { 0x1B,0x54,0x59,0xC3,0xAB,0x6E,0x44,0xBF,0xA2,0xF3,0xB9,0x10,0x75,0x33,0x16,0x5D,0x23,0x97,0x21,0xB7,0x01,0xA7,0x49,0x38,0x93,0xA8,0x21,0xFB,0x23,0xC6,0xF0,0x5A };

    key = encode<d8u::transform::_DefaultHash>(domain, u);
    CHECK(true == validate_block<d8u::transform::_DefaultHash>(u));

    quick_decode(u, key);
    CHECK(std::equal(u2.begin(), u2.end(), u.begin()));
}*/