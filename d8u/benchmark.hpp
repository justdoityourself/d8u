/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include "../picobench.hpp"
#include "../nlohmann_json.hpp"
#include "../picojson.h"
#include "../simdjson.h"
#include "json.hpp"
#include "json_stack.hpp"
#include "json_simd.hpp"
#include "bench_data.hpp"

using namespace d8u::json;

namespace d8u
{
    namespace benchmark
    {
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

        template <typename D> void jscntctrl(picobench::state& s)
        {
            D d;
            size_t total = 0;
            {
                picobench::scope scope(s);

                for (auto _ : s)
                    total = JsonCountControl(d.data).count;
            }
        }

        template <typename D> void jsactrl(picobench::state& s)
        {
            D d;
            size_t total = 0;
            {
                picobench::scope scope(s);

                for (auto _ : s)
                    total = JsonAvx2ParseKVAlign64(d.data.data());
            }

            std::cout << "TTT" << total << std::endl;
        }

        template <typename D> void jsa16ctrl(picobench::state& s)
        {
            D d;
            size_t total = 0;
            {
                picobench::scope scope(s);

                for (auto _ : s)
                    total = JsonAvxParseKVAlign16(d.data.data());
            }

            std::cout << "TTT" << total << std::endl;
        }

        template <typename D> void jsoncontrol(picobench::state& s)
        {
            D d;
            size_t total = 0;
            {
                picobench::scope scope(s);

                for (auto _ : s)
                    total = JsonObjectParse(d.data);
            }

            std::cout << "CTL " << total << " " << d.data.size() << std::endl;
        }

        template <typename D> void partialcontrol(picobench::state& s)
        {
            D d;
            size_t total = 0;
            {
                picobench::scope scope(s);

                for (auto _ : s)
                    total = PartialAvxObjectParse(d.data);
            }

            std::cout << "PAVX " << total << " " << d.data.size() << std::endl;
        }

        template <typename D> void avxcontrol(picobench::state& s)
        {
            D d;
            size_t total = 0;
            {
                picobench::scope scope(s);

                for (auto _ : s)
                    total = JsonAvx2Parse(d.data.data());
            }

            std::cout << "AVX " << total << " " << d.data.size() << std::endl;
        }

        template <typename D, typename int_t, size_t max_c> void jsonfixedseq2(picobench::state& s)
        {
            D d;
            {
                picobench::scope scope(s);

                for (auto _ : s)
                    JsonFixedSeq2<int_t, max_c> q(d.data);
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

        template <typename D, typename int_t, size_t depth_c, size_t map_c> void jsondh(picobench::state& s)
        {
            D d;
            {
                picobench::scope scope(s);

                for (auto _ : s)
                    JsonFixedRng<depth_c, map_c, int_t, DoubleHash, false> q(d.data);
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

        template <typename D> void simd_json(picobench::state& s)
        {
            D d;
            simdjson::ondemand::parser parser;
            simdjson::ondemand::document doc;
            simdjson::simdjson_result error = 0;
            simdjson::padded_string ps(d.data);
            {
                picobench::scope scope(s);

                for (auto _ : s)
                    error = parser.iterate(ps).get(doc);
            }
            
            std::cout << error << std::endl;
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
        auto json_mjsoncontrol = jsoncontrol<Medium>;
        auto json_mavxcontrol = avxcontrol<Medium>;
        auto json_mnlohmann = nlohmann_json<Medium>;
        auto json_mpico = pico_json<Medium>;

        PICOBENCH(json_medium16);
        PICOBENCH(json_medium32);
        PICOBENCH(json_medium64);
        //PICOBENCH(json_mediumstack);
        //PICOBENCH(json_mediumseqf);
        PICOBENCH(json_mediumseqf2);
        PICOBENCH(json_mediumrngf);
        PICOBENCH(json_mjsoncontrol);
        PICOBENCH(json_mavxcontrol);
        PICOBENCH(json_mnlohmann);
        PICOBENCH(json_mpico);



        PICOBENCH_SUITE("Large JSON");

        auto json_large16 = json<JsonReader, Large>;
        auto json_large32 = json<JsonReaderL, Large>;
        auto json_large64 = json<JsonReaderH, Large>;
        auto json_largeseqf = jsonfixedseq<Large, 128>;
        auto json_largeseqf2 = jsonfixedseq2<Large, uint16_t, 128>;
        auto json_ljsoncontrol = jsoncontrol<Large>;
        auto json_ljpcontrol = partialcontrol<Large>;
        auto json_lavxcontrol = avxcontrol<Large>;
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
        PICOBENCH(json_ljsoncontrol);
        PICOBENCH(json_ljpcontrol);
        PICOBENCH(json_lavxcontrol);
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


        auto json_giantstack = stack<Giant>;
        auto json_giantpath= jsonpath<Giant,int32_t,64,1024*1024>;
        auto json_giantpath2 = jsondh<Giant, int32_t, 64, 1024 * 1024>;
        auto json_giantselect = jsonselect<Giant, int32_t, 64, 1024 * 1024>;
        auto json_gjsoncontrol = jsoncontrol<Giant>;
        auto json_gpcontrol = partialcontrol<Giant>;
        auto json_gavxcontrol = avxcontrol<Giant>;
        auto json_gnlohmann = nlohmann_json<Giant>;
        auto json_gpico = pico_json<Giant>;
        auto json_gsimd = simd_json<Giant>;

        PICOBENCH(json_giantstack).iterations({ 4 });
        PICOBENCH(json_giantpath).iterations({ 4 });
        PICOBENCH(json_giantpath2).iterations({ 4 });
        PICOBENCH(json_gjsoncontrol).iterations({ 4 });
        PICOBENCH(json_gpcontrol).iterations({ 4 });
        PICOBENCH(json_gavxcontrol).iterations({ 4 });
        PICOBENCH(json_giantselect).iterations({ 4 });
        PICOBENCH(json_gnlohmann).iterations({ 4 });
        PICOBENCH(json_gsimd).iterations({ 4 });
        PICOBENCH(json_gpico).iterations({ 4 });



        PICOBENCH_SUITE("MB25 JSON");


        auto json_25stack = stack<MB25>;
        auto json_25path = jsonpath<MB25, int32_t, 256, 6*1024 * 1024>;
        auto json_25path2 = jsondh<MB25, int32_t, 256, 6 * 1024 * 1024>;
        //auto json_25select = jsonselect<MB25, int32_t, 256, 8*1024 * 1024>;
        auto json_25control = jsoncontrol<MB25>;
        auto json_25pcontrol = partialcontrol<MB25>;
        auto json_25cc = jscntctrl<MB25>;
        auto json_actc = jsactrl<MB25>;
        auto json_a16ctc = jsa16ctrl<MB25>;
        auto json_25avxcontrol = avxcontrol<MB25>;
        auto json_25nlohmann = nlohmann_json<MB25>;
        auto json_25pico = pico_json<MB25>;
        auto json_25simd = simd_json<MB25>;

        PICOBENCH(json_25stack).iterations({ 1 });
        PICOBENCH(json_25path).iterations({ 1 });
        PICOBENCH(json_25path2).iterations({ 1 });
        //PICOBENCH(json_25select).iterations({ 1 });
        PICOBENCH(json_25control).iterations({ 1 });
        PICOBENCH(json_25pcontrol).iterations({ 1 });
        PICOBENCH(json_25cc).iterations({ 1 });
        PICOBENCH(json_actc).iterations({ 1 });
        PICOBENCH(json_a16ctc).iterations({ 1 });
        PICOBENCH(json_25avxcontrol).iterations({ 1 });
        PICOBENCH(json_25nlohmann).iterations({ 1 });
        PICOBENCH(json_25pico).iterations({ 1 });
        PICOBENCH(json_25simd).iterations({ 1 });
    }
}