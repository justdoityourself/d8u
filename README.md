# d8u
C++ Utilities

#json.hpp

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
int nested_int1 = nested_object["nested"]; // this one performs better

```
