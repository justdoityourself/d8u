/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */



#ifdef TEST_RUNNER


#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include "d8u/test.hpp"
#include "d8u/test_crypto.hpp"

int main(int argc, char* argv[])
{
    return Catch::Session().run(argc, argv);
}


#endif //TEST_RUNNER



#ifdef BENCHMARK_RUNNER


#define PICOBENCH_IMPLEMENT
#include "picobench.hpp"
#include "d8u/benchmark.hpp"

int main(int argc, char* argv[])
{
    picobench::runner runner;
    return runner.run();
}


#endif //BENCHMARK_RUNNER


#ifdef BENCHMARK_SIMD


#define PICOBENCH_IMPLEMENT
#include "picobench.hpp"
#include "d8u/bench_simd.hpp"

int main(int argc, char* argv[])
{
    picobench::runner runner;
    return runner.run();
}


#endif //BENCHMARK_SIMD



#if ! defined(BENCHMARK_RUNNER) && ! defined(TEST_RUNNER) && ! defined(BENCHMARK_SIMD)


int main(int argc, char* argv[])
{
    return 0;
}


#endif //! defined(BENCHMARK_RUNNER) && ! defined(TEST_RUNNER)


