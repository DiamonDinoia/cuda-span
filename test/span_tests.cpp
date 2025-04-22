#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <array>
#include <initializer_list>
#include <type_traits>
#include <algorithm>
#include <deque>

#define DD_SPAN_THROW_ON_CONTRACT_VIOLATION
#include "dd/span.hpp"

using dd::span;
using dd::make_span;
using dd::dynamic_extent;
using dd::get;
using dd::byte;
using dd::contract_violation_error;

// Compile-time assertions
static_assert(std::tuple_size<span<int,3>>::value == 3, "tuple_size mismatch");
static_assert(std::is_same<std::tuple_element<1, span<int,3>>::type, int>::value, "tuple_element mismatch");
static_assert(std::is_same<decltype(dd::get<2>(span<int>{nullptr,3})), int&>::value,
              "get<I> return type mismatch");

// Compile-time assertions
static_assert(std::tuple_size<span<int,3>>::value == 3, "tuple_size mismatch");
static_assert(std::is_same<std::tuple_element<1, span<int,3>>::type, int>::value, "tuple_element mismatch");
static_assert(std::is_same<decltype(dd::get<2>(span<int, dynamic_extent>{nullptr,3})), int&>::value,
              "get<I> return type mismatch");


TEST_CASE("Default construction (dynamic_extent)", "[span][ctor]") {
    span<int> s;
    REQUIRE(s.data() == nullptr);
    REQUIRE(s.size() == 0);
    REQUIRE(s.empty());
}

TEST_CASE("Pointer-count constructor and element access", "[span][ctor][access]") {
    int arr[3] = {1, 2, 3};
    span<int> s(arr, 3);
    REQUIRE(s.size() == 3);
    REQUIRE(!s.empty());
    REQUIRE(s[0] == 1);
    REQUIRE(s[1] == 2);
    REQUIRE(s[2] == 3);
    REQUIRE(s.front() == 1);
    REQUIRE(s.back() == 3);
    REQUIRE(s.data() == arr);
}

TEST_CASE("Pointer-range constructor", "[span][ctor]") {
    int arr[] = {10, 11, 12, 13, 14};
    span<int> s(arr + 1, arr + 4);
    REQUIRE(s.size() == 3);
    REQUIRE(s[0] == 11);
    REQUIRE(s[2] == 13);
}

TEST_CASE("Static extent from C-array", "[span][static][ctor]") {
    constexpr int carr[4] = {5, 6, 7, 8};
    span<const int,4> s(carr);
    REQUIRE(s.size() == 4);
    REQUIRE(s[1] == 6);
    REQUIRE(!s.empty());
}

TEST_CASE("Construction from std::array", "[span][ctor]") {
    std::array<int,3> a = {7, 8, 9};
    span<int,3> s1(a);
    span<const int,3> s2(a);
    REQUIRE(s1.size() == 3);
    REQUIRE(s1[2] == 9);
    REQUIRE(s2[0] == 7);
}

TEST_CASE("Construction from container (std::vector)", "[span][ctor]") {
    std::vector<int> v = {4, 5, 6, 7};
    span<int> s(v);
    REQUIRE(s.size() == v.size());
    for (size_t i = 0; i < v.size(); ++i) REQUIRE(s[i] == v[i]);
}


TEST_CASE("Construction from initializer_list", "[span][ctor][ilist]") {
    std::initializer_list<int> il = {9, 8, 7};
    span<const int> s(il);
    REQUIRE(s.size() == il.size());
    size_t idx = 0;
    for (int v : il) {
        REQUIRE(s[idx++] == v);
    }
}

TEST_CASE("Subviews (template overloads)", "[span][subspan]") {
    int arr[] = {0, 1, 2, 3, 4, 5};
    span<int> s(arr, 6);
    auto f2 = s.first<2>();
    REQUIRE(f2.size() == 2);
    REQUIRE(f2[0] == 0);
    auto l3 = s.last<3>();
    REQUIRE(l3.size() == 3);
    REQUIRE(l3[0] == 3);
    auto sub = s.subspan<1,4>();
    REQUIRE(sub.size() == 4);
    REQUIRE(sub[0] == 1);
    REQUIRE(sub[3] == 4);
}

TEST_CASE("Subviews (runtime overloads)", "[span][subspan]") {
    int arr[] = {10, 11, 12, 13, 14};
    span<int> s(arr, 5);
    auto f = s.first(3);
    REQUIRE(f.size() == 3);
    REQUIRE(f.back() == 12);
    auto l = s.last(2);
    REQUIRE(l.size() == 2);
    REQUIRE(l.front() == 13);
    auto ss = s.subspan(1, 3);
    REQUIRE(ss.size() == 3);
    REQUIRE(ss[0] == 11);
}

TEST_CASE("Observers: size, size_bytes, empty", "[span][observers]") {
    double darr[4] = {0.1, 0.2, 0.3, 0.4};
    span<double> s(darr, 4);
    REQUIRE(s.size() == 4);
    REQUIRE(s.size_bytes() == 4 * sizeof(double));
    REQUIRE(!s.empty());
    span<double> empty;
    REQUIRE(empty.empty());
}

TEST_CASE("Iterators and reverse iterators", "[span][iter]") {
    int arr[] = {1,2,3,4};
    span<int> s(arr,4);
    int sum = 0;
    for (auto it = s.begin(); it != s.end(); ++it) sum += *it;
    REQUIRE(sum == 10);
    sum = 0;
    for (auto it = s.rbegin(); it != s.rend(); ++it) sum += *it;
    REQUIRE(sum == 10);
}

TEST_CASE("Conversion to const span", "[span][conversion]") {
    int arr[] = {1,2,3,4};
    span<int> mutable_s(arr,4);
    span<const int> const_s = mutable_s;
    REQUIRE(const_s.size() == mutable_s.size());
    REQUIRE(const_s[2] == 3);
}

TEST_CASE("Deduction guides", "[span][deduction]") {
    int arr[3] = {7,8,9};
    auto s = span(arr); // should deduce span<int,3>
    static_assert(std::is_same<decltype(s), span<int,3>>::value, "Deduction failed");
    REQUIRE(s.size() == 3);
    REQUIRE(s[1] == 8);
}

TEST_CASE("Empty static extent", "[span][static][empty]") {
    span<int,0> s0;
    REQUIRE(s0.size() == 0);
    REQUIRE(s0.empty());
}

TEST_CASE("Data pointer arithmetic", "[span][pointer]") {
    int arr[10];
    for(int i=0;i<10;++i) arr[i]=i;
    span<int> s(arr,10);
    auto mid = s.subspan(5,2);
    REQUIRE(mid.data() == arr + 5);
    REQUIRE(mid[1] == 6);
}

TEST_CASE("as_bytes returns const byte span", "[span][bytes][const]") {
    uint32_t arr[2] = {0xAABBCCDD, 0};
    auto bs = dd::as_bytes(span<uint32_t,2>(arr));
    REQUIRE(bs.size() == sizeof(arr));
    REQUIRE(std::is_same<decltype(bs)::element_type, const byte>::value);
    REQUIRE(static_cast<unsigned char>(bs[3]) == ((const unsigned char*)arr)[3]);
}

TEST_CASE("Mutation through span reflects underlying data", "[span][mutation]") {
    int parr[5] = {0, 1, 2, 3, 4};
    span<int> ps(parr, 5);
    ps[2] = 20;
    REQUIRE(parr[2] == 20);

    int rarr[4] = {10, 11, 12, 13};
    span<int> rs(rarr, rarr+4);
    rs.front() = 100;
    REQUIRE(rarr[0] == 100);
    rs.back() = 130;
    REQUIRE(rarr[3] == 130);

    std::array<int,3> a = {1,2,3};
    span<int,3> sa(a);
    sa[1] = 22;
    REQUIRE(a[1] == 22);

    std::vector<int> v = {5,6,7,8};
    span<int> sv(v);
    sv[0] = 50;
    REQUIRE(v[0] == 50);

    int subarr[6] = {0,1,2,3,4,5};
    span<int> sub(subarr,6);
    auto mid2 = sub.subspan(2,3);
    mid2[1] = 42;
    REQUIRE(subarr[3] == 42);

    int barr[2] = {0x11223344, 0};
    auto wb = dd::as_writable_bytes(span<int,2>(barr));
    wb[1] = std::byte{0xFF};
    REQUIRE(reinterpret_cast<unsigned char*>(barr)[1] == 0xFF);
}

// Contract‑checking tests (static extent)
TEST_CASE("Contract checking: pointer‑count ctor (static extent)", "[span][contract]") {
    std::vector<int> vec{1,2,3};
    REQUIRE_THROWS_AS((span<int,3>{nullptr,1}), contract_violation_error);
    REQUIRE_THROWS_AS((span<int,3>{vec.data(),1}), contract_violation_error);
}

TEST_CASE("Contract checking: pointer‑range ctor (static extent)", "[span][contract]") {
    std::vector<int> vec{1,2,3};
    REQUIRE_THROWS_AS((span<int,3>{nullptr,nullptr}), contract_violation_error);
    REQUIRE_THROWS_AS((span<int,3>{vec.data(), vec.data()+1}), contract_violation_error);
}

// // Fixed‑size subspans (template)
// TEST_CASE("Contract checking: fixed‑size subspans", "[span][contract]") {
//     std::vector<int> vec{1,2,3,4,5};
//     auto s = make_span(vec);
//     REQUIRE_THROWS_AS((s.first<4>()),   contract_violation_error);
//     REQUIRE_THROWS_AS((s.last<4>()),    contract_violation_error);
//     REQUIRE_THROWS_AS((s.subspan<6>()), contract_violation_error);
//     REQUIRE_THROWS_AS((s.subspan<0,6>()), contract_violation_error);
// }

// Dynamic‑size subspans (runtime)
TEST_CASE("Contract checking: dynamic‑size subspans", "[span][contract]") {
    std::vector<int> vec{1,2,3};
    auto s = make_span(vec);
    REQUIRE_THROWS_AS(s.first(-1), contract_violation_error);
    REQUIRE_THROWS_AS(s.first(4),  contract_violation_error);
    REQUIRE_THROWS_AS(s.last(-1),  contract_violation_error);
    REQUIRE_THROWS_AS(s.last(4),   contract_violation_error);

    std::vector<int> vec2{1,2,3,4,5};
    auto s2 = make_span(vec2);
    REQUIRE_THROWS_AS(s2.subspan(-1),     contract_violation_error);
    REQUIRE_THROWS_AS(s2.subspan(6),      contract_violation_error);
    REQUIRE_THROWS_AS(s2.subspan(0,-2),   contract_violation_error);
    REQUIRE_THROWS_AS(s2.subspan(0,6),    contract_violation_error);
}

// Member access out‑of‑range
TEST_CASE("Contract checking: element access out of range", "[span][contract]") {
    std::vector<int> vec{1,2,3};
    auto s = make_span(vec);
    REQUIRE_THROWS_AS(s[-2], contract_violation_error);
    REQUIRE_THROWS_AS(s[42], contract_violation_error);
}

// front() and back() on empty
TEST_CASE("Contract checking: front/back on empty", "[span][contract]") {
    span<int> e;
    REQUIRE_THROWS_AS(e.front(), contract_violation_error);
    REQUIRE_THROWS_AS(e.back(),  contract_violation_error);
}


// Deduction guides
#ifdef DD_SPAN_HAVE_DEDUCTION_GUIDES
TEST_CASE("Deduction: raw array", "[span][deduction]") {
    int arr[]{1,2,3};
    auto s = span{arr};
    static_assert(std::is_same_v<decltype(s), span<int,3>>);
    REQUIRE(std::equal(std::begin(arr), std::end(arr), s.begin()));
}
TEST_CASE("Deduction: const raw array", "[span][deduction]") {
    constexpr int arr[]{1,2,3};
    auto s = span{arr};
    static_assert(std::is_same_v<decltype(s), span<const int,3>>);
    REQUIRE(std::equal(std::begin(arr), std::end(arr), s.begin()));
}
TEST_CASE("Deduction: std::array", "[span][deduction]") {
    std::array<int,3> arr{1,2,3};
    auto s = span{arr};
    static_assert(std::is_same_v<decltype(s), span<int,3>>);
    REQUIRE(std::equal(arr.begin(), arr.end(), s.begin()));
}
TEST_CASE("Deduction: const std::array", "[span][deduction]") {
    const std::array<int,3> arr{1,2,3};
    auto s = span{arr};
    static_assert(std::is_same_v<decltype(s), span<const int,3>>);
    REQUIRE(std::equal(arr.begin(), arr.end(), s.begin()));
}
TEST_CASE("Deduction: std::vector", "[span][deduction]") {
    std::vector v{1,2,3};
    auto s = span{v};
    static_assert(std::is_same_v<decltype(s), span<int>>);
    REQUIRE(std::equal(v.begin(), v.end(), s.begin()));
}
TEST_CASE("Deduction: const std::vector", "[span][deduction]") {
    const std::vector v{1,2,3};
    auto s = span{v};
    static_assert(std::is_same_v<decltype(s), span<const int>>);
    REQUIRE(std::equal(v.begin(), v.end(), s.begin()));
}
#ifdef DD_SPAN_HAVE_CPP17
TEST_CASE("Deduction: std::string_view", "[span][deduction]") {
    std::string_view str{"hello"};
    auto s = span{str};
    static_assert(std::is_same_v<decltype(s), span<const char>>);
    REQUIRE(std::equal(str.begin(), str.end(), s.begin()));
}
TEST_CASE("Deduction: const std::string_view", "[span][deduction]") {
    const std::string_view str{"hello"};
    auto s = span{str};
    static_assert(std::is_same_v<decltype(s), span<const char>>);
    REQUIRE(std::equal(str.begin(), str.end(), s.begin()));
}
#endif
#endif

// Structured bindings
TEST_CASE("Structured bindings", "[span][structured]") {
    int arr[]{1,2,3};
    auto&& [a1,a2,a3] = make_span(arr);
    REQUIRE(a1 == arr[0]);
    REQUIRE(a2 == arr[1]);
    REQUIRE(a3 == arr[2]);
    a1 = 99;
    REQUIRE(arr[0] == 99);
    a2 = 100;
    REQUIRE(arr[1] == 100);
}

// Constructibility traits
TEST_CASE("Constructibility from containers & spans", "[span][traits]") {
    using vec_t = std::vector<int>;
    using deque_t = std::deque<int>;
    static_assert(std::is_constructible_v<span<int>, vec_t&>);
    static_assert(!std::is_constructible_v<span<int>, const vec_t&>);
    static_assert(!std::is_constructible_v<span<int>, deque_t&>);
    static_assert(std::is_constructible_v<span<const int>, vec_t&>);
    static_assert(std::is_constructible_v<span<const int>, const vec_t&>);
    static_assert(!std::is_constructible_v<span<const int>, deque_t&>);
}

// Span conversion between extents
TEST_CASE("Span to span conversions & static assertions", "[span][traits]") {
    using zero_span = span<int,0>;
    using zero_const_span = span<const int,0>;
    using big_span = span<int,1000000>;
    using big_const_span = span<const int,1000000>;
    using dynamic_span = span<int>;
    using dynamic_const_span = span<const int>;

    static_assert(std::is_trivially_copyable_v<zero_span>);
    static_assert(std::is_trivially_move_constructible_v<zero_span>);
    static_assert(!std::is_constructible_v<zero_span, zero_const_span>);
    static_assert(!std::is_constructible_v<zero_span, big_span>);
    static_assert(!std::is_constructible_v<zero_span, big_const_span>);
    // static_assert(std::is_nothrow_constructible_v<zero_span, dynamic_span>);
    static_assert(!std::is_constructible_v<zero_span, dynamic_const_span>);

    static_assert(std::is_nothrow_constructible_v<zero_const_span, zero_span>);
    static_assert(std::is_trivially_copyable_v<zero_const_span>);
    static_assert(std::is_trivially_move_constructible_v<zero_const_span>);
    static_assert(!std::is_constructible_v<zero_const_span, big_span>);
    static_assert(!std::is_constructible_v<zero_const_span, big_const_span>);
    // static_assert(std::is_nothrow_constructible_v<zero_const_span, dynamic_span>);
    // static_assert(std::is_nothrow_constructible_v<zero_const_span, dynamic_const_span>);

    static_assert(!std::is_constructible_v<big_span, zero_span>);
    static_assert(!std::is_constructible_v<big_span, zero_const_span>);
    static_assert(std::is_trivially_copyable_v<big_span>);
    static_assert(std::is_trivially_move_constructible_v<big_span>);
    static_assert(!std::is_constructible_v<big_span, big_const_span>);
    // static_assert(std::is_nothrow_constructible_v<big_span, dynamic_span>);
    static_assert(!std::is_constructible_v<big_span, dynamic_const_span>);

    static_assert(!std::is_constructible_v<big_const_span, zero_span>);
    static_assert(!std::is_constructible_v<big_const_span, zero_const_span>);
    static_assert(std::is_trivially_copyable_v<big_const_span>);
    static_assert(std::is_trivially_move_constructible_v<big_const_span>);
    static_assert(std::is_nothrow_constructible_v<big_const_span, big_span>);
    // static_assert(std::is_nothrow_constructible_v<big_const_span, dynamic_span>);
    // static_assert(std::is_nothrow_constructible_v<big_const_span, dynamic_const_span>);

    static_assert(std::is_nothrow_constructible_v<dynamic_span, zero_span>);
    static_assert(!std::is_constructible_v<dynamic_span, zero_const_span>);
    static_assert(std::is_nothrow_constructible_v<dynamic_span, big_span>);
    static_assert(!std::is_constructible_v<dynamic_span, big_const_span>);
    static_assert(std::is_trivially_copyable_v<dynamic_span>);
    static_assert(std::is_trivially_move_constructible_v<dynamic_span>);
    static_assert(!std::is_constructible_v<dynamic_span, dynamic_const_span>);

    static_assert(std::is_nothrow_constructible_v<dynamic_const_span, zero_span>);
    static_assert(std::is_nothrow_constructible_v<dynamic_const_span, zero_const_span>);
    static_assert(std::is_nothrow_constructible_v<dynamic_const_span, big_span>);
    static_assert(std::is_nothrow_constructible_v<dynamic_const_span, big_const_span>);
    static_assert(std::is_trivially_copyable_v<dynamic_const_span>);
    static_assert(std::is_trivially_move_constructible_v<dynamic_const_span>);

    constexpr zero_const_span zc{};
    constexpr dynamic_const_span dc{zc};
    static_assert(dc.size() == 0);
    static_assert(dc.data() == nullptr);
    static_assert(dc.begin() == dc.end());
}

// Member subview operations (static extent)
TEST_CASE("Member subview operations", "[span][subspan]") {
    SECTION("first<N>") {
        int arr[]{1,2,3,4,5};
        span<int,5> s(arr);
        auto f = s.first<3>();
        static_assert(std::is_same_v<decltype(f), span<int,3>>);
        REQUIRE(f.size()==3);
        REQUIRE(f.data()==arr);
        REQUIRE(f.begin()==arr);
        REQUIRE(f.end()==arr+3);
    }
    SECTION("last<N>") {
        int arr[]{1,2,3,4,5};
        span<int,5> s(arr);
        auto l = s.last<3>();
        static_assert(std::is_same_v<decltype(l), span<int,3>>);
        REQUIRE(l.size()==3);
        REQUIRE(l.data()==arr+2);
        REQUIRE(l.begin()==arr+2);
        REQUIRE(l.end()==std::end(arr));
    }
    SECTION("subspan<N>") {
        int arr[]{1,2,3,4,5};
        span<int,5> s(arr);
        auto ss = s.subspan<1,2>();
        static_assert(std::is_same_v<decltype(ss), span<int,2>>);
        REQUIRE(ss.size()==2);
        REQUIRE(ss.data()==arr+1);
        REQUIRE(ss.begin()==arr+1);
        REQUIRE(ss.end()==arr+1+2);
    }
}

// Observers and element access
TEST_CASE("Observers and element access", "[span][observ]") {
    constexpr span<int,0> empty{};
    static_assert(empty.size()==0);
    static_assert(empty.empty());
    int arr[]{1,2,3};
    span<const int> s(arr);
    REQUIRE(s[0]==1);
    REQUIRE(s[1]==2);
    REQUIRE(s[2]==3);
}

// Iterator support
TEST_CASE("Iterator support", "[span][iter]") {
    std::vector<int> v;
    span<int> s(v);
    std::sort(s.begin(), s.end());
    REQUIRE(std::is_sorted(v.begin(), v.end()));
    const std::vector<int> vc{1,2,3};
    span<const int> cs(vc);
    REQUIRE(std::equal(cs.rbegin(), cs.rend(), vc.rbegin()));
}

// make_span free functions
TEST_CASE("make_span free functions", "[span][free]") {
    // C-array
    int arr[]{1,2,3};
    auto s1 = make_span(arr);
    static_assert(std::is_same_v<decltype(s1), span<int,3>>);
    REQUIRE(s1.data()==arr);
    REQUIRE(s1.size()==3);
    // std::array
    std::array<int,3> a{1,2,3};
    auto s2 = make_span(a);
    static_assert(std::is_same_v<decltype(s2), span<int,3>>);
    REQUIRE(s2.data()==a.data());
    REQUIRE(s2.size()==3);
    // std::vector
    std::vector<int> v2{1,2,3};
    auto s3 = make_span(v2);
    static_assert(std::is_same_v<decltype(s3), span<int>>);
    REQUIRE(s3.data()==v2.data());
    REQUIRE(s3.size()==3);
    #ifdef DD_SPAN_HAVE_CPP17
    std::string_view sv{"hello"};
    auto s4 = make_span(sv);
    static_assert(std::is_same_v<decltype(s4), span<const char>>);
    REQUIRE(s4.data()==sv.data());
    REQUIRE(s4.size()==sv.size());
    #endif
}
