/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include "../picobench.hpp"
#include "../nlohmann_json.hpp"
#include "../picojson.h"
#include "json.hpp"

using namespace d8u::json;

namespace d8u
{
    namespace benchmark
    {
        auto const _small = R"(
        {
          "aliceblue": "#f0f8ff",
          "antiquewhite": "#faebd7",
          "aqua": "#00ffff",
          "aquamarine": "#7fffd4",
          "azure": "#f0ffff",
          "beige": "#f5f5dc",
          "bisque": "#ffe4c4",
          "black": "#000000",
          "blanchedalmond": "#ffebcd",
          "blue": "#0000ff",
          "blueviolet": "#8a2be2",
          "brown": "#a52a2a"
        })";

        auto const _medium = R"(
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

        auto const _large = R"(
        {
          "query":[{
                  "_id": {
                    "$oid": "5968dd23fc13ae04d9000001"
                  },
                  "product_name": "sildenafil citrate",
                  "supplier": "Wisozk Inc",
                  "quantity": 261,
                  "unit_cost": "$10.47"
                }, {
                  "_id": {
                    "$oid": "5968dd23fc13ae04d9000002"
                  },
                  "product_name": "Mountain Juniperus ashei",
                  "supplier": "Keebler-Hilpert",
                  "quantity": 292,
                  "unit_cost": "$8.74"
                }, {
                  "_id": {
                    "$oid": "5968dd23fc13ae04d9000003"
                  },
                  "product_name": "Dextromathorphan HBr",
                  "supplier": "Schmitt-Weissnat",
                  "quantity": 211,
                  "unit_cost": "$20.53"
                }],
            "more":[
  {
      "id": 157538,
      "date": "2017-07-21T10:30:34",
      "date_gmt": "2017-07-21T17:30:34",
      "guid": {
          "rendered": "https://www.sitepoint.com/?p=157538"
      },
      "modified": "2017-07-23T21:56:35",
      "modified_gmt": "2017-07-24T04:56:35",
      "slug": "why-the-iot-threatens-your-wordpress-site-and-how-to-fix-it",
      "status": "publish",
      "type": "post",
      "link": "https://www.sitepoint.com/why-the-iot-threatens-your-wordpress-site-and-how-to-fix-it/",
      "title": {
          "rendered": "Why the IoT Threatens Your WordPress Site (and How to Fix It) "
      },
      "content": {
   
        },
            "excerpt" : {

            },
                "author" : 72546,
                    "featured_media" : 157542,
                    "comment_status" : "open",
                    "ping_status" : "closed",
                    "sticky" : false,
                    "template" : "",
                    "format" : "standard",
                    "meta" : [] ,
                    "categories" : [
                        6132
                    ] ,
                    "tags" : [
                        1798,
                            6298
                    ]

      }
  ],
          "total": 25,
          "limit": 10,
          "skip": 0,
          "data": [{
            "_id": "5968fcad629fa84ab65a5247",
            "first_name": "Sabrina",
            "last_name": "Mayert",
            "address": "69756 Wendy Junction",
            "phone": "1-406-866-3476 x478",
            "email": "donny54@yahoo.com",
            "updatedAt": "2017-07-14T17:17:33.010Z",
            "createdAt": "2017-07-14T17:17:33.010Z",
            "__v": 0
          }, {
            "_id": "5968fcad629fa84ab65a5246",
            "first_name": "Taryn",
            "last_name": "Dietrich",
            "address": "42080 Federico Greens",
            "phone": "(197) 679-7020 x98462",
            "email": "betty_schaefer1@gmail.com",
            "updatedAt": "2017-07-14T17:17:33.006Z",
            "createdAt": "2017-07-14T17:17:33.006Z",
            "__v": 0
          }
          ],
            "as": "AS16509 Amazon.com, Inc.",
          "city": "Boardman",
          "country": "United States",
          "countryCode": "US",
          "isp": "Amazon",
          "lat": 45.8696,
          "lon": -119.688,
          "org": "Amazon",
          "region": "OR",
          "regionName": "Oregon",
          "status": "success",
          "timezone": "America\/Los_Angeles",
          "zip": "97818"
        })";

        class Small
        {
        public:
            static constexpr  const char * data = _small;
        };

        class Medium
        {
        public:
            static constexpr  const char* data = _medium;
        };

        class Large
        {
        public:
            static constexpr  const char* data = _large;
        };

        template <typename D> void nlohmann_json(picobench::state& s)
        {
            {
                picobench::scope scope(s);

                for (auto _ : s)
                    auto j3 = nlohmann::json::parse(D::data);
            }
        }

        template <typename D> void pico_json(picobench::state& s)
        {
            {
                picobench::scope scope(s);

                for (auto _ : s)
                {
                    picojson::value v;
                    std::string err = picojson::parse(v, D::data);
                }
            }
        }

        template <typename T,typename D> void json(picobench::state& s)
        {
            {
                picobench::scope scope(s);

                for (auto _ : s)
                    T s(D::data);
            }
        }


        PICOBENCH_SUITE("Small JSON");

        auto json_small8 = json<JsonReaderS,Small>;
        auto json_small16 = json<JsonReader, Small>;
        auto json_small32 = json<JsonReaderL, Small>;
        auto json_nlohmann = nlohmann_json<Small>;
        auto json_pico = pico_json<Small>;

        PICOBENCH(json_small8);
        PICOBENCH(json_small16);
        PICOBENCH(json_small32);
        PICOBENCH(json_nlohmann);
        PICOBENCH(json_pico);



        PICOBENCH_SUITE("Medium JSON");

        auto json_medium16 = json<JsonReader, Medium>;
        auto json_medium32 = json<JsonReaderL, Medium>;
        auto json_mnlohmann = nlohmann_json<Medium>;
        auto json_mpico = pico_json<Medium>;

        PICOBENCH(json_medium16);
        PICOBENCH(json_medium32);
        PICOBENCH(json_mnlohmann);
        PICOBENCH(json_mpico);



        PICOBENCH_SUITE("Large JSON");

        auto json_large16 = json<JsonReader, Large>;
        auto json_large32 = json<JsonReaderL, Large>;
        auto json_lnlohmann = nlohmann_json<Large>;
        auto json_lpico = pico_json<Large>;

        PICOBENCH(json_large16);
        PICOBENCH(json_large32);
        PICOBENCH(json_lnlohmann);
        PICOBENCH(json_lpico);

        /*
            TODO Access Times
        */
    }
}