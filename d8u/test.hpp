/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <string>

#include "../catch.hpp"
#include "json.hpp"

using namespace d8u::json;

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