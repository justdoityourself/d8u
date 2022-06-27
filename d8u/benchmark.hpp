/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include "../picobench.hpp"
#include "../nlohmann_json.hpp"
#include "../picojson.h"
#include "json.hpp"
#include "json_stack.hpp"

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
                , data(file.data(),file.size())
            {

            }

            mio::mmap_source file;
            const std::string_view data;
        };

        class Giant
        {
        public:
            Giant()
                : file("big.json")
                , data(file.data(),file.size())
            {

            }

            mio::mmap_source file;
            const std::string_view data;
        };

        template <typename D> void nlohmann_json(picobench::state& s)
        {
            D d;
            {
                picobench::scope scope(s);

                for (auto _ : s)
                    auto j3 = nlohmann::json::parse(d.data);
            }
        }

        template <typename D> void pico_json(picobench::state& s)
        {
            D d;
            {
                picobench::scope scope(s);

                for (auto _ : s)
                {
                    picojson::value v;
                    std::string err = picojson::parse(v, d.data.data());
                }
            }
        }

        template <typename T,typename D> void json(picobench::state& s)
        {
            D d;
            {
                picobench::scope scope(s);

                for (auto _ : s)
                    T s(d.data);
            }
        }

        template <typename D> void stack(picobench::state& s)
        {
            D d;
            {
                picobench::scope scope(s);

                for (auto _ : s)
                    StreamJsonNoRecursion(d.data, [](auto,auto, auto, auto) {});
            }
        }

        template <typename D> void stackmirror(picobench::state& s)
        {
            D d;
            {
                picobench::scope scope(s);

                for (auto _ : s)
                    StreamJsonMirror(d.data);
            }
        }

        template <typename D> void stackhash(picobench::state& s)
        {
            D d;
            JsonStackHash jh;
            {
                picobench::scope scope(s);

                for (auto _ : s)
                    jh.Add(d.data);
            }
        }

        template <typename D> void jsonseq(picobench::state& s)
        {
            D d;
            {
                picobench::scope scope(s);

                for (auto _ : s)
                    JsonSeq q(d.data);
            }
        }

        template <typename D,size_t max_c> void jsonfixedseq(picobench::state& s)
        {
            D d;
            {
                picobench::scope scope(s);

                for (auto _ : s)
                    JsonFixedSeq<max_c> q(d.data);
            }
        }

        template <typename D, typename int_t, size_t max_c> void jsonfixedseq2(picobench::state& s)
        {
            D d;
            {
                picobench::scope scope(s);

                for (auto _ : s)
                    JsonFixedSeq2<int_t,max_c> q(d.data);
            }
        }

        template <typename D, typename int_t, size_t depth_c, size_t map_c> void jsonfixedrng(picobench::state& s)
        {
            D d;
            {
                picobench::scope scope(s);

                for (auto _ : s)
                    JsonFixedRng<depth_c,map_c,int_t> q(d.data);
            }
        }

        template <typename D, typename int_t, size_t depth_c, size_t map_c> void jsonpath(picobench::state& s)
        {
            D d;
            {
                picobench::scope scope(s);

                for (auto _ : s)
                    JsonFixedRng<depth_c, map_c, int_t,FastHash,false> q(d.data);
            }
        }

        template <typename D, typename int_t, size_t depth_c, size_t map_c> void jsonselect(picobench::state& s)
        {
            D d;
            {
                picobench::scope scope(s);

                for (auto _ : s)
                    JsonFixedRng<depth_c, map_c, int_t, FastHash, true> q(d.data);
            }
        }

        template <typename D> void jsonrep(picobench::state& s)
        {
            D d;
            JsonSeq q(d.data);
            {
                picobench::scope scope(s);

                for (auto _ : s)
                    q.Stream([](auto, auto, auto, auto) {});
            }
        }


        PICOBENCH_SUITE("Small JSON");

        auto json_small8 = json<JsonReaderS,Small>;
        auto json_small16 = json<JsonReader, Small>;
        auto json_small32 = json<JsonReaderL, Small>;
        auto json_small64 = json<JsonReaderH, Small>;
        auto json_smallstack = stack<Small>;
        auto json_smallmirror = stackmirror<Small>;
        auto json_smallhash = stackhash<Small>;
        auto json_smallrep = jsonrep<Small>;
        auto json_smallseq = jsonseq<Small>;
        auto json_smallseqf = jsonfixedseq<Small,64>;
        auto json_smallseqf2 = jsonfixedseq2<Small, uint16_t, 64>;
        auto json_smallrngf = jsonfixedrng<Small, uint16_t, 16, 64>;
        auto json_nlohmann = nlohmann_json<Small>;
        auto json_pico = pico_json<Small>;

        PICOBENCH(json_small8);
        PICOBENCH(json_small16);
        PICOBENCH(json_small32);
        PICOBENCH(json_small64);
        //PICOBENCH(json_smallstack);
        //PICOBENCH(json_smallmirror);
        //PICOBENCH(json_smallhash);
        //PICOBENCH(json_smallrep);
       // PICOBENCH(json_smallseq);
        //PICOBENCH(json_smallseqf);
        PICOBENCH(json_smallseqf2);
        PICOBENCH(json_smallrngf);
        PICOBENCH(json_nlohmann);
        PICOBENCH(json_pico);



        PICOBENCH_SUITE("Medium JSON");

        auto json_medium16 = json<JsonReader, Medium>;
        auto json_medium32 = json<JsonReaderL, Medium>;
        auto json_medium64 = json<JsonReaderH, Medium>;
        auto json_mediumstack = stack<Medium>;
        auto json_mediumseqf = jsonfixedseq<Medium, 128>;
        auto json_mediumseqf2 = jsonfixedseq2<Medium, uint16_t, 128>;
        auto json_mediumrngf = jsonfixedrng<Medium, uint16_t, 16, 128>;
        auto json_mnlohmann = nlohmann_json<Medium>;
        auto json_mpico = pico_json<Medium>;

        PICOBENCH(json_medium16);
        PICOBENCH(json_medium32);
        PICOBENCH(json_medium64);
        //PICOBENCH(json_mediumstack);
        //PICOBENCH(json_mediumseqf);
        PICOBENCH(json_mediumseqf2);
        PICOBENCH(json_mediumrngf);
        PICOBENCH(json_mnlohmann);
        PICOBENCH(json_mpico);



        PICOBENCH_SUITE("Large JSON");

        auto json_large16 = json<JsonReader, Large>;
        auto json_large32 = json<JsonReaderL, Large>;
        auto json_large64 = json<JsonReaderH, Large>;
        auto json_largeseqf = jsonfixedseq<Large, 128>;
        auto json_largeseqf2 = jsonfixedseq2<Large, uint16_t, 128>;
        auto json_largerngf = jsonfixedrng<Large, uint16_t, 16, 128>;
        auto json_largestack = stack<Large>;
        auto json_lnlohmann = nlohmann_json<Large>;
        auto json_lpico = pico_json<Large>;

        PICOBENCH(json_large16);
        PICOBENCH(json_large32);
        PICOBENCH(json_large64);
        //PICOBENCH(json_largestack);
        //PICOBENCH(json_largeseqf);
        PICOBENCH(json_largeseqf2);
        PICOBENCH(json_largerngf);
        PICOBENCH(json_lnlohmann);
        PICOBENCH(json_lpico);



        PICOBENCH_SUITE("Huge JSON");

        //auto json_huge16 = json<JsonReader, Huge>;
        //auto json_huge32 = json<JsonReaderL, Huge>;
        //auto json_huge64 = json<JsonReaderH, Huge>;
        auto json_hugestack = stack<Huge>;
        auto json_hnlohmann = nlohmann_json<Huge>;
        auto json_hpico = pico_json<Huge>;

        //PICOBENCH(json_huge16);
        //PICOBENCH(json_huge32);
        //PICOBENCH(json_huge64);
        //PICOBENCH(json_hugestack).iterations({ 1,4 });
        PICOBENCH(json_hnlohmann).iterations({ 1,4 });
        //PICOBENCH(json_hpico);



        PICOBENCH_SUITE("Giant JSON");

        //auto json_giant16 = json<JsonReader, Giant>;
        //auto json_giant32 = json<JsonReaderL, Giant>;
        //auto json_giant64 = json<JsonReaderH, Giant>;
        auto json_giantstack = stack<Giant>;
        auto json_giantpath= jsonpath<Giant,int32_t,64,1024*1024>;
        auto json_giantselect = jsonselect<Giant, int32_t, 64, 1024 * 1024>;
        auto json_gnlohmann = nlohmann_json<Giant>;
        auto json_gpico = pico_json<Giant>;

        //PICOBENCH(json_giant16);
        //PICOBENCH(json_giant32);
        //PICOBENCH(json_giant64);
        //PICOBENCH(json_giantstack).iterations({ 1 });
        PICOBENCH(json_giantpath).iterations({ 1 });
        PICOBENCH(json_giantselect).iterations({ 1 });
        PICOBENCH(json_gnlohmann).iterations({ 1 });
        //PICOBENCH(json_gpico);



        /*
            TODO Access Times
        */
    }
}