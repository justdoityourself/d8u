/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <string>

#include "../catch.hpp"
#include "json.hpp"
#include "transform.hpp"

using namespace d8u::json;
using namespace d8u::transform;


TEST_CASE("json read only interface", "[d8u::json]")
{
    JsonReader root(R"(
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
        "markers": [
            {
                "name": "Rixos The Palm Dubai",
                "position": [25.1212, 55.1535],
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
    })");

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

TEST_CASE("Compress / Decompress", "[d8u::transform]")
{
    std::vector<uint8_t> v = { 0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0 };
    std::vector<uint8_t> t = { 0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0 };

    compress(v);
    decompress(v);

    CHECK(std::equal(v.begin(), v.end(), t.begin()));


    std::vector<uint8_t> u = { 0x1B,0x54,0x59,0xC3,0xAB,0x6E,0x44,0xBF,0xA2,0xF3,0xB9,0x10,0x75,0x33,0x16,0x5D,0x23,0x97,0x21,0xB7,0x01,0xA7,0x49,0x38,0x93,0xA8,0x21,0xFB,0x23,0xC6,0xF0,0x5A };
    std::vector<uint8_t> u2 = { 0x1B,0x54,0x59,0xC3,0xAB,0x6E,0x44,0xBF,0xA2,0xF3,0xB9,0x10,0x75,0x33,0x16,0x5D,0x23,0x97,0x21,0xB7,0x01,0xA7,0x49,0x38,0x93,0xA8,0x21,0xFB,0x23,0xC6,0xF0,0x5A };

    compress(u);
    decompress(u);

    CHECK(std::equal(u.begin(), u.end(), u2.begin()));
}

TEST_CASE("AES256 encrypt/decrypt", "[d8u::transform]")
{
    std::vector<uint8_t> v = { 0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0 };
    std::vector<uint8_t> t = { 0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0 };

    Password pw(std::string_view("PASSWORD"));

    encrypt(v, pw);
    decrypt(v, pw);

    CHECK(std::equal(v.begin(), v.end(), t.begin()));


    std::vector<uint8_t> u = { 0x1B,0x54,0x59,0xC3,0xAB,0x6E,0x44,0xBF,0xA2,0xF3,0xB9,0x10,0x75,0x33,0x16,0x5D,0x23,0x97,0x21,0xB7,0x01,0xA7,0x49,0x38,0x93,0xA8,0x21,0xFB,0x23,0xC6,0xF0,0x5A };
    std::vector<uint8_t> u2 = { 0x1B,0x54,0x59,0xC3,0xAB,0x6E,0x44,0xBF,0xA2,0xF3,0xB9,0x10,0x75,0x33,0x16,0x5D,0x23,0x97,0x21,0xB7,0x01,0xA7,0x49,0x38,0x93,0xA8,0x21,0xFB,0x23,0xC6,0xF0,0x5A };

    encrypt(u, pw);
    decrypt(u, pw);

    CHECK(std::equal(u.begin(), u.end(), u2.begin()));
}

TEST_CASE("Encode/Decode", "[d8u::transform]")
{
    std::vector<uint8_t> v = { 0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0 };
    std::vector<uint8_t> t = { 0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0 };

    std::array<uint8_t, 64> domain = { 0 };

    auto key = encode(domain, v);
    CHECK(true == validate_block(v));

    decode(domain, v, key);
    CHECK(std::equal(t.begin(), t.end(), v.begin()));


    std::vector<uint8_t> u = { 0x1B,0x54,0x59,0xC3,0xAB,0x6E,0x44,0xBF,0xA2,0xF3,0xB9,0x10,0x75,0x33,0x16,0x5D,0x23,0x97,0x21,0xB7,0x01,0xA7,0x49,0x38,0x93,0xA8,0x21,0xFB,0x23,0xC6,0xF0,0x5A };
    std::vector<uint8_t> u2 = { 0x1B,0x54,0x59,0xC3,0xAB,0x6E,0x44,0xBF,0xA2,0xF3,0xB9,0x10,0x75,0x33,0x16,0x5D,0x23,0x97,0x21,0xB7,0x01,0xA7,0x49,0x38,0x93,0xA8,0x21,0xFB,0x23,0xC6,0xF0,0x5A };

    key = encode(domain, u);
    CHECK(true == validate_block(u));

    quick_decode(u, key);
    CHECK(std::equal(u2.begin(), u2.end(), u.begin()));
}