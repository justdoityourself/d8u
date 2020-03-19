# d8u
C++ Utilities

[![D8U](http://img.youtube.com/vi/k3paPWOxjRg/0.jpg)](https://www.youtube.com/watch?v=k3paPWOxjRg "D8U")

## json.hpp

Very fast very light weight json reader.

JsonReader builds an index of the json buffer that is passed in. It doesn't copy or move the data.
It provides an interface to find any value/array/object/string in the json string, conversion from a string type happens when the object is accessed.

```c++
#include "d8u/json.hpp"

using namespace d8u::json;

...

auto string json = R"(
{
  "value":15,
  "object":
  {
    "nested":42
  },
  "array":
  [
  ],
  "string":"This is a string"
}
)";
JsonReader root(json);

string_view v = root["value"]; // [] returns string segments... v = "15"
string_view v1 = root[0]; // you can use strings or integer indexes to access values/objects, integers are of course faster.
int v2 = root["value"]; // This converts the string to an int.

auto nested_object = root("object"); // () returns an object type.

int nested_int = root("object")["nested"]; // ether of these work
int nested_int1 = nested_object["nested"]; // this one performs better if you are going to access this root more than once.

```

#Benchmarks

There are 3 variations of JsonReader: JsonReaderS, JsonReader and JsonReaderL. Each of them has different maximum json string parse lengths. This is designed to further optimise performance for your use case, if you know you the max length of the json string you are parsing you can choose the one that best matches your situation.

Compared to picojson and nlohmannjson.

Small JSON:
===============================================================================
   Name (baseline is *)   |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
===============================================================================
            json_small8 * |       8 |     0.026 |    3262 |      - |   306513.4
             json_small16 |       8 |     0.036 |    4512 |  1.383 |   221606.6
             json_small32 |       8 |     0.107 |   13325 |  4.084 |    75046.9
            json_nlohmann |       8 |     0.329 |   41112 | 12.602 |    24323.5
                json_pico |       8 |     0.277 |   34637 | 10.617 |    28870.4
            json_small8 * |      64 |     0.183 |    2857 |      - |   349918.0
             json_small16 |      64 |     0.204 |    3190 |  1.116 |   313418.2
             json_small32 |      64 |     0.814 |   12717 |  4.450 |    78633.7
            json_nlohmann |      64 |     2.557 |   39948 | 13.979 |    25032.3
                json_pico |      64 |     2.306 |   36026 | 12.606 |    27757.3
            json_small8 * |     512 |     1.685 |    3290 |      - |   303893.6
             json_small16 |     512 |     2.034 |    3972 |  1.207 |   251745.5
             json_small32 |     512 |     7.906 |   15441 |  4.693 |    64760.1
            json_nlohmann |     512 |    21.210 |   41425 | 12.589 |    24139.6
                json_pico |     512 |    18.060 |   35274 | 10.720 |    28349.5
            json_small8 * |    4096 |    12.743 |    3111 |      - |   321421.3
             json_small16 |    4096 |    14.985 |    3658 |  1.176 |   273349.1
             json_small32 |    4096 |    58.061 |   14175 |  4.556 |    70546.7
            json_nlohmann |    4096 |   184.209 |   44972 | 14.455 |    22235.6
                json_pico |    4096 |   153.983 |   37593 | 12.083 |    26600.3
            json_small8 * |    8192 |    28.156 |    3436 |      - |   290954.6
             json_small16 |    8192 |    31.038 |    3788 |  1.102 |   263937.1
             json_small32 |    8192 |   121.614 |   14845 |  4.319 |    67360.5
            json_nlohmann |    8192 |   350.203 |   42749 | 12.438 |    23392.2
                json_pico |    8192 |   304.601 |   37182 | 10.818 |    26894.2
===============================================================================
Medium JSON:
===============================================================================
   Name (baseline is *)   |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
===============================================================================
          json_medium16 * |       8 |     0.075 |    9437 |      - |   105960.3
            json_medium32 |       8 |     0.143 |   17912 |  1.898 |    55826.9
           json_mnlohmann |       8 |     1.390 |  173775 | 18.413 |     5754.6
               json_mpico |       8 |     1.269 |  158575 | 16.803 |     6306.2
          json_medium16 * |      64 |     0.684 |   10685 |      - |    93580.9
            json_medium32 |      64 |     1.169 |   18267 |  1.709 |    54743.0
           json_mnlohmann |      64 |     9.628 |  150431 | 14.077 |     6647.6
               json_mpico |      64 |    12.891 |  201417 | 18.849 |     4964.8
          json_medium16 * |     512 |     5.626 |   10988 |      - |    91001.2
            json_medium32 |     512 |    10.703 |   20904 |  1.902 |    47836.6
           json_mnlohmann |     512 |    94.193 |  183970 | 16.742 |     5435.6
               json_mpico |     512 |    88.144 |  172156 | 15.666 |     5808.7
          json_medium16 * |    4096 |    47.693 |   11643 |      - |    85882.6
            json_medium32 |    4096 |    95.442 |   23301 |  2.001 |    42916.1
           json_mnlohmann |    4096 |   695.396 |  169774 | 14.581 |     5890.2
               json_mpico |    4096 |   755.140 |  184360 | 15.833 |     5424.2
          json_medium16 * |    8192 |   106.256 |   12970 |      - |    77096.7
            json_medium32 |    8192 |   188.279 |   22983 |  1.772 |    43509.8
           json_mnlohmann |    8192 |  1424.523 |  173891 | 13.406 |     5750.7
               json_mpico |    8192 |  1472.476 |  179745 | 13.858 |     5563.4
===============================================================================
Large JSON:
===============================================================================
   Name (baseline is *)   |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
===============================================================================
           json_large16 * |       8 |     0.197 |   24637 |      - |    40588.5
             json_large32 |       8 |     0.364 |   45512 |  1.847 |    21972.0
           json_lnlohmann |       8 |     2.594 |  324300 | 13.163 |     3083.6
               json_lpico |       8 |     1.886 |  235775 |  9.570 |     4241.3
           json_large16 * |      64 |     1.722 |   26898 |      - |    37176.9
             json_large32 |      64 |     6.445 |  100703 |  3.744 |     9930.2
           json_lnlohmann |      64 |    19.766 |  308837 | 11.482 |     3237.9
               json_lpico |      64 |    18.306 |  286034 | 10.634 |     3496.1
           json_large16 * |     512 |    15.575 |   30419 |      - |    32873.4
             json_large32 |     512 |    19.237 |   37571 |  1.235 |    26615.8
           json_lnlohmann |     512 |   155.744 |  304187 | 10.000 |     3287.4
               json_lpico |     512 |   151.220 |  295351 |  9.709 |     3385.8
           json_large16 * |    4096 |   124.055 |   30286 |      - |    33017.5
             json_large32 |    4096 |   174.589 |   42624 |  1.407 |    23460.9
           json_lnlohmann |    4096 |  1296.124 |  316436 | 10.448 |     3160.2
               json_lpico |    4096 |  1107.728 |  270441 |  8.929 |     3697.7
           json_large16 * |    8192 |   256.940 |   31364 |      - |    31882.9
             json_large32 |    8192 |   368.757 |   45014 |  1.435 |    22215.2
           json_lnlohmann |    8192 |  2576.257 |  314484 | 10.027 |     3179.8
               json_lpico |    8192 |  2250.789 |  274754 |  8.760 |     3639.6
===============================================================================
