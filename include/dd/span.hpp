// SPDX-License-Identifier: MIT
//
// MIT License
//
// Copyright (c) 2025 Marco Barbone
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <type_traits>

// CUDA host/device annotation
#if defined(__CUDACC__)
#define DD_SPAN_API __host__ __device__
#else
#define DD_SPAN_API
#endif

#ifndef DD_SPAN_NO_EXCEPTIONS
// Attempt to discover whether we're being compiled with exception support
#if !(defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND))
#define DD_SPAN_NO_EXCEPTIONS
#endif
#endif

#ifndef DD_SPAN_NO_EXCEPTIONS
#include <cstdio>
#include <stdexcept>
#endif

// Feature test
#ifndef DD_SPAN_NAMESPACE_NAME
#define DD_SPAN_NAMESPACE_NAME dd
#endif
#if __cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
#define DD_SPAN_HAVE_CPP17
#endif
#if __cplusplus >= 201402L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201402L)
#define DD_SPAN_HAVE_CPP14
#endif

namespace DD_SPAN_NAMESPACE_NAME {

// Default contract checking
#if !defined(DD_SPAN_THROW_ON_CONTRACT_VIOLATION) && !defined(DD_SPAN_TERMINATE_ON_CONTRACT_VIOLATION) &&              \
    !defined(DD_SPAN_NO_CONTRACT_CHECKING)
#if defined(NDEBUG) || !defined(DD_SPAN_HAVE_CPP14)
#define DD_SPAN_NO_CONTRACT_CHECKING
#else
#define DD_SPAN_TERMINATE_ON_CONTRACT_VIOLATION
#endif
#endif

#if defined(DD_SPAN_THROW_ON_CONTRACT_VIOLATION)
struct DD_SPAN_API contract_violation_error : std::logic_error {
  explicit contract_violation_error(const char *msg) : std::logic_error(msg) {}
};
DD_SPAN_API inline void contract_violation(const char *msg) { throw contract_violation_error(msg); }
#elif defined(DD_SPAN_TERMINATE_ON_CONTRACT_VIOLATION)
[[noreturn]] DD_SPAN_API inline void contract_violation(const char * /*unused*/) { std::terminate(); }
#endif

#if !defined(DD_SPAN_NO_CONTRACT_CHECKING)
#define DD_SPAN_STRINGIFY(cond) #cond
#define DD_SPAN_EXPECT(cond) (cond ? (void)0 : contract_violation("Expected " DD_SPAN_STRINGIFY(cond)))
#else
#define DD_SPAN_EXPECT(cond)
#endif

#if defined(DD_SPAN_HAVE_CPP17) || defined(__cpp_inline_variables)
#define DD_SPAN_INLINE_VAR inline
#else
#define DD_SPAN_INLINE_VAR
#endif

#if defined(DD_SPAN_HAVE_CPP14) || (defined(__cpp_constexpr) && __cpp_constexpr >= 201304)
#define DD_SPAN_CONSTEXPR14 constexpr
#else
#define DD_SPAN_CONSTEXPR14
#endif

#if defined(DD_SPAN_HAVE_CPP14) && (!defined(_MSC_VER) || _MSC_VER > 1900)
#define DD_SPAN_CONSTEXPR_ASSIGN constexpr
#else
#define DD_SPAN_CONSTEXPR_ASSIGN
#endif

#if defined(DD_SPAN_NO_CONTRACT_CHECKING)
#define DD_SPAN_CONSTEXPR11 constexpr
#else
#define DD_SPAN_CONSTEXPR11
#endif

#if defined(DD_SPAN_HAVE_CPP17) || defined(__cpp_deduction_guides)
#define DD_SPAN_HAVE_DEDUCTION_GUIDES
#endif

#if defined(DD_SPAN_HAVE_CPP17) || defined(__cpp_lib_byte)
#define DD_SPAN_HAVE_STD_BYTE
#endif

#if defined(DD_SPAN_HAVE_CPP17) || defined(__cpp_lib_array_constexpr)
#define DD_SPAN_HAVE_CONSTEXPR_STD_ARRAY_ETC
#endif

#if defined(DD_SPAN_HAVE_CONSTEXPR_STD_ARRAY_ETC)
#define DD_SPAN_ARRAY_CONSTEXPR constexpr
#else
#define DD_SPAN_ARRAY_CONSTEXPR
#endif

#ifdef DD_SPAN_HAVE_STD_BYTE
using byte = std::byte;
#else
using byte = unsigned char;
#endif

#if defined(DD_SPAN_HAVE_CPP17)
#define DD_SPAN_NODISCARD [[nodiscard]]
#else
#define DD_SPAN_NODISCARD
#endif

DD_SPAN_INLINE_VAR constexpr std::size_t dynamic_extent = SIZE_MAX;

template <typename ElementType, std::size_t Extent = dynamic_extent> class span;

namespace detail {

// Storage
template <typename E, std::size_t S> struct span_storage {
  DD_SPAN_API constexpr span_storage() noexcept = default;
  DD_SPAN_API constexpr span_storage(E *p_ptr, std::size_t /*unused*/) noexcept : ptr(p_ptr) {}
  E *ptr = nullptr;
  static constexpr std::size_t size = S;
};

template <typename E> struct span_storage<E, dynamic_extent> {
  DD_SPAN_API constexpr span_storage() noexcept = default;
  DD_SPAN_API constexpr span_storage(E *p_ptr, std::size_t p_size) noexcept : ptr(p_ptr), size(p_size) {}
  E *ptr = nullptr;
  std::size_t size = 0;
};

// data/size
#if defined(DD_SPAN_HAVE_CPP17) || defined(__cpp_lib_nonmember_container_access)
using std::data;
using std::size;
#else
template <class C> DD_SPAN_API constexpr auto size(const C &c) -> decltype(c.size()) { return c.size(); }
template <class T, std::size_t N> DD_SPAN_API constexpr std::size_t size(const T (&)[N]) noexcept { return N; }
template <class C> DD_SPAN_API constexpr auto data(C &c) -> decltype(c.data()) { return c.data(); }
template <class C> DD_SPAN_API constexpr auto data(const C &c) -> decltype(c.data()) { return c.data(); }
template <class T, std::size_t N> DD_SPAN_API constexpr T *data(T (&array)[N]) noexcept { return array; }
template <class E> DD_SPAN_API constexpr const E *data(std::initializer_list<E> il) noexcept { return il.begin(); }
#endif

#if defined(DD_SPAN_HAVE_CPP17) || defined(__cpp_lib_void_t)
using std::void_t;
#else
template <typename...> using void_t = void;
#endif

template <typename T> using uncvref_t = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

template <typename> struct is_span : std::false_type {};
template <typename T, std::size_t S> struct is_span<span<T, S>> : std::true_type {};

template <typename> struct is_std_array : std::false_type {};
template <typename T, std::size_t N> struct is_std_array<std::array<T, N>> : std::true_type {};

template <typename, typename = void> struct has_size_and_data : std::false_type {};
template <typename T>
struct has_size_and_data<T,
                         void_t<decltype(detail::size(std::declval<T>())), decltype(detail::data(std::declval<T>()))>>
    : std::true_type {};

template <typename C, typename U = uncvref_t<C>> struct is_container {
  static constexpr bool value =
      !is_span<U>::value && !is_std_array<U>::value && !std::is_array<U>::value && has_size_and_data<C>::value;
};

template <typename T> using remove_pointer_t = typename std::remove_pointer<T>::type;

template <typename, typename, typename = void> struct is_container_element_type_compatible : std::false_type {};
template <typename T, typename E>
struct is_container_element_type_compatible<
    T, E,
    typename std::enable_if<
        !std::is_same<typename std::remove_cv<decltype(detail::data(std::declval<T>()))>::type, void>::value &&
        std::is_convertible<remove_pointer_t<decltype(detail::data(std::declval<T>()))> (*)[], E (*)[]>::value>::type>
    : std::true_type {};

template <typename, typename = std::size_t> struct is_complete : std::false_type {};
template <typename T> struct is_complete<T, decltype(sizeof(T))> : std::true_type {};

} // namespace detail

// span class

template <typename ElementType, std::size_t Extent> class span {
  static_assert(std::is_object<ElementType>::value, "ElementType must be object");
  static_assert(detail::is_complete<ElementType>::value, "ElementType must be complete");
  static_assert(!std::is_abstract<ElementType>::value, "ElementType cannot be abstract");
  using storage_type = detail::span_storage<ElementType, Extent>;

public:
  using element_type = ElementType;
  using value_type = typename std::remove_cv<ElementType>::type;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using pointer = element_type *;
  using const_pointer = const element_type *;
  using reference = element_type &;
  using const_reference = const element_type &;
  using iterator = pointer;
  using reverse_iterator = std::reverse_iterator<iterator>;
  static constexpr size_type extent = Extent;

  // constructors
  template <std::size_t E = Extent, typename std::enable_if<(E == dynamic_extent || E <= 0), int>::type = 0>
  DD_SPAN_API constexpr span() noexcept {}

  DD_SPAN_API DD_SPAN_CONSTEXPR11 span(pointer ptr, size_type count) : storage_(ptr, count) {
    DD_SPAN_EXPECT(extent == dynamic_extent || count == extent);
  }
  DD_SPAN_API DD_SPAN_CONSTEXPR11 span(pointer first, pointer last) : storage_(first, last - first) {
    DD_SPAN_EXPECT(extent == dynamic_extent || (last - first) == static_cast<std::ptrdiff_t>(extent));
  }
  template <typename U, std::size_t N,
            typename std::enable_if<std::is_convertible<U (*)[], ElementType (*)[]>::value &&
                                        (Extent == dynamic_extent || Extent == N),
                                    int>::type = 0>
  DD_SPAN_API constexpr span(const span<U, N> &other) noexcept : storage_(other.data(), other.size()) {}

  DD_SPAN_API constexpr span(const span &other) noexcept = default;
  DD_SPAN_API DD_SPAN_CONSTEXPR_ASSIGN span &operator=(const span &other) noexcept = default;
  DD_SPAN_API ~span() noexcept = default;

  // container-based ctors
  template <std::size_t N, std::size_t E = Extent,
            typename std::enable_if<(E == dynamic_extent || N == E) && detail::is_container_element_type_compatible<
                                                                           element_type (&)[N], ElementType>::value,
                                    int>::type = 0>
  DD_SPAN_API constexpr span(element_type (&arr)[N]) noexcept : storage_(arr, N) {}

  template <typename T, std::size_t N, std::size_t E = Extent,
            typename std::enable_if<(E == dynamic_extent || N == E) && detail::is_container_element_type_compatible<
                                                                           std::array<T, N> &, ElementType>::value,
                                    int>::type = 0>
  DD_SPAN_API DD_SPAN_ARRAY_CONSTEXPR span(std::array<T, N> &arr) noexcept : storage_(arr.data(), N) {}
  template <typename T, std::size_t N, std::size_t E = Extent,
            typename std::enable_if<
                (E == dynamic_extent || N == E) &&
                    detail::is_container_element_type_compatible<const std::array<T, N> &, ElementType>::value,
                int>::type = 0>
  DD_SPAN_API DD_SPAN_ARRAY_CONSTEXPR span(const std::array<T, N> &arr) noexcept : storage_(arr.data(), N) {}

  template <typename Container, std::size_t E = Extent,
            typename std::enable_if<E == dynamic_extent && detail::is_container<Container>::value &&
                                        detail::is_container_element_type_compatible<Container &, ElementType>::value,
                                    int>::type = 0>
  DD_SPAN_API constexpr span(Container &cont) : storage_(detail::data(cont), detail::size(cont)) {}
  template <
      typename Container, std::size_t E = Extent,
      typename std::enable_if<E == dynamic_extent && detail::is_container<Container>::value &&
                                  detail::is_container_element_type_compatible<const Container &, ElementType>::value,
                              int>::type = 0>
  DD_SPAN_API constexpr span(const Container &cont) : storage_(detail::data(cont), detail::size(cont)) {}

  // subviews
  template <std::size_t Count> DD_SPAN_API DD_SPAN_CONSTEXPR11 span<element_type, Count> first() const {
    DD_SPAN_EXPECT(Count <= size());
    return {data(), Count};
  }
  template <std::size_t Count> DD_SPAN_API DD_SPAN_CONSTEXPR11 span<element_type, Count> last() const {
    DD_SPAN_EXPECT(Count <= size());
    return {data() + (size() - Count), Count};
  }
  template <std::size_t Offset, std::size_t Count = dynamic_extent>
  DD_SPAN_API DD_SPAN_CONSTEXPR11 auto subspan() const {
    DD_SPAN_EXPECT(Offset <= size() && (Count == dynamic_extent || Offset + Count <= size()));
    return span < ElementType, Count != dynamic_extent
                                   ? Count
                                   : (Extent != dynamic_extent ? Extent - Offset : dynamic_extent) >
                                         (data() + Offset, Count != dynamic_extent ? Count : size() - Offset);
  }
  DD_SPAN_API DD_SPAN_CONSTEXPR11 span<element_type, dynamic_extent> first(size_type count) const {
    DD_SPAN_EXPECT(count <= size());
    return {data(), count};
  }
  DD_SPAN_API DD_SPAN_CONSTEXPR11 span<element_type, dynamic_extent> last(size_type count) const {
    DD_SPAN_EXPECT(count <= size());
    return {data() + size() - count, count};
  }
  DD_SPAN_API DD_SPAN_CONSTEXPR11 span<element_type, dynamic_extent> subspan(size_type off,
                                                                             size_type cnt = dynamic_extent) const {
    DD_SPAN_EXPECT(off <= size() && (cnt == dynamic_extent || off + cnt <= size()));
    return {data() + off, cnt == dynamic_extent ? size() - off : cnt};
  }

  // observers
  DD_SPAN_API constexpr size_type size() const noexcept { return storage_.size; }
  DD_SPAN_API constexpr size_type size_bytes() const noexcept { return size() * sizeof(element_type); }
  DD_SPAN_API DD_SPAN_NODISCARD constexpr bool empty() const noexcept { return size() == 0; }

  // element access
  DD_SPAN_API DD_SPAN_CONSTEXPR11 reference operator[](size_type idx) const {
    DD_SPAN_EXPECT(idx < size());
    return *(data() + idx);
  }
  DD_SPAN_API DD_SPAN_CONSTEXPR11 reference front() const {
    DD_SPAN_EXPECT(!empty());
    return *data();
  }
  DD_SPAN_API DD_SPAN_CONSTEXPR11 reference back() const {
    DD_SPAN_EXPECT(!empty());
    return *(data() + size() - 1);
  }
  DD_SPAN_API constexpr pointer data() const noexcept { return storage_.ptr; }

  // iterators
  DD_SPAN_API constexpr iterator begin() const noexcept { return data(); }
  DD_SPAN_API constexpr iterator end() const noexcept { return data() + size(); }
  DD_SPAN_API DD_SPAN_ARRAY_CONSTEXPR reverse_iterator rbegin() const noexcept { return reverse_iterator(end()); }
  DD_SPAN_API DD_SPAN_ARRAY_CONSTEXPR reverse_iterator rend() const noexcept { return reverse_iterator(begin()); }

private:
  storage_type storage_;
};

#ifdef DD_SPAN_HAVE_DEDUCTION_GUIDES
template <class T, size_t N> DD_SPAN_API span(T (&)[N]) -> span<T, N>;
template <class T, size_t N> DD_SPAN_API span(std::array<T, N> &) -> span<T, N>;
template <class T, size_t N> DD_SPAN_API span(const std::array<T, N> &) -> span<const T, N>;
template <class Container>
DD_SPAN_API span(Container &)
    -> span<typename std::remove_reference<decltype(*detail::data(std::declval<Container &>()))>::type>;
template <class Container> DD_SPAN_API span(const Container &) -> span<const typename Container::value_type>;
#endif

// free makers

template <typename ET, std::size_t E> DD_SPAN_API constexpr span<ET, E> make_span(span<ET, E> s) noexcept { return s; }
template <typename T, size_t N> DD_SPAN_API constexpr span<T, N> make_span(T (&arr)[N]) noexcept { return {arr}; }
template <typename T, size_t N>
DD_SPAN_API DD_SPAN_ARRAY_CONSTEXPR span<T, N> make_span(std::array<T, N> &arr) noexcept {
  return {arr};
}
template <typename T, size_t N>
DD_SPAN_API DD_SPAN_ARRAY_CONSTEXPR span<const T, N> make_span(const std::array<T, N> &arr) noexcept {
  return {arr};
}
template <typename Container>
DD_SPAN_API constexpr span<typename std::remove_reference<decltype(*detail::data(std::declval<Container &>()))>::type>
make_span(Container &cont) {
  return {cont};
}
template <typename Container>
DD_SPAN_API constexpr span<const typename Container::value_type> make_span(const Container &cont) noexcept {
  return {cont};
}

template <typename ET, size_t E>
DD_SPAN_API span<const byte, ((E == dynamic_extent) ? dynamic_extent : sizeof(ET) * E)>
as_bytes(span<ET, E> s) noexcept {
  return {reinterpret_cast<const byte *>(s.data()), s.size_bytes()};
}
template <class ET, size_t E, typename std::enable_if<!std::is_const<ET>::value, int>::type = 0>
DD_SPAN_API span<byte, ((E == dynamic_extent) ? dynamic_extent : sizeof(ET) * E)>
as_writable_bytes(span<ET, E> s) noexcept {
  return {reinterpret_cast<byte *>(s.data()), s.size_bytes()};
}

template <std::size_t I, typename ET, size_t E> DD_SPAN_API constexpr auto get(span<ET, E> s) -> decltype(s[I]) {
  return s[I];
}

} // namespace DD_SPAN_NAMESPACE_NAME

namespace std {

template <typename ET, size_t E>
struct tuple_size<DD_SPAN_NAMESPACE_NAME::span<ET, E>> : public integral_constant<size_t, E> {};
template <typename ET> struct tuple_size<DD_SPAN_NAMESPACE_NAME::span<ET, DD_SPAN_NAMESPACE_NAME::dynamic_extent>>;
template <size_t I, typename ET, size_t E> struct tuple_element<I, DD_SPAN_NAMESPACE_NAME::span<ET, E>> {
  static_assert(E != DD_SPAN_NAMESPACE_NAME::dynamic_extent && I < E, "");
  using type = ET;
};

} // namespace std
