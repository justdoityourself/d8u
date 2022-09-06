/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include "../mio.hpp"


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
            const std::string_view data = _small;
        };

        class Medium
        {
        public:
            const std::string_view data = _medium;
        };

        class Large
        {
        public:
            const std::string_view data = _large;
        };

        class Huge
        {
        public:
            Huge()
                : file("small.json")
                , data(file.data(),file.size()) { }

            mio::mmap_source file;
            const std::string_view data;
        };

        class Giant
        {
        public:
            Giant()
                : file("big.json")
                , data(file.data(),file.size()) { }

            mio::mmap_source file;
            const std::string_view data;
        };

        class MB25
        {
        public:
            MB25()
                : file("mjs.json")
                , data(file.data(), file.size()) { }

            mio::mmap_source file;
            const std::string_view data;
        };

        class MB100
        {
        public:
            MB100()
                : file("100MB.json")
                , data(file.data(), file.size()) { }

            mio::mmap_source file;
            const std::string_view data;
        };

        class TTT
        {
        public:
            TTT()
                : file("target2.json")
                , data(file.data(), file.size()) { }

            mio::mmap_source file;
            const std::string_view data;
        };
    }
}