[![Standard](https://img.shields.io/badge/C%2B%2B-11/14/17/20-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](https://opensource.org/licenses/MIT)
[![Try it on godbolt online](https://img.shields.io/badge/on-godbolt-blue.svg)](https://godbolt.org/z/ndE9hYfPx) 

CUDA-Compatible `std::span` Implementation for C++11 and Later
===============================================================

This repository contains a single-header, CUDA-compatible implementation of C++20's `std::span`,
adapted for both host and device use. It conforms to the C++20 specification and works with C++11 and later,
falling back to available features per standard version.

This version improves upon the original by:
- Supporting CUDA (host/device annotations included)
- Using modern CMake conventions
- Replacing the Boost Software License with the MIT License
- Living in the `dd` namespace, header: `include/dd/span.hpp`

It differs from the Microsoft GSL implementation by being header-only, minimal, and compatible with CUDA kernels.

Usage
-----

Copy `include/dd/span.hpp` into your project and `#include <dd/span.hpp>`.

You can override the namespace by defining the macro `DD_SPAN_NAMESPACE_NAME` before including the header.

Compatibility
-------------

Tested with:
- CUDA 11+
- GCC 5+
- Clang 3.5+
- MSVC 2015+

Also works in device code and supports `__host__ __device__` annotations.

License
-------

Licensed under the [MIT License](https://opensource.org/licenses/MIT).

Documentation
-------------

Refer to [`std::span` on cppreference](https://en.cppreference.com/w/cpp/container/span) for specification details.

Key Features
------------

### CUDA Support

All API functions are annotated with `__host__ __device__` where appropriate via `DD_SPAN_API`.

### Contract Checking

Bounds checking is controlled via the following macros:

- `DD_SPAN_THROW_ON_CONTRACT_VIOLATION`: Throws `contract_violation_error` (inherits `std::logic_error`)
- `DD_SPAN_TERMINATE_ON_CONTRACT_VIOLATION`: Calls `std::terminate()`
- `DD_SPAN_NO_CONTRACT_CHECKING`: Disables contract checks entirely

Defaults:
- Debug builds (`!NDEBUG`) use termination
- Release builds (`NDEBUG`) disable checks

### Constexpr Support

Fully `constexpr` under C++17 and later. Earlier versions are best-effort.

### Span Deduction and `make_span`

Constructor guides and `make_span()` helpers are available:

```cpp
constexpr int arr[] = {1, 2, 3};
std::array<int, 3> std_arr{1, 2, 3};
std::vector<int> vec{1, 2, 3};

auto s1 = make_span(arr);      // span<const int, 3>
auto s2 = make_span(std_arr);  // span<int, 3>
auto s3 = make_span(vec);      // span<const int, dynamic_extent>
```

Examples
--------

```cpp
#include <dd/span.hpp>

__global__ void kernel(dd::span<const float> input) {
    int i = threadIdx.x;
    if (i < input.size()) {
        printf("%f\n", input[i]);
    }
}
```

Alternatives
------------

* [Microsoft/GSL](https://github.com/Microsoft/GSL): Official proposal source.
* [martinmoene/span_lite](https://github.com/martinmoene/span-lite): C++98-compatible span implementation.
* [[![Standard](https://img.shields.io/badge/C%2B%2B-11/14/17/20-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](https://opensource.org/licenses/MIT)
[![Try it on godbolt online](https://img.shields.io/badge/on-godbolt-blue.svg)](https://godbolt.org/z/-vlZZR) 

CUDA-Compatible `std::span` Implementation for C++11 and Later
===============================================================

This repository contains a single-header, CUDA-compatible implementation of C++20's `std::span`,
adapted for both host and device use. It conforms to the C++20 specification and works with C++11 and later,
falling back to available features per standard version.

This version improves upon the original by:
- Supporting CUDA (host/device annotations included)
- Using modern CMake conventions
- Replacing the Boost Software License with the MIT License
- Living in the `dd` namespace, header: `include/dd/span.hpp`

It differs from the Microsoft GSL implementation by being header-only, minimal, and compatible with CUDA kernels.

Usage
-----

Copy `include/dd/span.hpp` into your project and `#include <dd/span.hpp>`.

You can override the namespace by defining the macro `DD_SPAN_NAMESPACE_NAME` before including the header.

Compatibility
-------------

Tested with:
- CUDA 11+
- GCC 5+
- Clang 3.5+
- MSVC 2015+

Also works in device code and supports `__host__ __device__` annotations.

License
-------

Licensed under the [MIT License](https://opensource.org/licenses/MIT).

Documentation
-------------

Refer to [`std::span` on cppreference](https://en.cppreference.com/w/cpp/container/span) for specification details.

Key Features
------------

### CUDA Support

All API functions are annotated with `__host__ __device__` where appropriate via `DD_SPAN_API`.

### Contract Checking

Bounds checking is controlled via the following macros:

- `DD_SPAN_THROW_ON_CONTRACT_VIOLATION`: Throws `contract_violation_error` (inherits `std::logic_error`)
- `DD_SPAN_TERMINATE_ON_CONTRACT_VIOLATION`: Calls `std::terminate()`
- `DD_SPAN_NO_CONTRACT_CHECKING`: Disables contract checks entirely

Defaults:
- Debug builds (`!NDEBUG`) use termination
- Release builds (`NDEBUG`) disable checks

### Constexpr Support

Fully `constexpr` under C++17 and later. Earlier versions are best-effort.

### Span Deduction and `make_span`

Constructor guides and `make_span()` helpers are available:

```cpp
constexpr int arr[] = {1, 2, 3};
std::array<int, 3> std_arr{1, 2, 3};
std::vector<int> vec{1, 2, 3};

auto s1 = make_span(arr);      // span<const int, 3>
auto s2 = make_span(std_arr);  // span<int, 3>
auto s3 = make_span(vec);      // span<const int, dynamic_extent>
```

Examples
--------

```cpp
#include <dd/span.hpp>

__global__ void kernel(float * ptr, int size) {
    span<const float> input (ptr, size);
    int i = threadIdx.x;
    if (i < input.size()) {
        printf("%f\n", input[i]);
    }
}
```

Alternatives
------------

* [Microsoft/GSL](https://github.com/Microsoft/GSL): Official proposal source.
* [martinmoene/span_lite](https://github.com/martinmoene/span-lite): C++98-compatible span implementation.
]
* [tcbrindle/span](https://github.com/tcbrindle/span): largely inspired this project. However is not CUDA compatible.