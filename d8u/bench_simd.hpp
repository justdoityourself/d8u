/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include "../picobench.hpp"
#include "bench_data.hpp"
#include "json_simd.hpp"

namespace d8u
{
    namespace benchmark
    {
        template <typename D> void count_native(picobench::state& s)
        {
            D d;
            size_t count = 0;
            {
                picobench::scope scope(s);

                for (auto _ : s)
                    count = d8u::count_chars_8(d.data,':');
            }

            std::cout << count << std::endl;
        }

        template <typename D> void count_128(picobench::state& s)
        {
            D d;
            size_t count = 0;
            {
                picobench::scope scope(s);

                for (auto _ : s)
                    count = d8u::count_chars_128(d.data, ':');
            }

            std::cout << count << std::endl;
        }

        template <typename D> void count_256(picobench::state& s)
        {
            D d;
            size_t count = 0;
            {
                picobench::scope scope(s);

                for (auto _ : s)
                    count = d8u::count_chars_256(d.data, ':');
            }

            std::cout << count << std::endl;
        }



        PICOBENCH_SUITE("Small JSON");

        auto c8_sm = count_native<Small>;
        auto c128_sm = count_128<Small>;

        auto c256_sm = count_256<Small>;


        PICOBENCH(c8_sm);
        PICOBENCH(c128_sm);
        PICOBENCH(c256_sm);


        PICOBENCH_SUITE("Medium JSON");

        auto c8_m = count_native<Medium>;
        auto c128_m = count_128<Medium>;

        auto c256_m = count_256<Medium>;
        

        PICOBENCH(c8_m);
        PICOBENCH(c128_m);
        PICOBENCH(c256_m);


        PICOBENCH_SUITE("Large JSON");

        auto c8_l = count_native<Large>;
        auto c128_l = count_128<Large>;
        auto c256_l = count_256<Large>;


        PICOBENCH(c8_l).iterations({ 4 });
        PICOBENCH(c128_l).iterations({ 4 });
        PICOBENCH(c256_l).iterations({ 4 });

        PICOBENCH_SUITE("Huge JSON");

        auto c8_h = count_native<Huge>;
        auto c128_h = count_128<Huge>;
        auto c256_h = count_256<Huge>;

        PICOBENCH(c8_h).iterations({ 4 });
        PICOBENCH(c128_h).iterations({ 4 });
        PICOBENCH(c256_h).iterations({ 4 });



        PICOBENCH_SUITE("Giant JSON");

        auto c8_g = count_native<Giant>;
        auto c128_g = count_128<Giant>;
        auto c256_g = count_256<Giant>;

        PICOBENCH(c8_g).iterations({ 4 });
        PICOBENCH(c128_g).iterations({ 4 });
        PICOBENCH(c256_g).iterations({ 4 });



        PICOBENCH_SUITE("MB25 JSON");

        auto c8_25 = count_native<MB25>;
        auto c128_25 = count_128<MB25>;
        auto c256_25 = count_256<MB25>;

        PICOBENCH(c8_25).iterations({ 4 });
        PICOBENCH(c128_25).iterations({ 4 });
        PICOBENCH(c256_25).iterations({ 4 });



        PICOBENCH_SUITE("MB100 JSON");

        auto c8_100 = count_native<MB100>;
        auto c128_100 = count_128<MB100>;
        auto c256_100 = count_256<MB100>;

        PICOBENCH(c8_100).iterations({ 4 });
        PICOBENCH(c128_100).iterations({ 4 });
        PICOBENCH(c256_100).iterations({ 4 });
    }
}